#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"

#include "RibbonsApp.h"
#include "Resources.h"
#include "RibbonManager.h"
#include "AudioProcessor.h"

#define AUDIO_FILE "/Users/bryce/Documents/Dev/Ribbons/assets/audio.m4a"

using namespace ci;
using namespace ci::app;

struct BeatCircle;

class RibbonsApp : public AppNative {
public:
    explicit RibbonsApp();

    void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();

private:
    CameraPersp mCam;
    Vec3f mEye, mCenter, mUp;
    float mElapsed;
    std::vector<BeatCircle> beats;

    RibbonManager mRibbons;
    AudioProcessor mProcessor;
};

struct BeatCircle {
    Vec3f center;
    float radius;
};

RibbonsApp::RibbonsApp()
    : mElapsed(0) {
}

void RibbonsApp::prepareSettings(Settings *settings) {
    settings->setWindowSize(1024, 768);
    settings->setFrameRate(60.0f);
}

void RibbonsApp::setup() {
    mCam.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 200.0f);
    mCam.lookAt(Vec3f(0, 0, CAMERA_DISTANCE), Vec3f::zero());

#if 0
    fs::path file = getOpenFilePath("~/Music");
    if (file == "") exit(1);
#else
    fs::path file(AUDIO_FILE);
#endif

    mProcessor.init(ci::DataSourcePath::create(file));
    mRibbons.setProcessor(&mProcessor);

    mProcessor.start();
}

void RibbonsApp::update() {
    float now = getElapsedSeconds();

    mProcessor.update(now);
    mRibbons.update(now - mElapsed);

    for (size_t i = 0; i < beats.size(); ++i) {
        BeatCircle &beat = beats.at(i);
        beat.radius += 0.4f;
        if (beat.radius > 8.0f) {
            beats.erase(beats.begin() + i);
            --i;
        }
    }

    if (mProcessor.isBeat()) {
        BeatCircle circle;
        circle.center = Vec3f(randf() * 50.0f - 25.0f, randf() * 50.0f - 25.0f, 0);
        circle.radius = 1.0f;
        beats.push_back(circle);
    }
    
    mElapsed = now;
}

void RibbonsApp::draw() {
    float vol = mProcessor.getVolume();
    mCam.setFov(60.0f - vol * 5.0f);

    gl::clear(Color(1.0f, 1.0f, 1.0f - vol * 0.4f), false);
    gl::setMatrices(mCam);

    for (auto circle : beats) {
        gl::color(0.f, 0.f, 1.f, 0.1f);
        gl::drawStrokedCircle(Vec2f(circle.center.x, circle.center.y), circle.radius);//)(circle.center, circle.radius, 20);
    }

    mRibbons.draw();
}

CINDER_APP_NATIVE(RibbonsApp, RendererGl)
