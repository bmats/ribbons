#if 1

#define VR

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"
#include "cinder/gl/Light.h"
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
    AudioProcessor mProcessor;

#ifdef VR
    gl::Fbo mFbo[ovrEye_Count];
    ovrHmd mHmd;
    OVR::Sizei mRecTexSize[ovrEye_Count];
    OVR::Sizei mRenderTargetSize;
    ovrGLTexture mEyeTexture[2];
#endif
};

RibbonsApp::RibbonsApp()
    : mElapsed(0) {
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
    if (file == "") exit(1);
#else
    fs::path file("/Users/bryce/Documents/Dev/Ribbons/assets/audio.mp3");
#endif

    mProcessor.init(ci::DataSourcePath::create(file));
    mRibbons.setProcessor(&mProcessor);

    mProcessor.start();
}

void RibbonsApp::keyDown(KeyEvent e) {
    switch (e.getCode()) {
        // Exit
        case KeyEvent::KEY_ESCAPE:
            shutdown();
            quit();
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

void RibbonsApp::draw() {
    float vol = mProcessor.getVolume();
    mCam.setFov(60.0f - vol * 5.0f);

    gl::enableAlphaBlending();
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

    gl::clear(ColorA(1.0f, 1.0f, 1.0f - vol * 0.4f), true);
    gl::pushMatrices();
    gl::setMatrices(mCam);

    for (auto circle : beats) {
        gl::draw(circle.getMesh());
    }

//    gl::color(1, 0, 0);
//    gl::drawSphere(Vec3f::zero(), 50);

    mRibbons.draw();

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else //////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Utilities.h"

#define OVR_OS_MAC
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OculusTestApp : public AppNative {
public:
    void prepareSettings(Settings *settings);
    void setup();
    void mouseDown(MouseEvent event);
    void keyDown(KeyEvent event);
    void update();
    void draw();

private:
    CameraStereo mCamera;
    gl::Fbo mFbo;

    ovrHmd mHmd;
    ovrSizei mEyeRes[2];
    ovrGLTexture mEyeTex[2];
    ovrEyeRenderDesc mEyeRdesc[2];
    ovrPosef mEyePose[2];

private:
    void createFbo(int width, int height);
    void render();
};

void OculusTestApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(1280, 800);
    settings->setTitle("Stereoscopic Rendering");
    settings->setFrameRate(300.0f);
}

void OculusTestApp::setup()
{
    // setup the camera
    mCamera.setEyePoint(Vec3f(0.2f, 1.3f, -11.5f));
    mCamera.setCenterOfInterestPoint(Vec3f(0.5f, 1.5f, -0.1f));
    mCamera.setWorldUp(Vec3f(0.0f, -1.0f, 0.0f));
    mCamera.setFov(60.0f);

    ovr_Initialize();
    mHmd = ovrHmd_Create(0);

    if (!mHmd)
    {
        mHmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    }

//    if (mHmd)
//    {
//        OutputDebugString(L"Got HMD: ");
//        switch (mHmd->Type) {
//            case ovrHmd_None:
//                OutputDebugString(L"None\n");
//                break;
//            case ovrHmd_DK1:
//                OutputDebugString(L"DK1\n");
//                break;
//            case ovrHmd_DKHD:
//                OutputDebugString(L"DKHD\n");
//                break;
//            case ovrHmd_DK2:
//                OutputDebugString(L"DK2\n");
//                break;
//            case ovrHmd_Other:
//                OutputDebugString(L"Other\n");
//                break;
//            default:
//                OutputDebugString(L"Unknown\n");
//                break;
//        }
//    }

    setWindowSize(mHmd->Resolution.w, mHmd->Resolution.h);
    setWindowPos(mHmd->WindowsPos.x, mHmd->WindowsPos.y);

    ovrHmd_ConfigureTracking(mHmd, ovrTrackingCap_Orientation |
                             ovrTrackingCap_Position |
                             ovrTrackingCap_MagYawCorrection, 0);

    ovrSizei eyeRes[2];
    eyeRes[0] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Left, mHmd->DefaultEyeFov[0], 1.0);
    eyeRes[1] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Right, mHmd->DefaultEyeFov[1], 1.0);

    createFbo(eyeRes[0].w + eyeRes[1].w, ci::math<int>::max(eyeRes[0].h, eyeRes[1].h));

    for (int i = 0; i < 2; i++)
    {
        mEyeTex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
        mEyeTex[i].OGL.Header.TextureSize.w = mFbo.getSize().x;
        mEyeTex[i].OGL.Header.TextureSize.h = mFbo.getSize().y;
        mEyeTex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : mFbo.getSize().x / 2.0;
        mEyeTex[i].OGL.Header.RenderViewport.Pos.y = 0;
        mEyeTex[i].OGL.Header.RenderViewport.Size.w = mFbo.getSize().x / 2.0;
        mEyeTex[i].OGL.Header.RenderViewport.Size.h = mFbo.getSize().y;

        mEyeTex[i].OGL.TexId = mFbo.getTexture(0).getId();
    }

    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.RTSize = mHmd->Resolution;
    cfg.OGL.Header.Multisample = 1;
    if (mHmd->HmdCaps & ovrHmdCap_ExtendDesktop) {
        console() << "running in \"extended desktop\" mode" << endl;
    }
    else
    {
//        HWND sys_win = GetActiveWindow();
//
//        cfg.OGL.Window = sys_win;
//        cfg.OGL.DC = NULL;
//        cfg.OGL.Window = sys_win;
//        ovrHmd_AttachToWindow(mHmd, sys_win, NULL, NULL);
//        console() << "could not initialize" << endl;
//        console() << "running in \"direct-hmd\" moden" << endl;
    }

    // enable low-persistence display and dynamic prediction for lattency compensation
    ovrHmd_SetEnabledCaps(mHmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    ovrEyeRenderDesc eyeRenderDescOut[2];
    unsigned int dcaps = ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp |
    ovrDistortionCap_Overdrive;

    if (!ovrHmd_ConfigureRendering(mHmd, &cfg.Config, dcaps, mHmd->DefaultEyeFov, eyeRenderDescOut)) {
        console() << "failed to configure distortion renderer" << endl;
    }

}

void OculusTestApp::createFbo(int width, int height)
{
    gl::Fbo::Format fmt;
    fmt.setMagFilter(GL_LINEAR);
    fmt.setMinFilter(GL_LINEAR);
    fmt.enableColorBuffer();
    fmt.enableDepthBuffer();
    fmt.setSamples(16);
    fmt.setCoverageSamples(16);

    mFbo = gl::Fbo(width, height, fmt);
}

void OculusTestApp::mouseDown( MouseEvent event )
{
}

void OculusTestApp::keyDown(KeyEvent event)
{
    switch (event.getCode())
    {
        case KeyEvent::KEY_ESCAPE:
            if (mHmd) {
                ovrHmd_Destroy(mHmd);
            }
            ovr_Shutdown();
            quit();
            break;
        default:
            ovrHSWDisplayState state;
            ovrHmd_GetHSWDisplayState(mHmd, &state);
            if (state.Displayed)
            {
                ovrHmd_DismissHSWDisplay(mHmd);
            }
            break;
    }
}

void OculusTestApp::update()
{
    // Extract Oculus Orientation and Update Camera
    ovrTrackingState ts = ovrHmd_GetTrackingState(mHmd, ovr_GetTimeInSeconds());
    ovrPosef pose = ts.HeadPose.ThePose;

    ovrQuatf orientation = pose.Orientation;
    Quatf ci_quat(orientation.w, orientation.x, orientation.y, orientation.z);
    mCamera.setOrientation(ci_quat * Quatf(Vec3f(0, 1, 0), M_PI));
}

void OculusTestApp::draw()
{
    Vec2i size = mFbo.getSize();
//    ovrHmd_BeginFrame(mHmd, 0);
//    mFbo.bindFramebuffer();

    gl::enableDepthRead();
    gl::enableDepthWrite();

    // clear out the window with black
    gl::clear(Color(0, 0.4, 0));

    // store current viewport
//    glPushAttrib(GL_VIEWPORT_BIT);

    for (int i = 0; i < 2; i++)
    {
        ovrEyeType eye = mHmd->EyeRenderOrder[i];
        if (eye == ovrEye_Left)
        {
            // draw to left half of window only
            gl::setViewport(Area(0, 0, size.x / 2, size.y));

            // render left camera
            mCamera.enableStereoLeft();
        }
        else
        {
            // draw to right half of window only
            gl::setViewport(Area(size.x / 2, 0, size.x, size.y));

            // render right camera
            mCamera.enableStereoRight();
        }
//        mEyePose[i] = ovrHmd_GetEyePose(mHmd, eye);

        render();
    }

    // restore viewport
//    glPopAttrib();
//    mFbo.unbindFramebuffer();

//    ovrHmd_EndFrame(mHmd, mEyePose, &mEyeTex[0].Texture);

    gl::setMatricesWindow(getWindowSize());
    gl::draw(mFbo.getTexture(0), Rectf(0, 0, getWindowSize().x, getWindowSize().y));
}

void OculusTestApp::render()
{
    // enable 3D rendering
    gl::enableDepthRead();
    gl::enableDepthWrite();

    // set 3D camera matrices
//    gl::pushMatrices();
    gl::setMatrices(mCamera);

//    // draw grid
    gl::color(Color(0.8f, 0.8f, 0.8f));
    for (int i = -100; i <= 100; ++i) {
        gl::drawLine(Vec3f((float)i, 0, -100), Vec3f((float)i, 0, 100));
        gl::drawLine(Vec3f(-100, 0, (float)i), Vec3f(100, 0, (float)i));
    }

    // draw floor
    gl::enableAlphaBlending();
    gl::color(ColorA(1, 1, 1, 0.75f));
    gl::drawCube(Vec3f(0.0f, -0.5f, 0.0f), Vec3f(200.0f, 1.0f, 200.0f));

    gl::color(ColorA(1, 0.5, 0, 0.75f));
    gl::drawTorus(2.0f, 1.0f);
    gl::disableAlphaBlending();


    // restore 2D rendering
//    gl::popMatrices();
    gl::disableDepthWrite();
    gl::disableDepthRead();
}

CINDER_APP_NATIVE( OculusTestApp, RendererGl )

#endif
