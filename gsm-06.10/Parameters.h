/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _Parameters_h
#define _Parameters_h

#include <cstdint>

namespace radlib {

struct SubFrameParameters {
    uint8_t Nc;
    uint8_t bc;
    uint8_t Mc;
    uint8_t xmaxc;
    uint8_t xMc_00;
    uint8_t xMc_01;
    uint8_t xMc_02;
    uint8_t xMc_03;
    uint8_t xMc_04;
    uint8_t xMc_05;
    uint8_t xMc_06;
    uint8_t xMc_07;
    uint8_t xMc_08;
    uint8_t xMc_09;
    uint8_t xMc_10;
    uint8_t xMc_11;
    uint8_t xMc_12;
};

struct Parameters {
    
    uint8_t LARc_1;
    uint8_t LARc_2;
    uint8_t LARc_3;
    uint8_t LARc_4;
    uint8_t LARc_5;
    uint8_t LARc_6;
    uint8_t LARc_7;
    uint8_t LARc_8;

    SubFrameParameters subFrames[4];
};

}    

#endif

