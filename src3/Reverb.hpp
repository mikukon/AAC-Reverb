//
//  Reverb.hpp
//  NSModule
//
//  Created by amos-mbp on 2018/1/26.
//  Copyright © 2018年 amos-mbp. All rights reserved.
//

#ifndef Reverb_hpp
#define Reverb_hpp

#include <stdio.h>
#include <stdint.h>
#include "SuperpoweredReverb.h"
#include "SuperpoweredSimple.h"

class Reverb{
public:
    Reverb(int sampleRate);
    ~Reverb();
    void applyReverbFX(float dry, float wet,float width, float roomsize, float mix, float damp);
    void process(short* input,int numberofSample);
private:
    SuperpoweredReverb* reverb;
    float* stereoBuffer;
};

#endif /* Reverb_hpp */
