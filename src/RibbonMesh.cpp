//
//  RibbonMesh.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 9/27/14.
//
//

#include "RibbonMesh.h"
#include "RibbonsApp.h"

#define RADIAL_RIBBONS

// Accuracy of ribbon, # quads / 1 unit length
#define RIBBON_QUAD_PER_LENGTH 2

#define FADE_IN_DIST 10.0f

RibbonMesh::RibbonMesh()
    : mColor( ci::Color(0.5f, 0.5f, 0.5f) ) {

    mPeriod = mAmp = 0.0f;
    mWidth  = 1.0f;
    mSpeed  = 10.0f;
    mLength = 15.0f;

    reset();
}

void RibbonMesh::reset() {
    mAngle = randf() * 2.0f * PI;
    mDist  = 10.0f + randf() * 5.0f;

    mPos.x = cosf(mAngle) * mDist;
    mPos.y = sinf(mAngle) * mDist;
    mPos.z = randf() * -CAMERA_DISTANCE;

    mStartZ = mPos.z;
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

    if (mPos.z - mLength > CAMERA_DISTANCE) {
        reset();
    }

    // Fade the color in if necessary over a distance from white
    if (mPos.z - mStartZ > FADE_IN_DIST) {
        mDisplayColor = mColor;
    }
    else
        mDisplayColor = ci::ColorA(mColor, (mPos.z - mStartZ) / FADE_IN_DIST);

    // Build mesh
    int nQuads = (int)roundf(mLength * RIBBON_QUAD_PER_LENGTH);
    float quadLen = mLength / nQuads;

//    float slope = -1.0f/tanf(mAngle); // perp to radius

    mMesh.clear();

    for (int i = 0; i < nQuads; ++i) {
        float yOff     = mAmp * sinf((mPos.z + (nQuads - i)     * quadLen) * PI / mPeriod),
              nextYOff = mAmp * sinf((mPos.z + (nQuads - i - 1) * quadLen) * PI / mPeriod);
        size_t idx = mMesh.getNumVertices();

#ifdef RADIAL_RIBBONS
        ci::Vec3f quadCenter(cosf(mAngle) * (mDist + yOff),
                             sinf(mAngle) * (mDist + yOff),
                             mPos.z - i * quadLen);
        ci::Vec3f nextQuadCenter(cosf(mAngle) * (mDist + nextYOff),
                                 sinf(mAngle) * (mDist + nextYOff),
                                 mPos.z - (i + 1) * quadLen);

        mMesh.appendVertex(quadCenter + ci::Vec3f(-sinf(mAngle) * mWidth * -0.5f,
                                                  -cosf(mAngle) * mWidth * -0.5f,
                                                  0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(quadCenter + ci::Vec3f(-sinf(mAngle) * mWidth * 0.5f,
                                                  -cosf(mAngle) * mWidth * 0.5f,
                                                  0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(nextQuadCenter + ci::Vec3f(-sinf(mAngle) * mWidth * 0.5f,
                                                      -cosf(mAngle) * mWidth * 0.5f,
                                                      0.0f));
        mMesh.appendColorRgba(mDisplayColor);
        mMesh.appendVertex(nextQuadCenter + ci::Vec3f(-sinf(mAngle) * mWidth * -0.5f,
                                                      -cosf(mAngle) * mWidth * -0.5f,
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
