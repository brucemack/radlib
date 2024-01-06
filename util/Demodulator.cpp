/*
Copyright (C) 2024 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <cstdint>
#include <iostream>
#include <cstring>

#include "../util/dsp_util.h"
#include "Demodulator.h"

using namespace std;

namespace radlib {

Demodulator::Demodulator(uint16_t sampleFreq, uint16_t lowestFreq, uint16_t log2fftN,
    q15* fftTrigTable, q15* fftWindow,
    cq15* fftResultSpace, q15* bufferSpace)
:   _sampleFreq(sampleFreq),
    _fftN(1 << log2fftN),
    _log2fftN(log2fftN),
    _firstBin((_fftN * lowestFreq) / sampleFreq),
    _fftWindow(fftWindow),
    _fftResult(fftResultSpace),
    _fft(_fftN, fftTrigTable),
    _buffer(bufferSpace) { 

    // Build the Hann window for the FFT (raised cosine) if a space has 
    // been provided for it.
    if (_fftWindow != 0) {
        for (uint16_t i = 0; i < _fftN; i++) {
            _fftWindow[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)_fftN))));
        }
    }

    memset((void*)_buffer, 0, _fftN);
    memset((void*)_maxBinHistory, 0, sizeof(_maxBinHistory));
    memset((void*)_demodulatorTone, 0, sizeof(_demodulatorTone));
    memset((void*)_maxCorrHistory, 0, sizeof(_maxCorrHistory));
}

void Demodulator::setFrequencyLock(float lockedMarkHz) {

    _frequencyLocked = true;
    _lockedMarkFreq = lockedMarkHz;

    make_complex_tone(_demodulatorTone[0], _demodulatorToneN, 
        _sampleFreq, lockedMarkHz - _symbolSpreadHz, 0.5);
    make_complex_tone(_demodulatorTone[1], _demodulatorToneN, 
        _sampleFreq, lockedMarkHz, 0.5);

    _listener->frequencyLocked(lockedMarkHz, lockedMarkHz - _symbolSpreadHz);                    
}

void Demodulator::reset() {
    _frequencyLocked = false;
    _edgeRiseSampleCounter = 0;
}

void Demodulator::processSample(q15 sample) {

    // Capture the sample in the circular buffer            
    _buffer[_bufferPtr] = sample;
    // Remember where the reading starts
    const uint16_t readBufferPtr = _bufferPtr;
    // Increment the write pointer and wrap if needed
    _bufferPtr = (_bufferPtr + 1) % _fftN;
    _sampleCount++;

    // Did we just finish a new block?  If so, run the FFT
    if (_bufferPtr % _blockSize == 0) {
        
        _blockCount++;

        // Compute the average across the FFT buffer for the purposes of DC
        // bias removal
        q15 avg = mean_q15(_buffer, _log2fftN);

        // Do the FFT in the result buffer, including the window.  
        for (uint16_t i = 0; i < _fftN; i++) {
            if (_fftWindow != 0) {
                _fftResult[i].r = mult_q15(
                    _buffer[wrapIndex(readBufferPtr, i, _fftN)] - avg, 
                    _fftWindow[i]
                );
            } 
            else {
                _fftResult[i].r = _buffer[wrapIndex(readBufferPtr, i, _fftN)] - avg;
            }
            _fftResult[i].i = 0;
        }

        _fft.transform(_fftResult);

        // Find the largest power. Notice that we ignore some low bins (DC)
        // since that's not relevant to the spectral analysis.
        const uint16_t maxBin = max_idx_2(_fftResult, _firstBin, _fftN / 2);

         // Capture DC magnitude for diagnostics
        _lastDCPower = _fftResult[0].mag_f32_squared();
 
        // If we are not yet frequency locked, try to lock
        if (!_frequencyLocked && _autoLockEnabled) {

            // Find the total power
            float totalPower = 0;
            for (uint16_t i = _firstBin; i < _fftN / 2; i++) {
                totalPower += _fftResult[i].mag_f32_squared();
            }
            // Find the percentage of power at the max (and two adjacent)
            float maxBinPower = _fftResult[maxBin].mag_f32_squared();
            if (maxBin > 1) {
                maxBinPower += _fftResult[maxBin - 1].mag_f32_squared();
            }
            if (maxBin < (_fftN / 2) - 1) {
                maxBinPower += _fftResult[maxBin + 1].mag_f32_squared();
            }
            const float maxBinPowerFract = maxBinPower / totalPower;

            // Shift the history collection area and accumulate the new 
            // observation.
            for (uint16_t i = 0; i < _maxBinHistorySize - 1; i++) {
                _maxBinHistory[i] = _maxBinHistory[i + 1];
            }
            _maxBinHistory[_maxBinHistorySize - 1] = maxBin;

            // Find the variance of the max bin across the recent observations
            // to see if we have stability.  We are only looking 
            uint16_t binHistoryStart;
            uint16_t binHistoryLength;
            if (_longMarkBlocks > _maxBinHistorySize) {
                binHistoryStart = 0;
                binHistoryLength = _maxBinHistorySize;
            } else {
                binHistoryStart = _maxBinHistorySize - _longMarkBlocks;
                binHistoryLength = _longMarkBlocks;
            }

            // The locking logic only works when the history is full
            if (_blockCount >= binHistoryLength) {

                // Calculate the percentage of the recent history that is 
                // within a few bins of the current max.  We are only looking at 
                // the training section of the history here.
                uint16_t hitCount = 0;
                for (uint16_t i = binHistoryStart; i < _maxBinHistorySize; i++) {
                    if (_maxBinHistory[i] >= maxBin - 1 &&
                        _maxBinHistory[i] <= maxBin + 1) {
                        hitCount++;
                    }
                }

                // TODO: REMOVE FLOATING POINT 
                float hitPct = (float)hitCount / (float)binHistoryLength;

                // If one bin is dominating then perform a lock
                if (maxBinPower > _binPowerThreshold && 
                    hitPct > 0.75 && 
                    maxBinPowerFract > 0.20) {

                    // Convert the bin number to a frequency in Hz
                    float lockedMarkHz = (float)maxBin * (float)_sampleFreq / (float)_fftN;

                    setFrequencyLock(lockedMarkHz);
                }
            }
        }
    }

    // ----- Quadrature Demodulation -----------------------------------------

    if (_frequencyLocked) {

        // Figure out the detection starting point. Back up the length of the 
        // demodulator series, wrapping as necessary.
        // 
        // TEST CASE 2: fftN = 512, demodToneN = 16, readBufferPtr = 15
        // We need to start at 511.  Gap = 16-15 = 1, start = 512 - 1
        //
        uint16_t demodulatorStart = 0;
        if (readBufferPtr >= _demodulatorToneN) {
            demodulatorStart = readBufferPtr - _demodulatorToneN;
        } else {
            uint16_t gap = _demodulatorToneN - readBufferPtr;
            demodulatorStart = _fftN - gap;
        }

        // Correlate recent history with each of the symbol tones templates
        float maxCorr = 0;
        for (uint16_t s = 0; s < _symbolCount; s++) {
            // Here we have automatic wrapping in the _buffer space, so don't
            // worry if demodulatorStart is close to the end.
            _symbolCorr[s] = corr_real_complex_2(_buffer, demodulatorStart, _fftN, 
                _demodulatorTone[s], _demodulatorToneN);
            maxCorr = std::max(maxCorr, _symbolCorr[s]);
        }

        // Here we track the recent history of the maximum correlation
        _maxCorrHistory[_maxCorrHistoryPtr] = maxCorr;
        _maxCorrHistoryPtr = incAndWrap(_maxCorrHistoryPtr, _maxCorrHistoryN);
    
        // Figure out the current threshold by averaging 
        float thresholdCorr = 0;
        for (uint16_t i = 0; i < _maxCorrHistoryN; i++) {
            thresholdCorr += _maxCorrHistory[i];
        }
        // The correlation diff must reach 33% of the recent maximum correlation 
        // to be considered a transition.
        thresholdCorr /= (3.0 * (float)_maxCorrHistoryN);

        // The difference is adjusted so that a transition is always an increasing
        // difference.
        float corrDiff;
        if (_activeSymbol == 0) {
            corrDiff = _symbolCorr[1] - _symbolCorr[0];
        } else {
            corrDiff = _symbolCorr[0] - _symbolCorr[1];
        }

        // Look for an inflection point in the respective correlations 
        // of the symbols.  
        if (corrDiff > thresholdCorr) {
            // If we are still increasing then let it keep going up before 
            // declaring an edge transition
            if (corrDiff > _lastCorrDiff && 
                _edgeRiseSampleCounter < _edgeRiseSampleLimit) {
                _edgeRiseSampleCounter++;
            }
            else {
                // Reverse the active symbol
                if (_activeSymbol == 0) {
                    _activeSymbol = 1;
                }
                else {
                    _activeSymbol = 0;
                }
                _edgeRiseSampleCounter = 0;
                _listener->bitTransitionDetected();
            }
        }

        _lastCorrDiff = corrDiff;

        // Report out all of the key parameters
        // TODO: NEED TO CLEAN THIS UP
        _listener->sampleMetrics(sample, _activeSymbol, false, 0, _symbolCorr, thresholdCorr, corrDiff);

        // Hand off a demodulated symbol to the decoder
        _processSymbol(_activeSymbol);
    }
}

float Demodulator::getMarkFreq() const {
    return _lockedMarkFreq;
}

}

