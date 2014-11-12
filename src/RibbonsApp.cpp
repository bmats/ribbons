#undef VR

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"
#include "cinder/Timeline.h"

#ifdef VR
#define OVR_OS_MAC
#include "OVR.h"
#include "OVR_CAPI_GL.h"
#endif

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "RibbonsApp.h"
#include "Resources.h"
#include "RibbonManager.h"
#include "AudioProcessor.h"
#include "BeatCircle.h"
#include "PitchCircle.h"
#include "Starfield.h"

#define NIGHT_CLEAR_COLOR ColorA(0.124f, 0.179f, 0.35f)
#define TRAILS_ALPHA 0.25f

using namespace ci;
using namespace ci::app;

class RibbonsApp : public AppNative {
public:
    explicit RibbonsApp();

    void prepareSettings(Settings *settings);
    void setup();
    void keyDown(KeyEvent event);
    void update();
    void draw();
#ifdef VR
    void shutdown();
#endif

private:
    CameraPersp mCam;
    Vec3f mEye, mCenter, mUp;
    float mElapsed;
    std::vector<BeatCircle> beats;

    RibbonManager mRibbons;
    PitchCircle mPitchCircle;
    AudioProcessor mProcessor;
    Starfield mStarfield;

    ColorA mClearColor;
    Anim<ColorA> mClearColorAnim;

    bool mIsNightModeEnabled = false;
    Anim<float> mNightModeOpacityAnim;

    bool mIsTrailsEnabled = false;

#ifdef VR
    gl::Fbo mFbo[ovrEye_Count];
    ovrHmd mHmd;
    OVR::Sizei mRecTexSize[ovrEye_Count];
    OVR::Sizei mRenderTargetSize;
    ovrGLTexture mEyeTexture[2];
#endif
};

RibbonsApp::RibbonsApp()
    : mElapsed(0),
      mClearColor(ColorA::white()) {
}

void RibbonsApp::prepareSettings(Settings *settings) {
#ifdef VR
//    size_t numDisplays = Display::getDisplays().size();
//    if (numDisplays > 1) {
//        settings->setDisplay(Display::getDisplays()[numDisplays - 1]);
//    }

    settings->setFullScreen(true);
    settings->setFrameRate(20.0f);
#else
    settings->setWindowSize(1024, 768);
    settings->setFrameRate(60.0f);
#endif
}

void RibbonsApp::setup() {
    // Rendering
    mCam.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 200.0f);
    mCam.setEyePoint(Vec3f(0, 0, CAMERA_DISTANCE));
    mCam.setCenterOfInterestPoint(Vec3f::zero());

#ifdef VR
    mCam.setFov(60.0f);

    // Rift init
    ovr_Initialize();
    printf("Detected %i HMDs.\n", ovrHmd_Detect());

    mHmd = ovrHmd_Create(0);
    if (!mHmd) {
        printf("Using debug DK2.\n");
        mHmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    }
    if (mHmd->ProductName[0] == '\0') {
        printf("HMD detected, display not enabled.\n");
        exit(1);
    }

    printf("Found a %s.\n", mHmd->ProductName);

#ifdef OVR_OS_WIN32
    ovrHmd_AttachToWindow(mHmd, window, NULL, NULL);
#endif

    ovrHmd_ConfigureTracking(mHmd, ovrTrackingCap_MagYawCorrection |
                             ovrTrackingCap_Orientation |
                             ovrTrackingCap_Position, 0);

    gl::Fbo::Format fboFormat;
    fboFormat.enableColorBuffer();
    fboFormat.enableDepthBuffer();
//    fboFormat.setSamples(8);
    fboFormat.setMagFilter(GL_LINEAR);
    fboFormat.setMinFilter(GL_LINEAR);
    fboFormat.setSamples(16);
    fboFormat.setCoverageSamples(16);

//    mRecTexSize[0] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Left, mHmd->DefaultEyeFov[0], 1.0);
//    mRecTexSize[1] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Right, mHmd->DefaultEyeFov[1], 1.0);

