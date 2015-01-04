#define VR
#define ENABLE_FILE_CHOOSER

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
    ~RibbonsApp();

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
    std::vector<BeatCircle> mBeats;

    RibbonManager mRibbons;
    PitchCircle mPitchCircle;
    AudioProcessor mProcessor;
    Starfield mStarfield;

    ColorA mClearColor;
    Anim<ColorA> mClearColorAnim;

    bool mIsNightModeEnabled = false;
    Anim<float> mNightModeOpacityAnim;

    bool mIsPaused = false;
    bool mIsTrailsEnabled = false;

#ifdef VR
    gl::Fbo          *mFbo = nullptr;
    ovrHmd            mHmd;
    OVR::Sizei        mRenderTargetSize;
    OVR::Sizei        mRecTexSize[2];
    ovrGLTexture      mEyeTexture[2];
    ovrEyeRenderDesc  mEyeRenderDesc[2];
    ovrVector3f       mHmdToEyeViewOffset[2];
#endif
};

RibbonsApp::RibbonsApp()
    : mElapsed(0),
      mClearColor(ColorA::white()) {
}

RibbonsApp::~RibbonsApp() {
#ifdef VR
    if (mFbo) {
        delete mFbo;
    }
#endif
}

void RibbonsApp::prepareSettings(Settings *settings) {
#ifdef VR
    // Use the Rift display
    size_t numDisplays = Display::getDisplays().size();
    settings->setDisplay(Display::getDisplays()[numDisplays - 1]);

    settings->setFullScreen(true);
    settings->setFrameRate(60.0f);
#else
    settings->setWindowSize(1024, 768);
    settings->setFrameRate(60.0f);
#endif
}

void RibbonsApp::setup() {
    // Setup camera
    mCam.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 200.0f);
    mCam.setEyePoint(Vec3f(0, 0, CAMERA_DISTANCE));
    mCam.setCenterOfInterestPoint(Vec3f::zero());

#ifdef VR
    // Init Rift
    ovr_Initialize();
    printf("Detected %i HMDs.\n", ovrHmd_Detect());

    mHmd = ovrHmd_Create(0);
    if (!mHmd) {
        printf("Using debug DK2.\n");
        mHmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    }
    if (mHmd->ProductName[0] == '\0') {
        printf("HMD detected, display not enabled.\n");
    }

    printf("Found a %s.\n", mHmd->ProductName);

    ovrHmd_ConfigureTracking(mHmd, ovrTrackingCap_MagYawCorrection |
                             ovrTrackingCap_Orientation |
                             ovrTrackingCap_Position, 0);

    for (int i = 0; i < ovrEye_Count; ++i) {
        ovrEyeType eye = mHmd->EyeRenderOrder[i];
        // TODO: how is this scaled down?
        mRecTexSize[i] = ovrHmd_GetFovTextureSize(mHmd, eye, mHmd->DefaultEyeFov[eye], 1.0f);
//        mRecTexSize[i] = OVR::Sizei(960, 1080);
    }

    // Use total width and max height for FBO
    mRenderTargetSize.w = mRecTexSize[0].w + mRecTexSize[1].w;
    mRenderTargetSize.h = fmax(mRecTexSize[0].h, mRecTexSize[1].h);

    gl::Fbo::Format fboFormat;
    fboFormat.enableColorBuffer();
    fboFormat.enableDepthBuffer();
//    fboFormat.setSamples(8);

    // Create the FBO
    mFbo = new gl::Fbo(mRenderTargetSize.w, mRenderTargetSize.h, fboFormat);

    // Setup the textures using the FBO
    mEyeTexture[0].OGL.Header.API            = ovrRenderAPI_OpenGL;
    mEyeTexture[0].OGL.Header.TextureSize    = mRenderTargetSize;
    mEyeTexture[0].OGL.Header.RenderViewport = OVR::Recti(0, 0, mRecTexSize[0].w, mRecTexSize[0].h);
    mEyeTexture[0].OGL.TexId                 = mFbo->getTexture().getId();

    mEyeTexture[1] = mEyeTexture[0];
    mEyeTexture[1].OGL.Header.RenderViewport = OVR::Recti(mRecTexSize[0].w, 0, mRecTexSize[1].w, mRecTexSize[1].h);

    ovrGLConfig cfg;
    cfg.OGL.Header.API            = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.BackBufferSize = mRenderTargetSize;
    cfg.OGL.Header.Multisample    = 0;

    ovrHmd_SetEnabledCaps(mHmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    if (!ovrHmd_ConfigureRendering(mHmd,
                                   &cfg.Config,
                                   ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
                                   mHmd->DefaultEyeFov,
                                   mEyeRenderDesc)) {
        printf("ConfigureRendering failed.\n");
        exit(1);
    }

    for (int i = 0; i < 2; ++i) {
        mHmdToEyeViewOffset[mEyeRenderDesc[i].Eye] = mEyeRenderDesc[i].HmdToEyeViewOffset;
    }
#endif // VR

    // Load the file
#ifdef ENABLE_FILE_CHOOSER
    fs::path file = getOpenFilePath("~/Music");
    if (file.empty()) exit(1);
#else
    fs::path file("/Users/bryce/Documents/Dev/Ribbons/assets/audio.mp3");
#endif

    mProcessor.init(ci::DataSourcePath::create(file));
    mRibbons.setProcessor(&mProcessor);
//    mPitchCircle.setProcessor(&mProcessor);
    mStarfield.setProcessor(&mProcessor);

    mProcessor.start();
}

