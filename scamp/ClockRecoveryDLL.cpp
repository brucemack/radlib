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
#include <iostream>
#include "ClockRecoveryDLL.h"

using namespace std;

namespace radlib {

ClockRecoveryDLL::ClockRecoveryDLL(uint16_t sampleRate) 
:   _sampleRate(sampleRate) {
}

void ClockRecoveryDLL::setClockFrequency(uint16_t dataFreq) {    
    uint32_t y = _sampleRate / dataFreq;
    _omega = (uint32_t)_maxPhi / y;
}

bool ClockRecoveryDLL::processSample(uint8_t symbol) {

    if (_lastSymbol != symbol) {
        _edgeDetected();
        _lastSymbol = symbol;
    }
    _samplesSinceEdge++;
    // Look for tht wrap-around
    bool capture = (int32_t)_phi + (int32_t)_omega > (int32_t)_maxPhi;
    // Move forward and wrap
    _phi = (_phi + _omega) & 0x7fff;
    _lastPhi = _phi;
    return capture;
}

int16_t ClockRecoveryDLL::getLastError() const { 
    return _lastError; 
}

float ClockRecoveryDLL::getLastPhaseError() const {
    return (float) _lastError / (float)_maxPhi;
}

uint32_t ClockRecoveryDLL::getClockFrequency() const {
    return (_omega * _sampleRate) / _maxPhi;
}

uint16_t ClockRecoveryDLL::getSamplesSinceEdge() const { 
    return _samplesSinceEdge; 
}

void ClockRecoveryDLL::_edgeDetected() {  
    // Error will be positive if we are lagging the target phase  
    int16_t error = _phi - _targetPhi;
    _lastError = error;
    _errorIntegration += error;

    // Apply the gain
    //int32_t adj = (error >> 1) + (error >> 2);
    int32_t adj = (error >> 1);

    /*
    cout << "_phi=" << _phi << ", _targetPhi=" << _targetPhi << ", omega=" << _omega 
        << ", error=" << error 
        << ", adj=" << -adj 
        << ", int=" << _errorIntegration 
        << endl;
    */

    _phi -= adj;
    _samplesSinceEdge = 0;
}

}
