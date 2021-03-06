/* 
 * Copyright (C) 2013 Gilad Dayagi.  All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/*
 * Emitter_Side.h - emit particles from one side of the box
 */

#ifndef emitter_side_h
#define emitter_side_h

#include "Emitter_Abstract.h"
#include "application.h"

class Emitter_Side : public Emitter_Abstract {
public:
    static byte baseHue;
    static byte maxTtl;
    unsigned int counter;
    char side;
    uint8_t minVelocity;
    uint8_t maxVelocity;

    Emitter_Side(char side);
    Emitter_Side(char side, uint8_t minVelocity, uint8_t maxVelocity);
    void emit(Particle_Abstract *particle, ParticleSysConfig *g);
};

#endif /* emitter_side_h */
