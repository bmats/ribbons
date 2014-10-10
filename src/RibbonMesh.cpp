//
//  RibbonMesh.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 9/27/14.
//
//

#include "RibbonMesh.h"

// Accuracy of ribbon, # quads / 1 unit length
#define RIBBON_QUAD_PER_LENGTH 2

#define PI 3.141592653589793f

#define RIBBON_WIDTH 3.0f

RibbonMesh::RibbonMesh()
    : mColor(0.5f, 0.5f, 0.5f) {
}

void RibbonMesh::setPos(float x, float y) {
    mPos.x = x;
    mPos.y = y;
}

void RibbonMesh::setColor(ci::Color color) {
    mColor = color;
}

void RibbonMesh::setSpeed(float speed) {
    mSpeed = speed;
}

void RibbonMesh::setAttr(float period, float amplitude, float length) {
    mPeriod = period;
    mAmp    = amplitude;
    mLength = length;
}

void RibbonMesh::update(float delta) {
    mPos.z += mSpeed * delta;
}

void RibbonMesh::buildMesh() {
    int nQuads = (int)roundf(mLength * RIBBON_QUAD_PER_LENGTH);
    float quadLen = mLength / nQuads;
    float startZ = mPos.z + quadLen * 0.5f;

    mMesh.clear();

    for (int i = 0; i < nQuads; ++i) {
        float yOff     = mAmp * sinf((startZ + (nQuads - i)     * quadLen) * PI / mPeriod),
              nextYOff = mAmp * sinf((startZ + (nQuads - i - 1) * quadLen) * PI / mPeriod);
        int idx = mMesh.getNumVertices();

        mMesh.appendVertex(ci::Vec3f(mPos.x,
                                     mPos.y + yOff,
                                     startZ - i * quadLen));
        mMesh.appendColorRgb(mColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x + RIBBON_WIDTH,
                                     mPos.y + yOff,
                                     startZ - i * quadLen));
        mMesh.appendColorRgb(mColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x + RIBBON_WIDTH,
                                     mPos.y + nextYOff,
                                     startZ - (i + 1) * quadLen));
        mMesh.appendColorRgb(mColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x,
                                     mPos.y + nextYOff,
                                     startZ - (i + 1) * quadLen));
        mMesh.appendColorRgb(mColor);

        mMesh.appendTriangle(idx + 0, idx + 1, idx + 2);
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 3);

        // backface?
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 1);
        mMesh.appendTriangle(idx + 0, idx + 3, idx + 2);
    }
}

ci::TriMesh *RibbonMesh::getMesh() {
    return &mMesh;
}
