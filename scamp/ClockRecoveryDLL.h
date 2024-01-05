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
#ifndef _ClockRecoveryDLL_H
#define _ClockRecoveryDLL_H

#include <cstdint>
#include "ClockRecovery.h"

namespace radlib {

/**
 * A Delay Locked LooP (DLL) class used for recovering the bit clock on an 
 * input stream.
 */
class ClockRecoveryDLL : public ClockRecovery {
public:

    /**
     * @param sampleRate Should be set to the base sampling frequency
     *   in Hertz.  For SCAMP this would typically be 2000.
    */
    ClockRecoveryDLL(uint16_t sampleRate = 2000);

    /**
     * @param clockFreqHz Set this to the expected data clock frequency.
    */
    void setClockFrequency(uint16_t clockFreqHz);

    /**
     * This is the main function.  Call this at the sample rate frequency with 
     * an observation of the incoming signal.  If the return is true then the 
     * caller should use the sample as the observation of the curent bit. If
     * the return is false then just ignore the sample and keep going.
     */
    bool processSample(uint8_t symbol);

    int32_t getIntegration() const { return 0; }

    int16_t getLastError() const;

    float getLastPhaseError() const;

    /**
     * @returns The lock frequency in Hertz.
     */
    uint32_t getClockFrequency() const;

    /**
     * @returns The number of samples received since the last edge
     *   transition.
     */
    uint16_t getSamplesSinceEdge() const;

    /**
     * Use this method to control whether the DLL is free or locked.
     * When locked, the ability to change phase is highly restricted.
    */
    void setLock(bool locked) { _locked = locked; };

private:

    void _edgeDetected();

    bool _locked = false;
    const int16_t _maxPhi = 0x7fff;
    const int16_t _targetPhi = _maxPhi >> 1;
    const uint16_t _sampleRate;
    uint16_t _omega = 0;
    int16_t _phi = 0;
    int16_t _lastPhi = 0;
    uint16_t _samplesSinceEdge = 0;
    int16_t _lastError = 0;
    uint8_t _lastSymbol = 0;
    int32_t _errorIntegration = 0;
};

}

#endif
