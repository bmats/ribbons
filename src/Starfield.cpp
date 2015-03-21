//
//  Starfield.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 11/12/14.
//
//

#include "Starfield.h"
#include "RibbonsApp.h"
#include "cinder/Rand.h"

#if VR
#define NUM_STARS 300
#else
#define NUM_STARS 50
#endif

Starfield::Starfield() {
    // Randomly distribute stars
    for (size_t i = 0; i < NUM_STARS; ++i) {
        Star s;
        s.binId = i % NUM_BIN_GROUPS;

#if VR
        // Get a random point on a sphere
        // see http://mathworld.wolfram.com/SpherePointPicking.html
        float rand = ci::randFloat(-1.0f, 1.0f), theta = ci::randFloat(2.0f * M_PI);

        s.pos = ci::Vec3f(sqrtf(1.0f - rand * rand) * cosf(theta) * CAMERA_DISTANCE,
                          rand * CAMERA_DISTANCE,
                          sqrtf(1.0f - rand * rand) * sinf(theta) * CAMERA_DISTANCE + CAMERA_DISTANCE);
#else
        s.pos = ci::Vec2f(std::rand() % 120 - 60, std::rand() % 90 - 45);
#endif

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
        if (it->radius > 0.8f)
            it->radius = 0.8f; // cap
    }
}

void Starfield::draw() {
    ci::gl::color(0.72f, 0.788f, 0.982f, mOpacity);

    for (auto it = mStars.begin(); it != mStars.end(); ++it) {
#if VR
        ci::gl::drawSphere(it->pos, it->radius, 8);
#else
        ci::gl::drawSolidCircle(it->pos, it->radius, 10);
#endif
    }
}

void Starfield::setOpacity(float opacity) {
    mOpacity = opacity;
}