void RibbonsApp::keyDown(KeyEvent e) {
#ifdef VR
    // Hide HSW on key press
    ovrHSWDisplayState state;
    ovrHmd_GetHSWDisplayState(mHmd, &state);
    if (state.Displayed) ovrHmd_DismissHSWDisplay(mHmd);
#endif

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

        // Pause
        case KeyEvent::KEY_p:
            mIsPaused = !mIsPaused;
            mProcessor.setPaused(mIsPaused);
            break;

        // Trails toggle
        case KeyEvent::KEY_t:
            mIsTrailsEnabled = !mIsTrailsEnabled;
            break;
    }
}

void RibbonsApp::update() {
    float now = getElapsedSeconds();
    float delta = now - mElapsed;

    if (!mIsPaused) {
        // Update all of the objects

        mProcessor.update(now);
        mRibbons.update(delta);

        for (size_t i = 0; i < mBeats.size(); ++i) {
            BeatCircle &beat = mBeats.at(i);
            beat.update(delta);
            if (beat.isDone()) {
                mBeats.erase(mBeats.begin() + i);
                --i;
            }
        }

//        mPitchCircle.update(delta);

        if (mIsNightModeEnabled || !mNightModeOpacityAnim.isComplete()) {
            mStarfield.update(delta);
        }

        // Add beats
        if (mProcessor.isBeat()) {
            BeatCircle circle;
            mBeats.push_back(circle);
        }
    }

    mElapsed = now;
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

#ifdef VR
    ovrHmd_BeginFrame(mHmd, 0);

    ovrPosef headPose[2];
    ovrHmd_GetEyePoses(mHmd, 0, mHmdToEyeViewOffset, headPose, nullptr);

    Area prevViewport = gl::getViewport();

    mFbo->bindFramebuffer();

    for (int i = 0; i < ovrEye_Count; ++i) { // get it... i... haha
        ovrEyeType eye = mHmd->EyeRenderOrder[i];

        ovrRecti *view = &mEyeTexture[eye].OGL.Header.RenderViewport;
        ovrPosef *pose = &headPose[eye];

        // Set camera orientation
        ovrQuatf *rot = &headPose[eye].Orientation;
        mCam.setOrientation(Quatf(rot->w, rot->x, rot->y, rot->z) * Quatf(Vec3f(0, 1, 0), 0));

        // Set camera position
        // TODO: fix position scaling
        Vec3f eyePointOffset = Vec3f(pose->Position.x, pose->Position.y, pose->Position.z) * 2.0f;
        mCam.setEyePoint(Vec3f(0, 0, CAMERA_DISTANCE) + eyePointOffset);

        // Configure camera properties
        mCam.setAspectRatio(view->Size.h / view->Size.w);
        mCam.setFov((atanf(mEyeRenderDesc[eye].Fov.LeftTan) + atanf(mEyeRenderDesc[eye].Fov.RightTan)) * 180.0f / M_PI);
        gl::setViewport(Area(view->Pos.x, view->Pos.y, view->Pos.x + view->Size.w, view->Pos.y + view->Size.h));

//        gl::enableDepthRead();
//        gl::enableDepthWrite();
//        glPushAttrib(GL_VIEWPORT_BIT);
#endif

        // Clear
        if (mClearColorAnim.isComplete()) {
            if (mIsNightModeEnabled) {
            }
            else {
                mClearColor.b = 1.0f - vol * 0.4f;
            }

            clear(mClearColor, mIsTrailsEnabled);
        }
        else {
            clear(mClearColorAnim.value(), mIsTrailsEnabled);
        }

        gl::pushMatrices();
        gl::setMatrices(mCam);

        if (mIsNightModeEnabled || !mNightModeOpacityAnim.isComplete()) {
            mStarfield.setOpacity(mNightModeOpacityAnim.value());
            mStarfield.draw();
        }

#ifndef VR
        // "Pulse" with the volume, but this wouldn't work with a HMD
        mCam.setFov(60.0f - vol * 5.0f);
        gl::setMatrices(mCam);
#endif

        for (auto circle : mBeats) {
            gl::draw(circle.getMesh());
        }

        mRibbons.draw();

//        mPitchCircle.draw();

        gl::popMatrices();

#ifdef VR
    }
    
    mFbo->unbindFramebuffer();
    gl::setViewport(prevViewport);
    
    ovrHmd_EndFrame(mHmd, headPose, &mEyeTexture[0].Texture);
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
