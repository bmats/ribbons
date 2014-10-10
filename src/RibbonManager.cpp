//
//  RibbonManager.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 9/28/14.
//
//

#include "RibbonManager.h"
#include "cinder/gl/gl.h"
#include "cinder/Color.h"
#include "RibbonsApp.h"

#define NUM_BIN_GROUPS 8
#define RIBBONS_PER_BIN 8

#define PI 3.141592653589793f

static void randomizeRibbonPos(ci::Vec3f &pos) {
    float angle = randf() * 2.0f * pi;
    float dist  = 10.0f + randf() * 5.0f;

    pos.x = cos(angle) * dist;
    pos.y = sin(angle) * dist;
}

static ci::Color rainbowColor(float seed) {
    // see http://krazydad.com/tutorials/makecolors.php
    return ci::Color(
        (sin(0.025f * seed       ) * 127.0f + 128.0f) / 255.0f,
        (sin(0.025f * seed + 2.0f) * 127.0f + 128.0f) / 255.0f,
        (sin(0.025f * seed + 4.0f) * 127.0f + 128.0f) / 255.0f);
}

RibbonManager::RibbonManager() {
    for (uint b = 0; b < NUM_BIN_GROUPS; ++b) {
        RibbonBin bin;
//        ci::Color color(
//                        fmod(0.3f + b * 0.5f, 1),
//                        fmod(0.1f + b * 0.2f, 1),
//                        fmod(0.2f + b * 0.8f, 1));
//        ci::Color color(
//                        ((float)b / NUM_BIN_GROUPS) * 0.3f,
//                        ((float)b / NUM_BIN_GROUPS) * 0.6f,
//                        ((float)b / NUM_BIN_GROUPS) * 1.0f);
        ci::Color color = rainbowColor(b * 15.0f);

        for (uint r = 0; r < RIBBONS_PER_BIN + b; ++r) {
            RibbonMesh ribbon;
            ci::Vec3f &pos = ribbon.getPosVec();
            randomizeRibbonPos(pos);
            pos.z = randf() * -CAMERA_DISTANCE;
            ribbon.setStartZ(pos.z);

            // Dependent on bin
            ribbon.setWidth(3.0f - b * 0.3f);
            ribbon.setSpeed(8.0f + b * 0.5f);
            ribbon.setColor(color);

            bin.push_back(ribbon);
        }
        ribbons.push_back(bin);
    }

//    ribbon.setAttr(5.0f, 0.5f + sinf(getElapsedSeconds() * 2) * 2 * 0, 15.0f + sinf(getElapsedSeconds() * 2) * 5 * 0);
}

void RibbonManager::setProcessor(AudioProcessor *processor) {
    mProcessor = processor;
}

void RibbonManager::update(float delta) {
    size_t nBins = mProcessor->getNumBins();
    size_t binSize = nBins / NUM_BIN_GROUPS;

    size_t i = 0;
    for (auto binIt = ribbons.begin(); binIt != ribbons.end(); ++binIt) {
        float amp = mProcessor->getAmpForBins(i++ * binSize, binSize);

        for (auto ribbonIt = binIt->begin(); ribbonIt != binIt->end(); ++ribbonIt) {
            RibbonMesh &ribbon = *ribbonIt;
            ribbon.setAttr(5.0f, amp * 300.0f, 15.0f);

            ci::Vec3f &pos = ribbon.getPosVec();
            if (pos.z - ribbon.getLength() * 0.5f > CAMERA_DISTANCE) {
                randomizeRibbonPos(pos);
                ribbon.resetPosZ();
            }

            ribbon.update(delta);
        }
    }
}

//const ci::Vec3f &RibbonManager::getFirstPos() const {
//    return ribbons[0][0].getPos();
//}

void RibbonManager::draw() const {
    // TODO: combine calls to update and draw? Iterates twice.
    for (auto bin : ribbons) {
        // TODO: save all ribbons into one vector so we don't need to iterate bins
        for (auto ribbon : bin) {
            ci::gl::draw(ribbon.getMesh());
        }
    }
}
