//
//  Starfield.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 11/12/14.
//
//

#include "Starfield.h"

#define NUM_STARS 50
#define NUM_BIN_GROUPS 8

Starfield::Starfield() {
    // randomly distribute stars
    for (size_t i = 0; i < NUM_STARS; ++i) {
        Star s;
        s.binId = i % NUM_BIN_GROUPS;
        s.pos = ci::Vec2f(std::rand() % 120 - 60, std::rand() % 90 - 45);

        mStars.push_back(s);
    }
}

void Starfield::setProcessor(AudioProcessor *processor) {
    mProcessor = processor;
}

void Starfield::update(float delta) {
    size_t nBins = mProcessor->getNumBins();
    size_t binSize = nBins / NUM_BIN_GROUPS;

    for (auto it = mStars.begin(); it != mStars.end(); ++it) {
        it->radius = mProcessor->getAmpForBins(it->binId * binSize, binSize) * 60.0f;
        if (it->radius > 0.8f) it->radius = 0.8f; // cap
    }
}

void Starfield::draw() {
    ci::gl::color(0.72f, 0.788f, 0.982f, mOpacity);

    for (auto it = mStars.begin(); it != mStars.end(); ++it) {
        ci::gl::drawSolidCircle(it->pos, it->radius, 10);
    }
}

void Starfield::setOpacity(float opacity) {
    mOpacity = opacity;
}
