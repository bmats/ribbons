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

    void setPos(float x, float y);
    void setColor(ci::Color color);
    void setSpeed(float speed);
    void setAttr(float period, float amplitude, float length);
    void update(float delta);

    void buildMesh();
    ci::TriMesh *getMesh();
    
private:
    ci::TriMesh mMesh;
    ci::Color mColor;
    ci::Vec3f mPos;
    float mSpeed, mPeriod, mAmp, mLength;
};

#endif /* defined(__Ribbons__RibbonMesh__) */
