//
//  BeatCircle.h
//  Ribbons
//
//  Created by Bryce Matsumori on 10/10/14.
//
//

#ifndef __Ribbons__BeatCircle__
#define __Ribbons__BeatCircle__

#include "cinder/TriMesh.h"
#include "RibbonsApp.h"

class BeatCircle {
public:
    explicit BeatCircle();

    void update(float delta);
    bool isDone() const;
    const ci::TriMesh &getMesh();

private:
    float mRadius = 0;
    ci::TriMesh mMesh;
};

#endif /* defined(__Ribbons__BeatCircle__) */
