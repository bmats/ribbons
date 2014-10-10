//
//  RibbonManager.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 9/28/14.
//
//

#include "RibbonManager.h"
#include "cinder/gl/gl.h"

#define BIN_GROUP_SIZE 1024 / 10
#define RIBBONS_PER_BIN 5

RibbonManager::RibbonManager() {
//    ribbon.setPos(2.0f, -2.0f);
//    ribbon.setColor(Color(0.2f, 0.3f, 0.5f));
//    ribbon.setSpeed(10.0f);
//    ribbon.setAttr(5.0f, 0.5f, 15.0f);

//    ribbon.setAttr(5.0f, 0.5f + sinf(getElapsedSeconds() * 2) * 2 * 0, 15.0f + sinf(getElapsedSeconds() * 2) * 5 * 0);
}

void RibbonManager::update(float delta) {
    for (auto &ribbon : ribbons) {
        ribbon.update(delta);
        ribbon.buildMesh();
    }
}

void RibbonManager::draw() {
    // TODO: combine calls to update and draw? Iterates twice.
    for (auto &ribbon : ribbons) {
        ci::gl::draw(*ribbon.getMesh());
    }
}
