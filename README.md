# Ribbons

A little music visualizer.

## To do

- [X] ribbons radially outward
- [ ] smooth out amplitude
- [X] accelerate ribbons proportionally on beat?
- [ ] lines with l/r spectrum along sides or volume history
- [ ] album art: id3lib.sourceforge.net

## OVR Integration

Not supported with current cinder (not glnext) and ovr 0.4. See https://forum.libcinder.org/topic/oculus-rift-dk2-block

- gl::clear working, but geometry does not draw

Possible Reasons

- implement stereocamera?

use:
setWindowSize(mHmd->Resolution.w, mHmd->Resolution.h); //this
setWindowPos(mHmd->WindowsPos.x, mHmd->WindowsPos.y);
?

- X image scaled incorrectly
  - X sphere with 90 rad fills screen, does not change
  - X maybe diff fov? sphere larger radius than camera, disable cull
- X not passed to/processed by ovr
  - X warp applied
  - X image color changes with clear
- mesh not drawing to texture
  - existing improper use of cinder exposed by ovr
  - mesh not correctly passed to ovr... but clear is?
  - X mesh is not drawn to texture
- texture not drawing correctly onto screen
  - how is ovr drawing it onto canvas (finding canvas?) since I'm not drawing onto it?
- incorrectly configurerendering ing?
  - disabling this and draw code makes it draw fine

X CAPI_GL_DistortionRenderer.cpp:542 glDisables
X disable swap buffers
X show only one frame, things reset after?


glBindFramebuffer(GL_FRAMEBUFFER, 0);
setViewport( Recti(0,0, RParams.RTSize.w, RParams.RTSize.h) );

if (DistortionCaps & ovrDistortionCap_SRGB)
glEnable(GL_FRAMEBUFFER_SRGB);
else
glDisable(GL_FRAMEBUFFER_SRGB);

glDisable(GL_CULL_FACE);
glDisable(GL_DEPTH_TEST);

if (glState->GLVersionInfo.SupportsDrawBuffers)
{
glDisablei(GL_BLEND, 0);
glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}
else
{
glDisable(GL_BLEND);
glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}

glDisable(GL_DITHER);
glDisable(GL_RASTERIZER_DISCARD);
if ((glState->GLVersionInfo.MajorVersion >= 3 && glState->GLVersionInfo.MinorVersion >= 2) || glState->GLVersionInfo.MajorVersion >= 4)
{
glDisable(GL_SAMPLE_MASK);
}

glClearColor(
RState.ClearColor[0],
RState.ClearColor[1],
RState.ClearColor[2],
RState.ClearColor[3] );

glClear(GL_COLOR_BUFFER_BIT);