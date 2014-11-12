//
//  PitchCircle.h
//  Ribbons
//
//  Created by Bryce Matsumori on 11/10/14.
//
//

#ifndef __Ribbons__PitchCircle__
#define __Ribbons__PitchCircle__

#include "AudioProcessor.h"
#include "cinder/TriMesh.h"
#include "cinder/Vector.h"

class PitchCircle {
public:
    explicit PitchCircle();
    void setProcessor(AudioProcessor *processor);

    void update(float delta);
    void draw();

private:
    float mRadius = -1;
    ci::TriMesh mMesh;
    ci::Vec3f mCenter;

    AudioProcessor *mProcessor;
};

#endif /* defined(__Ribbons__PitchCircle__) */
