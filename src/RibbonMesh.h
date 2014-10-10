//
//  RibbonMesh.h
//  Ribbons
//
//  Created by Bryce Matsumori on 9/27/14.
//
//

#ifndef __Ribbons__RibbonMesh__
#define __Ribbons__RibbonMesh__

#include "cinder/TriMesh.h"
#include "cinder/Vector.h"

class RibbonMesh {
public:
    explicit RibbonMesh();

    void setAngle(float angle);

    void setPos(float x, float y, float z);
    void setStartZ(float z);
    void setColor(ci::Color color);
    void setWidth(float width);
    void setSpeed(float speed);
    void setAttr(float period, float amplitude, float length);
    void update(float delta);
    void resetPosZ();

    const ci::TriMesh &getMesh() const;

    ci::Vec3f &getPosVec();
    float getLength() const;
    
private:
    ci::TriMesh mMesh;
    ci::Color mColor, mDisplayColor;
    ci::Vec3f mPos;
    float mStartZ, mAngle = 0, mWidth, mSpeed, mPeriod, mAmp, mLength;
};

#endif /* defined(__Ribbons__RibbonMesh__) */
