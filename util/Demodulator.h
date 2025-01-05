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
#ifndef _Demodulator_h
#define _Demodulator_h

#include <cstdint>

#include "../util/fixed_math.h"
#include "../util/fixed_fft.h"
#include "DemodulatorListener.h"

#define SYMBOL_COUNT (2)

namespace radlib {

/**
 * This contains all of the FSK demodulator logic.  This is a stateful
 * object that will be called once every sample interval with the latest
 * available sample.
 */
class Demodulator {
public:

    /**
     * @param maxSampleN Controls how many samples are used to maintain the 
     *   "maximum sample value."  This feature is useful for tuning the 
     *   gain on the receiver.
    */

    Demodulator(uint16_t sampleFreq, uint16_t lowestFreq,
        uint16_t log2fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, cq15* fftResultSpace, 
        q15* bufferSpace,
        uint16_t maxSampleN = 512);

    virtual void setListener(DemodulatorListener* listener) { _listener = listener; };

    /**
     * Call this depending on the mode being used.  IMPORTANT: You need
     * to call setFrequencyLock() after this (or go through an auto-lock
     * sequence) for this to take effect.
     */
    void setSymbolSpread(float spreadHz) { _symbolSpreadHz = spreadHz; };

    /**
     * Call this function at the rate defined by sampleFreq and pass the latest
     * sample from the ADC.  Everything happens here!
     */
    void processSample(q15 sample);

    /**
     * Call this function to clear the frequency lock and any other internal
     * state.
    */
    virtual void reset();

    /**
     * Locks the demodulator on the specified mark frequency.
     */
    void setFrequencyLock(float markFreqHz);

    bool isFrequencyLocked() const { return _frequencyLocked; }

    void setAutoLockEnabled(bool en) { _autoLockEnabled = en; }

    float getMarkFreq() const;

    float getLastDCPower() const { return _lastDCPower; };

    /**
     * This is a statistical function that is useful for tuning the receiver
     * gain.
     * 
     * @return The maximum sample value seen over the window defined by the  
     *   maxSampleN parameter passed to the constructor.
     */    
    q15 getMaxSample() const { return _maxSample; }

    /**
     * This is a statistical function that is useful for tuning the receiver
     * DC bias and gain.
     * 
     * @return The number of above zero samples seen over the window defined by the  
     *   maxSampleN parameter passed to the constructor.
     */    
    uint16_t getPosCount() const { return _posCount; }

    /**
     * Sets the threshold value that determines whether a symbol is being 
     * heard vs noise.
    */
    void setDetectionCorrelationThreshold(float t) {
        _detectionCorrelationThreshold = t;
    }

    float getDetectionCorrelationThreshold() const { return _detectionCorrelationThreshold; }

protected:

    /**
     * This is called on every sample and indicates which symbol (if any) has been
     * detected.
     */
    virtual void _processSymbol(bool symbolValid, uint8_t symbol) = 0;

    DemodulatorListener* _listener;

private: 

    void _clearCorrelationHistory();

    const uint16_t _sampleFreq;
    const uint16_t _fftN;
    const uint16_t _log2fftN;
    // This is the first bin that we pay attention to
    uint16_t _firstBin;
    // Optional space passed in by user
    q15* _fftWindow;
    cq15* _fftResult;
    FixedFFT _fft;
  
    // FFT is performed every time this number of samples is collected
    const uint16_t _blockSize = 32;
    // This is the approximate symbol rate for SCAMP FSK.  This is used
    // only to estimate the length of the "long mark"    
    const unsigned int _samplesPerSymbol = 60;
    // Calculate the duration of the block in seconds
    const float _blockDuration = (float)_blockSize / (float)_sampleFreq;
    // Calculate the symbol duration in seconds
    const float _symbolDuration = (float)_samplesPerSymbol / (float)_sampleFreq;
    // Calculate the duration of the "long mark"
    const float _longMarkDuration = 24.0 * _symbolDuration;
    // Calculate the number of blocks that make up the long mark and
    // round down.  We give this a slight haircut to improve robustness.
    const uint16_t _longMarkBlocks = ((_longMarkDuration / _blockDuration) * 0.70);

    // Controls whether auto-lock is enabled
    bool _autoLockEnabled = true;

    // The distance between the two symbols in positive HZ. The
    // default here is relevant to the SCAMP FSK mode.
    float _symbolSpreadHz = 66.6666666666;

    uint32_t _sampleCount = 0;

    // Here's where we put the recent sample history in order to build
    // up enough to run the spectral analysis.
    uint16_t _bufferPtr = 0;
    q15* _buffer; 

    float _lastDCPower = 0;

    // This is where we store the recent history of the loudest bin 
    const uint16_t _maxBinHistorySize = 64;
    //const uint16_t _maxBinHistoryBins = 4;
    uint16_t _maxBinHistory[64];
    // The power threshold used for detecting a valid signal
    // Power of 0.002 was measured with Vpp = 1.5v
    float _binPowerThreshold = 5.0e-4;

    // Indicates whether the demodulator is locked onto a specific frequency
    // or whether it is in frequency acquisition mode.
    bool _frequencyLocked = false;

    // The frequency that has been selected to represent "mark"
    float _lockedMarkFreq = 0;

    uint16_t _blockCount = 0;
    uint8_t _activeSymbol = 0;

    // These buffers are loaded based on the frequency that the decoder
    // decides to lock onto. The signal is convolved with these tones
    // for demodulation.
    static const uint16_t _symbolCount = SYMBOL_COUNT;
    static const uint16_t _demodulatorToneN = 16;
    cq15 _demodulatorTone[_symbolCount][_demodulatorToneN];

    // The most recent correlation history for each symbol
    static const uint16_t _symbolCorrN = 64;
    float _symbolCorr[_symbolCount][_symbolCorrN];
    uint16_t _symbolCorrPtr = 0;

    float _detectionCorrelationThreshold = 0;
    float _lastCorrDiff = 0;

    const uint16_t _maxSampleN;
    uint16_t _maxSampleCtr;
    q15 _maxSampleAcc = 0;
    q15 _maxSample = 0;
    // Used to keep track of the number of samples above zero.
    // This is helpful for adjusting DC bias.
    int16_t _posCountAcc = 0;
    int16_t _posCount = 0;
};

}

#endif

