#undef VR

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
//#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"
#ifdef VR
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

#define AUDIO_FILE "/Users/bryce/Documents/Dev/Ribbons/assets/audio.mp3"

using namespace ci;
using namespace ci::app;

class RibbonsApp : public AppNative {
public:
    explicit RibbonsApp();

    void prepareSettings(Settings *settings);
	void setup();
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
#endif
};

RibbonsApp::RibbonsApp()
    : mElapsed(0) {
}

void RibbonsApp::prepareSettings(Settings *settings) {
#ifdef VR
    settings->setFullScreen(true);
    settings->setFrameRate(75.f);

//    size_t numDisplays = Display::getDisplays().size();
//    if (numDisplays > 1) {
//        settings->setDisplay(Display::getDisplays()[numDisplays - 1]);
//    }
#else
    settings->setWindowSize(1024, 768);
    settings->setFrameRate(60.0f);
#endif
}

void RibbonsApp::setup() {
    // Rendering
    mCam.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 200.0f);
    mCam.lookAt(Vec3f(0, 0, CAMERA_DISTANCE), Vec3f::zero());

//    gl::enableAlphaBlending();
//    gl::enable(GL_CULL_FACE);

#ifdef VR
    // Rift
    ovr_Initialize();
    printf("Detected %i HMDs.\n", ovrHmd_Detect());

    mHmd = ovrHmd_Create(0);
    if (!mHmd) {
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

    gl::Fbo::Format fboFormat;
    fboFormat.enableColorBuffer();
    fboFormat.enableDepthBuffer();
    fboFormat.setSamples(8);

    for (int i = 0; i < ovrEye_Count; ++i) {
        ovrEyeType eye = mHmd->EyeRenderOrder[i];

        mRecTexSize[i] = ovrHmd_GetFovTextureSize(mHmd, eye, mHmd->DefaultEyeFov[i], 1.0f);
        mFbo[i]     = gl::Fbo(mRecTexSize[i].w, mRecTexSize[i].h, fboFormat);
    }

    // total width, max height
    mRenderTargetSize.w = mRecTexSize[0].w + mRecTexSize[1].w;
    mRenderTargetSize.h = (mRecTexSize[0].h > mRecTexSize[1].h) ? mRecTexSize[0].h : mRecTexSize[1].h;

    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.OGL.Header.API         = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.RTSize      = mHmd->Resolution;//mRenderTargetSize;//OVR::Sizei(mHmd->Resolution.w, mHmd->Resolution.h);
    cfg.OGL.Header.Multisample = 1;
#ifdef OVR_OS_WIN32
    cfg.OGL.Window             = ?;
    cfg.OGL.DC                 = ?;
#endif

    ovrEyeRenderDesc eyeRenderDesc[2];

//    if (!ovrHmd_ConfigureRendering(mHmd,
//                              &cfg.Config,
//                              ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
//                              mHmd->DefaultEyeFov,
//                              eyeRenderDesc)) {
//        exit(1);
//    }

    ovrHmd_SetEnabledCaps(mHmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    ovrHmd_ConfigureTracking(mHmd,
                             ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Orientation | ovrTrackingCap_Position,
                             0);

//    ovrTrackingState ts = ovrHmd_GetTrackingState(mHmd, ovr_GetTimeInSeconds());
//
//    if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) {
//        OVR::Posef pose = ts.HeadPose.ThePose;
//    }

    ovrHmd_DismissHSWDisplay(mHmd);
#endif // VR

    // Visualizer
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
    float delta = now - mElapsed;

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
}

void RibbonsApp::draw() {
    float vol = mProcessor.getVolume();
    mCam.setFov(60.0f - vol * 5.0f);

#ifdef VR
//    gl::clear(Color(0, 0, 0));
//
//    ovrPosef headPose[2];
//    ovrGLTexture eyeTexture[2];
//
//    ovrHmd_BeginFrame(mHmd, 0);
//
//    for (int i = 0; i < ovrEye_Count; ++i) { // get it... i... haha
//        ovrEyeType eye = mHmd->EyeRenderOrder[i];
//        headPose[i] = ovrHmd_GetEyePose(mHmd, eye);
//
//        eyeTexture[i].OGL.Header.API            = ovrRenderAPI_OpenGL;
//        eyeTexture[i].OGL.Header.TextureSize    = mRecTexSize[i];
//        eyeTexture[i].OGL.Header.RenderViewport = OVR::Recti(0, 0, mRecTexSize[i].w, mRecTexSize[i].h);
//        eyeTexture[i].OGL.TexId                 = mFbo[i].getTexture().getId();
//
//        mFbo[i].bindFramebuffer();
#endif

    gl::clear(ColorA(1.0f, 1.0f, 1.0f - vol * 0.4f), true);
    gl::setMatrices(mCam);

    for (auto circle : beats) {
        gl::draw(circle.getMesh());
    }

    mRibbons.draw();

#ifdef VR
//        mFbo[i].unbindFramebuffer();
//    }
//
//    ovrHmd_EndFrame(mHmd, headPose, &eyeTexture[0].Texture);
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





//#include "cinder/app/AppNative.h"
//#include "cinder/gl/gl.h"
//
//#include "cinder/Camera.h"
//#include "cinder/gl/Fbo.h"
//#include "cinder/Utilities.h"
//
//#define OVR_OS_MAC
//#include "OVR_CAPI.h"
//#include "OVR_CAPI_GL.h"
//
//using namespace ci;
//using namespace ci::app;
//using namespace std;
//
//class OculusTestApp : public AppNative {
//public:
//    void prepareSettings(Settings *settings);
//    void setup();
//    void mouseDown(MouseEvent event);
//    void keyDown(KeyEvent event);
//    void update();
//    void draw();
//
//private:
//    CameraStereo mCamera;
//    gl::Fbo mFbo;
//
//    ovrHmd mHmd;
//    ovrSizei mEyeRes[2];
//    ovrGLTexture mEyeTex[2];
//    ovrEyeRenderDesc mEyeRdesc[2];
//    ovrPosef mEyePose[2];
//
//private:
//    void createFbo(int width, int height);
//    void render();
//};
//
//void OculusTestApp::prepareSettings(Settings *settings)
//{
//    settings->setWindowSize(1280, 800);
//    settings->setTitle("Stereoscopic Rendering");
//    settings->setFrameRate(300.0f);
//}
//
//void OculusTestApp::setup()
//{
//    // setup the camera
//    mCamera.setEyePoint(Vec3f(0.2f, 1.3f, -11.5f));
//    mCamera.setCenterOfInterestPoint(Vec3f(0.5f, 1.5f, -0.1f));
//    mCamera.setWorldUp(Vec3f(0.0f, -1.0f, 0.0f));
//    mCamera.setFov(60.0f);
//
//    ovr_Initialize();
//    mHmd = ovrHmd_Create(0);
//
////    if (!mHmd)
////    {
////        mHmd = ovrHmd_CreateDebug(ovrHmd_DK2);
////    }
//
////    if (mHmd)
////    {
////        OutputDebugString(L"Got HMD: ");
////        switch (mHmd->Type) {
////            case ovrHmd_None:
////                OutputDebugString(L"None\n");
////                break;
////            case ovrHmd_DK1:
////                OutputDebugString(L"DK1\n");
////                break;
////            case ovrHmd_DKHD:
////                OutputDebugString(L"DKHD\n");
////                break;
////            case ovrHmd_DK2:
////                OutputDebugString(L"DK2\n");
////                break;
////            case ovrHmd_Other:
////                OutputDebugString(L"Other\n");
////                break;
////            default:
////                OutputDebugString(L"Unknown\n");
////                break;
////        }
////    }
//
//    setWindowSize(mHmd->Resolution.w, mHmd->Resolution.h);
//    setWindowPos(mHmd->WindowsPos.x, mHmd->WindowsPos.y);
//
//    ovrHmd_ConfigureTracking(mHmd, ovrTrackingCap_Orientation |
//                             ovrTrackingCap_Position |
//                             ovrTrackingCap_MagYawCorrection, 0);
//
//    ovrSizei eyeRes[2];
//    eyeRes[0] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Left, mHmd->DefaultEyeFov[0], 1.0);
//    eyeRes[1] = ovrHmd_GetFovTextureSize(mHmd, ovrEye_Right, mHmd->DefaultEyeFov[1], 1.0);
//
//    createFbo(eyeRes[0].w + eyeRes[1].w, ci::math<int>::max(eyeRes[0].h, eyeRes[1].h));
//
//    for (int i = 0; i < 2; i++)
//    {
//        mEyeTex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
//        mEyeTex[i].OGL.Header.TextureSize.w = mFbo.getSize().x;
//        mEyeTex[i].OGL.Header.TextureSize.h = mFbo.getSize().y;
//        mEyeTex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : mFbo.getSize().x / 2.0;
//        mEyeTex[i].OGL.Header.RenderViewport.Pos.y = 0;
//        mEyeTex[i].OGL.Header.RenderViewport.Size.w = mFbo.getSize().x / 2.0;
//        mEyeTex[i].OGL.Header.RenderViewport.Size.h = mFbo.getSize().y;
//
//        mEyeTex[i].OGL.TexId = mFbo.getTexture(0).getId();
//    }
//
//    ovrGLConfig cfg;
//    memset(&cfg, 0, sizeof cfg);
//    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
//    cfg.OGL.Header.RTSize = mHmd->Resolution;
//    cfg.OGL.Header.Multisample = 1;
//    if (mHmd->HmdCaps & ovrHmdCap_ExtendDesktop) {
//        console() << "running in \"extended desktop\" mode" << endl;
//    }
//    else
//    {
////        HWND sys_win = GetActiveWindow();
////
////        cfg.OGL.Window = sys_win;
////        cfg.OGL.DC = NULL;
////        cfg.OGL.Window = sys_win;
////        ovrHmd_AttachToWindow(mHmd, sys_win, NULL, NULL);
////        console() << "could not initialize" << endl;
////        console() << "running in \"direct-hmd\" moden" << endl;
//    }
//
//    // enable low-persistence display and dynamic prediction for lattency compensation
//    ovrHmd_SetEnabledCaps(mHmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
//
//    ovrEyeRenderDesc eyeRenderDescOut[2];
//    unsigned int dcaps = ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp |
//    ovrDistortionCap_Overdrive;
//
//    if (!ovrHmd_ConfigureRendering(mHmd, &cfg.Config, dcaps, mHmd->DefaultEyeFov, eyeRenderDescOut)) {
//        console() << "failed to configure distortion renderer" << endl;
//    }
//
//}
//
//void OculusTestApp::createFbo(int width, int height)
//{
//    gl::Fbo::Format fmt;
//    fmt.setMagFilter(GL_LINEAR);
//    fmt.setMinFilter(GL_LINEAR);
//    fmt.enableColorBuffer();
//    fmt.enableDepthBuffer();
//    fmt.setSamples(16);
//    fmt.setCoverageSamples(16);
//
//    mFbo = gl::Fbo(width, height, fmt);
//}
//
//void OculusTestApp::mouseDown( MouseEvent event )
//{
//}
//
//void OculusTestApp::keyDown(KeyEvent event)
//{
//    switch (event.getCode())
//    {
//        case KeyEvent::KEY_ESCAPE:
//            if (mHmd) {
//                ovrHmd_Destroy(mHmd);
//            }
//            ovr_Shutdown();
//            quit();
//            break;
//        default:
//            ovrHSWDisplayState state;
//            ovrHmd_GetHSWDisplayState(mHmd, &state);
//            if (state.Displayed)
//            {
//                ovrHmd_DismissHSWDisplay(mHmd);
//            }
//            break;
//    }
//}
//
//void OculusTestApp::update()
//{
//    // Extract Oculus Orientation and Update Camera
//    ovrTrackingState ts = ovrHmd_GetTrackingState(mHmd, ovr_GetTimeInSeconds());
//    ovrPosef pose = ts.HeadPose.ThePose;
//
//    ovrQuatf orientation = pose.Orientation;
//    Quatf ci_quat(orientation.w, orientation.x, orientation.y, orientation.z);
//    mCamera.setOrientation(ci_quat * Quatf(Vec3f(0, 1, 0), M_PI));
//}
//
//void OculusTestApp::draw()
//{
//    Vec2i size = mFbo.getSize();
//    ovrHmd_BeginFrame(mHmd, 0);
//    mFbo.bindFramebuffer();
//
//    // clear out the window with black
//    gl::clear(Color(0, 0.4, 0));
//
//    // store current viewport
//    glPushAttrib(GL_VIEWPORT_BIT);
//
//    for (int i = 0; i < 2; i++)
//    {
//        ovrEyeType eye = mHmd->EyeRenderOrder[i];
//        if (eye == ovrEye_Left)
//        {
//            // draw to left half of window only
//            gl::setViewport(Area(0, 0, size.x / 2, size.y));
//
//            // render left camera
//            mCamera.enableStereoLeft();
//        }
//        else
//        {
//            // draw to right half of window only
//            gl::setViewport(Area(size.x / 2, 0, size.x, size.y));
//
//            // render right camera
//            mCamera.enableStereoRight();
//        }
//        mEyePose[i] = ovrHmd_GetEyePose(mHmd, eye);
//
//        render();
//    }
//
//    // restore viewport
//    glPopAttrib();
//    mFbo.unbindFramebuffer();
//
//    ovrHmd_EndFrame(mHmd, mEyePose, &mEyeTex[0].Texture);
//
//    gl::setMatricesWindow(getWindowSize());
//    gl::draw(mFbo.getTexture(0), Rectf(0, 0, getWindowSize().x, getWindowSize().y));
//}
//
//void OculusTestApp::render()
//{
//    // enable 3D rendering
//    gl::enableDepthRead();
//    gl::enableDepthWrite();
//
//    // set 3D camera matrices
//    gl::pushMatrices();
//    gl::setMatrices(mCamera);
//
//    // draw grid
//    gl::color(Color(0.8f, 0.8f, 0.8f));
//    for (int i = -100; i <= 100; ++i) {
//        gl::drawLine(Vec3f((float)i, 0, -100), Vec3f((float)i, 0, 100));
//        gl::drawLine(Vec3f(-100, 0, (float)i), Vec3f(100, 0, (float)i));
//    }
//
//    // draw floor
//    gl::enableAlphaBlending();
//    gl::color(ColorA(1, 1, 1, 0.75f));
//    gl::drawCube(Vec3f(0.0f, -0.5f, 0.0f), Vec3f(200.0f, 1.0f, 200.0f));
//
//    gl::color(ColorA(1, 0.5, 0, 0.75f));
//    gl::drawTorus(2.0f, 1.0f);
//    gl::disableAlphaBlending();
//    
//    
//    // restore 2D rendering
//    gl::popMatrices();
//    gl::disableDepthWrite();
//    gl::disableDepthRead();
//}
//
//CINDER_APP_NATIVE( OculusTestApp, RendererGl )