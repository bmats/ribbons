//
//  Starfield.h
//  Ribbons
//
//  Created by Bryce Matsumori on 11/12/14.
//
//

#ifndef __Ribbons__Starfield__
#define __Ribbons__Starfield__

#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include <vector>
#include "AudioProcessor.h"

struct Star {
    ci::Vec2f pos;
    float radius;
    size_t binId;
};

class Starfield {
public:
    explicit Starfield();
    void setProcessor(AudioProcessor *processor);

    void update(float delta);
    void draw();

    void setOpacity(float opacity);

private:
    AudioProcessor *mProcessor;
    float mOpacity = 0;

    std::vector<Star> mStars;
};

#endif /* defined(__Ribbons__Starfield__) */
