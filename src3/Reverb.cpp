//
//  Reverb.cpp
//  NSModule
//
//  Created by amos-mbp on 2018/1/26.
//  Copyright © 2018年 amos-mbp. All rights reserved.
//

#include "Reverb.hpp"
#include <malloc.h>

Reverb::Reverb(int sampleRate):stereoBuffer(NULL),
                               reverb(NULL){
    reverb = new SuperpoweredReverb(sampleRate);
}

Reverb::~Reverb(){
    if (reverb) {
        delete reverb;
    }
    
    if (stereoBuffer) {
        delete stereoBuffer;
        stereoBuffer = NULL;
    }
}

void Reverb::applyReverbFX(float dry, float wet,float width, float roomsize, float mix, float damp){
    reverb->reset();
    reverb->enable(true);
    
    reverb->setDry(dry);
    reverb->setWet(wet);
    reverb->setRoomSize(roomsize);
    reverb->setWidth(width);
    reverb->setDamp(damp);
    reverb->setMix(mix);
}

void Reverb::process(short *input, int numberofSample){
    if (!stereoBuffer) {
        stereoBuffer = (float *) memalign(16, (numberofSample + 16) * sizeof(float) * 2);
    }
    
    SuperpoweredShortIntToFloat(input, stereoBuffer, numberofSample);
    reverb->process(stereoBuffer, stereoBuffer, numberofSample);
    SuperpoweredFloatToShortInt(stereoBuffer, input, numberofSample);
}
