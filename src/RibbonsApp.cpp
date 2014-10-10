#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"

#include "Resources.h"
#include "RibbonManager.h"
#include "FrequencyProcessor.h"

#define AUDIO_FILE "/Users/bryce/Documents/Dev/Ribbons/assets/audio.mp3"

using namespace ci;
using namespace ci::app;

class RibbonsApp : public AppNative {
public:
    explicit RibbonsApp();

    void prepareSettings(Settings *settings);
	void setup();
    void mouseDown(MouseEvent event);
	void update();
	void draw();

private:
    CameraPersp mCam;
    Vec3f mEye, mCenter, mUp;
    double elapsed;

    RibbonManager mRibbons;
//    RibbonMesh ribbon;
    FrequencyProcessor mProcessor;
};

RibbonsApp::RibbonsApp()
    : elapsed(0) {
}

void RibbonsApp::prepareSettings(Settings *settings) {
    settings->setWindowSize(800, 600);
    settings->setFrameRate(60.0f);
}

void RibbonsApp::setup() {
    mCam.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 100.0f);
    mCam.lookAt(Vec3f(0, 0, 100.0f), Vec3f::zero());

//    ribbon.setPos(2.0f, -2.0f);
//    ribbon.setColor(Color(0.2f, 0.3f, 0.5f));
//    ribbon.setSpeed(10.0f);
//    ribbon.setAttr(5.0f, 0.5f, 15.0f);

    mProcessor.setSourceFile(ci::DataSourcePath::create(ci::fs::path(AUDIO_FILE)));
    mProcessor.start();
}

void RibbonsApp::update() {
    double now = getElapsedSeconds();
    mRibbons.update(now - elapsed);
    elapsed = now;

    mProcessor.update();
}

void RibbonsApp::draw() {
    gl::clear(Color(1.0f, 1.0f, 1.0f), true);
    gl::setMatrices(mCam);

    mRibbons.draw();
}

CINDER_APP_NATIVE(RibbonsApp, RendererGl)
