//
//  RibbonManager.h
//  Ribbons
//
//  Created by Bryce Matsumori on 9/28/14.
//
//

#ifndef __Ribbons__RibbonManager__
#define __Ribbons__RibbonManager__

#include <vector>

#include "RibbonMesh.h"
#include "AudioProcessor.h"

typedef std::vector<RibbonMesh> RibbonBin;

class RibbonManager {
public:
    explicit RibbonManager();
    void setProcessor(AudioProcessor *processor);

    void update(float delta);
    void draw() const;

private:
    std::vector<RibbonBin> ribbons;

    AudioProcessor *mProcessor;
};

#endif /* defined(__Ribbons__RibbonManager__) */
