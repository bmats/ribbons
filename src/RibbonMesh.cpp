//
//  RibbonMesh.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 9/27/14.
//
//

#include "RibbonMesh.h"
#include "RibbonsApp.h"
#include "cinder/Rand.h"

// Accuracy of ribbon, # quads / 1 unit length
#define RIBBON_QUAD_PER_LENGTH 2

#define FADE_IN_DIST 10.0f

RibbonMesh::RibbonMesh()
    : mColor( ci::Color(0.5f, 0.5f, 0.5f) ) {

    mPeriod = mAmp = 0.0f;
    mWidth  = 1.0f;
    mVel  = 10.0f;
    mLength = 15.0f;

    reset();
}

void RibbonMesh::reset() {
    mAngle = ci::randFloat(2.0f * M_PI);
    mDist  = 10.0f + ci::randFloat(5.0f);

    mPos.x = cosf(mAngle) * mDist;
    mPos.y = sinf(mAngle) * mDist;
#if VR
    mPos.z = ci::randFloat(-CAMERA_DISTANCE * 2);
#else
    mPos.z = ci::randFloat(-CAMERA_DISTANCE);
#endif

    mStartZ = mPos.z;
}

void RibbonMesh::setColor(ci::Color color) {
    mColor = color;
}

void RibbonMesh::setWidth(float width) {
    mWidth = width;
}

void RibbonMesh::setVel(float vel) {
    mVel = vel;
}

void RibbonMesh::setAttr(float period, float amplitude, float length) {
    mPeriod = period;
    mAmp    = amplitude;
    mLength = length;
}

void RibbonMesh::update(float delta) {
    // Ribbon velocity proportional to volume
    mPos.z += mVel * delta;

#if VR
    if (mPos.z - mLength > CAMERA_DISTANCE * 2) {
#else
    if (mPos.z - mLength > CAMERA_DISTANCE) {
#endif
        reset();
    }

    // Fade the color in or out over a distance from white
    if (mPos.z < FADE_IN_DIST) {
        mDisplayColor = ci::ColorA(mColor, (mPos.z - mStartZ) / FADE_IN_DIST);
    }
    else if ((CAMERA_DISTANCE * 2) - mPos.z < FADE_IN_DIST) {
        mDisplayColor = ci::ColorA(mColor, ((CAMERA_DISTANCE * 2) - mPos.z) / FADE_IN_DIST);
    }
    else {
        mDisplayColor = mColor;
    }

    // Build mesh
    int nQuads = (int)roundf(mLength * RIBBON_QUAD_PER_LENGTH);
    float quadLen = mLength / nQuads;

    mMesh.clear();

    for (int i = 0; i < nQuads; ++i) {
        float yOff     = mAmp * sinf((mPos.z + (nQuads - i)     * quadLen) * M_PI / mPeriod),
              nextYOff = mAmp * sinf((mPos.z + (nQuads - i - 1) * quadLen) * M_PI / mPeriod);
        size_t idx = mMesh.getNumVertices();

#if RADIAL_RIBBONS
        ci::Vec3f quadCenter(cosf(mAngle) * (mDist + yOff),
                             sinf(mAngle) * (mDist + yOff),
                             mPos.z - i * quadLen);
        ci::Vec3f nextQuadCenter(cosf(mAngle) * (mDist + nextYOff),
                                 sinf(mAngle) * (mDist + nextYOff),
                                 mPos.z - (i + 1) * quadLen);

        mMesh.appendVertex(quadCenter + ci::Vec3f(-cosf(M_PI * 0.5f + mAngle) * mWidth * -0.5f,
                                                  -sinf(M_PI * 0.5f + mAngle) * mWidth * -0.5f,
                                                  0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(quadCenter + ci::Vec3f(-cosf(M_PI * 0.5f + mAngle) * mWidth * 0.5f,
                                                  -sinf(M_PI * 0.5f + mAngle) * mWidth * 0.5f,
                                                  0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(nextQuadCenter + ci::Vec3f(-cosf(M_PI * 0.5f + mAngle) * mWidth * 0.5f,
                                                      -sinf(M_PI * 0.5f + mAngle) * mWidth * 0.5f,
                                                      0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(nextQuadCenter + ci::Vec3f(-cosf(M_PI * 0.5f + mAngle) * mWidth * -0.5f,
                                                      -sinf(M_PI * 0.5f + mAngle) * mWidth * -0.5f,
                                                      0.0f));
        mMesh.appendColorRgba(mDisplayColor);
#else
        mMesh.appendVertex(mPos + ci::Vec3f(0.0f,
                                            yOff,
                                            -i * quadLen));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(mPos + ci::Vec3f(mWidth,
                                            yOff,
                                            -i * quadLen));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(mPos + ci::Vec3f(mWidth,
                                            nextYOff,
                                            -(i + 1) * quadLen));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(mPos + ci::Vec3f(0.0f,
                                            nextYOff,
                                            -(i + 1) * quadLen));
        mMesh.appendColorRgba(mDisplayColor);
#endif

        mMesh.appendTriangle(idx + 0, idx + 1, idx + 2);
        mMesh.appendTriangle(idx + 0, idx + 2, idx + 3);

        // Backface
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