//    mFbo[0] = gl::Fbo(mRecTexSize[0].w, mRecTexSize[0].h, fboFormat);
//    mFbo[1] = gl::Fbo(mRecTexSize[1].w, mRecTexSize[1].h, fboFormat);

    for (int i = 0; i < ovrEye_Count; ++i) {
        ovrEyeType eye = mHmd->EyeRenderOrder[i];

        mRecTexSize[i] = ovrHmd_GetFovTextureSize(mHmd, eye, mHmd->DefaultEyeFov[i], 1.0f);
        mFbo[i]        = gl::Fbo(mRecTexSize[i].w, mRecTexSize[i].h, fboFormat);

        mEyeTexture[i].OGL.Header.API            = ovrRenderAPI_OpenGL;
        mEyeTexture[i].OGL.Header.TextureSize    = mRecTexSize[i];
        mEyeTexture[i].OGL.Header.RenderViewport = OVR::Recti(0, 0, mRecTexSize[i].w, mRecTexSize[i].h);
        mEyeTexture[i].OGL.TexId                 = mFbo[i].getTexture().getId();
    }

//    setWindowSize(mHmd->Resolution.w, mHmd->Resolution.h);
//    setWindowPos(mHmd->WindowsPos.x, mHmd->WindowsPos.y);

    // total width, max height
    mRenderTargetSize.w = mRecTexSize[0].w + mRecTexSize[1].w;
    mRenderTargetSize.h = (mRecTexSize[0].h > mRecTexSize[1].h) ? mRecTexSize[0].h : mRecTexSize[1].h;

    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.OGL.Header.API         = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.RTSize      = mHmd->Resolution;//mRenderTargetSize;//OVR::Sizei(mHmd->Resolution.w, mHmd->Resolution.h);
    cfg.OGL.Header.Multisample = 1;
#ifdef OVR_OS_WIN32
    cfg.OGL.Window             = ?;
    cfg.OGL.DC                 = ?;
#endif

    ovrHmd_SetEnabledCaps(mHmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    ovrEyeRenderDesc eyeRenderDescOut[2];

    if (!ovrHmd_ConfigureRendering(mHmd,
                                   &cfg.Config,
                                   ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
                                   mHmd->DefaultEyeFov,
                                   eyeRenderDescOut)) {
        printf("ConfigureRendering failed.\n");
        exit(1);
    }
#endif // VR

    // Visualizer
#if 0
    fs::path file = getOpenFilePath("~/Music");
    if (file.empty()) exit(1);
#else
    fs::path file("/Users/bryce/Documents/Dev/Ribbons/assets/audio.mp3");
#endif

    mProcessor.init(ci::DataSourcePath::create(file));
    mRibbons.setProcessor(&mProcessor);
    mPitchCircle.setProcessor(&mProcessor);
    mStarfield.setProcessor(&mProcessor);

    mProcessor.start();
}

void RibbonsApp::keyDown(KeyEvent e) {
    switch (e.getCode()) {
        // Exit
        case KeyEvent::KEY_ESCAPE:
            shutdown();
            quit();
            break;

        // Night mode toggle
        case KeyEvent::KEY_n: {
            mIsNightModeEnabled = !mIsNightModeEnabled;
            ColorA prevColor = mClearColor;
            if (mIsNightModeEnabled)
                mClearColor = NIGHT_CLEAR_COLOR;
            else
                mClearColor = ColorA::white();

            mClearColorAnim = prevColor;
            timeline().apply(&mClearColorAnim, mClearColor, 0.5f);

            // Night stars opacity
            mNightModeOpacityAnim = mIsNightModeEnabled ? 0.0f : 1.0f;
            timeline().apply(&mNightModeOpacityAnim, mIsNightModeEnabled ? 1.0f : 0.0f, 0.5f);
            break;
        }

        // Trails toggle
        case KeyEvent::KEY_t:
            mIsTrailsEnabled = !mIsTrailsEnabled;
            break;

#ifdef VR
        default:
            // Hide HSW on key press
            ovrHSWDisplayState state;
            ovrHmd_GetHSWDisplayState(mHmd, &state);
            if (state.Displayed) ovrHmd_DismissHSWDisplay(mHmd);
            break;
#endif
    }
}

void RibbonsApp::update() {
    float now = getElapsedSeconds();
    float delta = now - mElapsed;

//    mCam.lookAt(Vec3f(sinf(now * 4) * 8, 0, CAMERA_DISTANCE), Vec3f::zero());

    mProcessor.update(now);
    mRibbons.update(delta);

    for (size_t i = 0; i < beats.size(); ++i) {
        BeatCircle &beat = beats.at(i);
        beat.update(delta);
        if (beat.isDone()) {
            beats.erase(beats.begin() + i);
            --i;
        }
    }

    if (mProcessor.isBeat()) {
        BeatCircle circle;
        beats.push_back(circle);
    }

    mPitchCircle.update(delta);

    if (mIsNightModeEnabled || !mNightModeOpacityAnim.isComplete()) {
        mStarfield.update(delta);
    }

    mElapsed = now;

#if 0//def VR
    // Update camera position
    ovrTrackingState ts = ovrHmd_GetTrackingState(mHmd, ovr_GetTimeInSeconds());

    if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) {
        OVR::Posef pose = ts.HeadPose.ThePose;

        OVR::Quatf rot = pose.Rotation;
        Quatf ci_quat(rot.w, rot.x, rot.y, rot.z);
        mCam.setOrientation(ci_quat * Quatf(Vec3f(0, 1, 0), M_PI));
    }
