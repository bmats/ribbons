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

#define FADE_IN_DIST 10.0f

#define PI 3.141592653589793f

RibbonMesh::RibbonMesh()
    : mColor( ci::Color(0.5f, 0.5f, 0.5f) ),
      mPos( ci::Vec3f::zero() ) {

    mStartZ = mPeriod = mAmp = 0.0f;
    mWidth  = 1.0f;
    mSpeed  = 10.0f;
    mLength = 15.0f;
}

void RibbonMesh::setAngle(float angle) {
    mAngle = angle;
}

void RibbonMesh::setPos(float x, float y, float z) {
    mPos.x = x;
    mPos.y = y;
    mPos.z = z;
}

void RibbonMesh::setStartZ(float z) {
    mStartZ = z;
}

void RibbonMesh::setColor(ci::Color color) {
    mColor = color;
}

void RibbonMesh::setWidth(float width) {
    mWidth = width;
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

    // Fade the color in if necessary over a distance from white
    if (mPos.z - mStartZ > FADE_IN_DIST) {
        mDisplayColor = mColor;
    }
    else
        mDisplayColor = ci::Color::white() - (ci::Color::white() - mColor) * (mPos.z - mStartZ) / FADE_IN_DIST;

    // Build mesh
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
        mMesh.appendColorRgb(mDisplayColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x + mWidth,
                                     mPos.y + yOff,
                                     startZ - i * quadLen));
        mMesh.appendColorRgb(mDisplayColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x + mWidth,
                                     mPos.y + nextYOff,
                                     startZ - (i + 1) * quadLen));
        mMesh.appendColorRgb(mDisplayColor);
        mMesh.appendVertex(ci::Vec3f(mPos.x,
                                     mPos.y + nextYOff,
                                     startZ - (i + 1) * quadLen));
        mMesh.appendColorRgb(mDisplayColor);

        mMesh.appendTriangle(idx + 0, idx + 1, idx + 2);
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 3);

        // backface?
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 1);
        mMesh.appendTriangle(idx + 0, idx + 3, idx + 2);
    }
}

const ci::TriMesh &RibbonMesh::getMesh() const {
    return mMesh;
}

ci::Vec3f &RibbonMesh::getPosVec() {
    return mPos;
}

float RibbonMesh::getLength() const {
    return mLength;
}

void RibbonMesh::resetPosZ() {
    mPos.z = mStartZ;
}
