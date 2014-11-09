//
//  BeatCircle.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 10/10/14.
//
//

#include "BeatCircle.h"
#include "cinder/Vector.h"

#define EXPAND_RATE 14.0f
#define MAX_RADIUS 8.0f
#define MAX_SEGMENTS 20
#define BORDER_WIDTH 0.7f
#define CENTER ci::Vec3f::zero()

BeatCircle::BeatCircle()
    : mRadius(0) {
}

void BeatCircle::update(float delta) {
    mRadius += EXPAND_RATE * delta;

    float opacity = (MAX_RADIUS - mRadius) / MAX_RADIUS;
    ci::ColorA color(0.039f, 0.466f, 0.909f, 0.6f * opacity);

    // Build mesh
//    int nSegments = (int)(mRadius / MAX_RADIUS * MAX_SEGMENTS);

    mMesh.clear();

    float sectorAngle = 2.0f * M_PI / MAX_SEGMENTS;
    for (int i = 0; i < MAX_SEGMENTS; ++i) {
        float angle     = sectorAngle * i,
              nextAngle = sectorAngle * (i + 1);
        size_t idx = mMesh.getNumVertices();

        mMesh.appendVertex(CENTER + ci::Vec3f(cosf(angle) * mRadius,
                                              sinf(angle) * mRadius,
                                              0));
        mMesh.appendColorRgba(color);
        mMesh.appendVertex(CENTER + ci::Vec3f(cosf(angle) * (mRadius + BORDER_WIDTH),
                                              sinf(angle) * (mRadius + BORDER_WIDTH),
                                              0));
        mMesh.appendColorRgba(color);
        mMesh.appendVertex(CENTER + ci::Vec3f(cosf(nextAngle) * (mRadius + BORDER_WIDTH),
                                              sinf(nextAngle) * (mRadius + BORDER_WIDTH),
                                              0));
        mMesh.appendColorRgba(color);
        mMesh.appendVertex(CENTER + ci::Vec3f(cosf(nextAngle) * mRadius,
                                              sinf(nextAngle) * mRadius,
                                              0));
        mMesh.appendColorRgba(color);

        mMesh.appendTriangle(idx + 0, idx + 1, idx + 2);
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 3);
    }
}

bool BeatCircle::isDone() const {
    return mRadius >= MAX_RADIUS;
}

const ci::TriMesh &BeatCircle::getMesh() {
    return mMesh;
}