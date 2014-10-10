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

class RibbonManager {
public:
    explicit RibbonManager();

    void update(float delta);
    void draw();

private:
    std::vector<RibbonMesh> ribbons;
};

#endif /* defined(__Ribbons__RibbonManager__) */