#endif
}

static void clear(ColorA color, bool alpha = false) {
    color.a = alpha ? TRAILS_ALPHA : 1.0f;
    gl::color(color);
    gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
}

void RibbonsApp::draw() {
    float vol = mProcessor.getVolume();

    gl::enableAlphaBlending();
//    gl::enableDepthRead();
//    gl::enableDepthWrite();
    gl::enable(GL_CULL_FACE);

    // undo ovr
//    gl::enable(GL_FRAMEBUFFER_SRGB); // or alt
//    gl::enable(GL_CULL_FACE);
//    gl::enable(GL_DEPTH_TEST);
//    gl::enable(GL_BLEND);
//    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//    gl::enable(GL_DITHER);
////    gl::enable(GL_RASTERIZER_DISCARD);
//    gl::enable(GL_SAMPLE_MASK);

#ifdef VR
    gl::clear(Color(0, 0, 0));

    ovrHmd_BeginFrame(mHmd, 0);

    ovrPosef headPose[2];

    for (int i = 0; i < ovrEye_Count; ++i) { // get it... i... haha
        ovrEyeType eye = mHmd->EyeRenderOrder[i];
        headPose[i] = ovrHmd_GetEyePose(mHmd, eye);

        mFbo[i].bindFramebuffer();
        gl::enableDepthRead();
        gl::enableDepthWrite();
        glPushAttrib(GL_VIEWPORT_BIT);
#endif

    if (mClearColorAnim.isComplete()) {
        if (mIsNightModeEnabled) {
        }
        else {
            mClearColor.b = 1.0f - vol * 0.4f;
        }

        clear(mClearColor, mIsTrailsEnabled);
//        gl::clear(mClearColor, false); // not using depth
    }
    else {
        clear(mClearColorAnim.value(), mIsTrailsEnabled);
//        gl::clear(mClearColorAnim.value(), false);
    }

    gl::pushMatrices();
    mCam.setFov(60.0f);
    gl::setMatrices(mCam);

    if (mIsNightModeEnabled || !mNightModeOpacityAnim.isComplete()) {
        mStarfield.setOpacity(mNightModeOpacityAnim.value());
        mStarfield.draw();
    }

    mCam.setFov(60.0f - vol * 5.0f);
    gl::setMatrices(mCam);

    for (auto circle : beats) {
        gl::draw(circle.getMesh());
    }

//    gl::color(1, 0, 0);
//    gl::drawSphere(Vec3f::zero(), 50);

    mRibbons.draw();

    mPitchCircle.draw();

    gl::popMatrices();

#ifdef VR
//        Surface s(mFbo[i].getTexture());
//        Color avg = s.areaAverage(Area(0, 0, s.getWidth(), s.getHeight()));
//        printf("%f %f %f\n", avg.r, avg.g, avg.b);

        gl::disableDepthWrite();
        gl::disableDepthRead();
        glPopAttrib();
        mFbo[i].unbindFramebuffer();
    }

    ovrHmd_EndFrame(mHmd, headPose, &mEyeTexture[0].Texture);

//    gl::setMatricesWindow(getWindowSize());
//    if (mFbo[0].getTexture())
//        gl::draw(mFbo[0].getTexture(), Rectf(0, 0, mFbo[0].getTexture().getWidth(), mFbo[0].getTexture().getHeight()));
//    gl::draw(mFbo[1].getTexture(), Rectf(mFbo[0].getTexture().getWidth(), 0, mFbo[1].getTexture().getWidth(), mFbo[1].getTexture().getHeight()));
#endif
}

#ifdef VR
void RibbonsApp::shutdown() {
    if (mHmd) {
        ovrHmd_Destroy(mHmd);
    }
    ovr_Shutdown();
}
#endif

CINDER_APP_NATIVE(RibbonsApp, RendererGl)
