//
//  PitchCircle.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 11/10/14.
//
//

#include "PitchCircle.h"
#include "cinder/gl/gl.h"

#define MAX_FREQ 10000
#define LOW_PASS_RATIO 0.5f
#define MAX_SEGMENTS 20
#define BORDER_WIDTH 0.7f
#define COLOR ci::ColorA(0.479f, 0.083f, 0.627f, 0.5f)

PitchCircle::PitchCircle() {
    mCenter = ci::Vec3f(0, 0, 0);
}

void PitchCircle::setProcessor(AudioProcessor *processor) {
    mProcessor = processor;
}

void PitchCircle::update(float delta) {
    float freq = mProcessor->getMaxFreq(1000, 9000);
    float radius = freq / MAX_FREQ * 200.0f;

//    if (mRadius == -1.0f || mRadius > 100.0f) { // first or weird
//        mRadius = radius;
//    }
//    else {
        mRadius = mRadius * (1.0f - LOW_PASS_RATIO * delta) + radius * LOW_PASS_RATIO * delta;
//    }

    mMesh.clear();

    float sectorAngle = 2.0f * M_PI / MAX_SEGMENTS;
    for (int i = 0; i < MAX_SEGMENTS; ++i) {
        float angle     = sectorAngle * i,
        nextAngle = sectorAngle * (i + 1);
        size_t idx = mMesh.getNumVertices();

        mMesh.appendVertex(mCenter + ci::Vec3f(cosf(angle) * mRadius,
                                              sinf(angle) * mRadius,
                                              0));
        mMesh.appendColorRgba(COLOR);
        mMesh.appendVertex(mCenter + ci::Vec3f(cosf(angle) * (mRadius + BORDER_WIDTH),
                                              sinf(angle) * (mRadius + BORDER_WIDTH),
                                              0));
        mMesh.appendColorRgba(COLOR);
        mMesh.appendVertex(mCenter + ci::Vec3f(cosf(nextAngle) * (mRadius + BORDER_WIDTH),
                                              sinf(nextAngle) * (mRadius + BORDER_WIDTH),
                                              0));
        mMesh.appendColorRgba(COLOR);
        mMesh.appendVertex(mCenter + ci::Vec3f(cosf(nextAngle) * mRadius,
                                              sinf(nextAngle) * mRadius,
                                              0));
        mMesh.appendColorRgba(COLOR);

        mMesh.appendTriangle(idx + 0, idx + 1, idx + 2);
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 3);
    }
}

void PitchCircle::draw() {
    ci::gl::draw(mMesh);
}
