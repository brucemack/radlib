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

// LPF with cut-off at 33 Hz, transition 200 Hz, Blackman window.  Coefficients = 47.
// Built from: https://fiiir.com/
//
static const uint16_t h_lpf_33_size = 47;
static const float h_lpf_33[] = {
    0.000000000000000000,
    0.000031950343187103,
    0.000147936959029139,
    0.000384359636563844,
    0.000787003634738626,
    0.001411141838523909,
    0.002319784160773662,
    0.003579944811985529,
    0.005257067022493679,
    0.007408017440172351,
    0.010073303620179160,
    0.013269345114754810,
    0.016981715668375547,
    0.021160255073868633,
    0.025716820932183259,
    0.030526222709496793,
    0.035430575357155471,
    0.040246959675527820,
    0.044777920302203671,
    0.048824010340036958,
    0.052197341954698558,
    0.054734955050421391,
    0.056310790486405832,
    0.056845155734448642,
    0.056310790486405839,
    0.054734955050421391,
    0.052197341954698551,
    0.048824010340036979,
    0.044777920302203678,
    0.040246959675527820,
    0.035430575357155471,
    0.030526222709496811,
    0.025716820932183266,
    0.021160255073868626,
    0.016981715668375537,
    0.013269345114754815,
    0.010073303620179167,
    0.007408017440172354,
    0.005257067022493686,
    0.003579944811985531,
    0.002319784160773660,
    0.001411141838523911,
    0.000787003634738626,
    0.000384359636563845,
    0.000147936959029139,
    0.000031950343187103,
    0.000000000000000000,
};

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

    _clearCorrelationHistory();
}

void Demodulator::_clearCorrelationHistory() {
    for (uint16_t s = 0; s < _symbolCount; s++)
       for (uint16_t i = 0; i < _symbolCorrN; i++)
            _symbolCorr[s][i] = 0;
    _symbolCorrPtr = 0;
}

void Demodulator::setFrequencyLock(float lockedMarkHz) {

    _frequencyLocked = true;
    _lockedMarkFreq = lockedMarkHz;

    // NOTE: These tones are scaled by 0.5 to avoid overflow issues
    make_complex_tone_cq15(_demodulatorTone[0], _demodulatorToneN, 
        _sampleFreq, lockedMarkHz - _symbolSpreadHz, 0.5);
    make_complex_tone_cq15(_demodulatorTone[1], _demodulatorToneN, 
        _sampleFreq, lockedMarkHz, 0.5);

    _listener->frequencyLocked(lockedMarkHz, lockedMarkHz - _symbolSpreadHz);                    
}

void Demodulator::reset() {
    _frequencyLocked = false;
    _clearCorrelationHistory();
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
            } else {
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

        // ----- Matched Filter Implementation --------------------------------
        //
        // Correlate recent history with each of the symbol models to look 
        // for matches.
        float filteredSymbolCorr[_symbolCount];
        
        for (uint16_t s = 0; s < _symbolCount; s++) {

            // Correlate the received data with the model symbol.
            // Here we have automatic wrapping in the _buffer space, so don't
            // worry if demodulatorStart is close to the end.
            _symbolCorr[s][_symbolCorrPtr] = corr_q15_cq15_2(_buffer, demodulatorStart, _fftN, 
                _demodulatorTone[s], _demodulatorToneN);            

            // Apply a low-pass filter to the recent history of the correlations
            // so that we can properly identify the transitions.  The cut-off of this
            // filter is determined by the baud rate of the data being recovered.
            uint16_t corrPtr = _symbolCorrPtr;
            float conv = 0;
            for (uint16_t i = 0; i < h_lpf_33_size; i++) {
                // Multiply-accumulate
                conv += _symbolCorr[s][corrPtr] * h_lpf_33[i];
                // Track backwards through the recent correlations, wrapping as needed
                if (corrPtr == 0) {
                    corrPtr = _symbolCorrN - 1;
                } else {
                    corrPtr--;
                }
            }
            filteredSymbolCorr[s] = conv;
        }

        // Keep rotating through history of correlations, wrapping as needed
        if (++_symbolCorrPtr == _symbolCorrN) {
            _symbolCorrPtr = 0;
        }

        // The difference is adjusted so that a symbol transition is always an 
        // increasing difference in correlations.
        float corrDiff;
        if (_activeSymbol == 0) {
            corrDiff = filteredSymbolCorr[1] - filteredSymbolCorr[0];
        } else {
            corrDiff = filteredSymbolCorr[0] - filteredSymbolCorr[1];
        }

        // Once we cross 0 we have detected a transition
        if (corrDiff > 0) {
            // Reverse the active symbol
            if (_activeSymbol == 0) {
                _activeSymbol = 1;
            } else {
                _activeSymbol = 0;
            }
            _listener->symbolTransitionDetected();
        }

        bool aboveCorrelationThreshold = 
            filteredSymbolCorr[_activeSymbol] > _detectionCorrelationThreshold;

        _lastCorrDiff = corrDiff;

        // Report out all of the key parameters
        _listener->sampleMetrics(sample, _activeSymbol, filteredSymbolCorr, aboveCorrelationThreshold);

        // Hand off a demodulated symbol to the decoder
        _processSymbol(aboveCorrelationThreshold, _activeSymbol);
    }
}

float Demodulator::getMarkFreq() const {
    return _lockedMarkFreq;
}

}

