# Ribbons

A little music visualizer with Oculus Rift support.

Turn VR support on and off by changing `VR=1` to `VR=0` or vice versa in Build Settings > "Preprocessor Macros Not Used In Precompiled Headers".

## To do

- [X] ribbons radially outward
- [ ] smooth out amplitude
- [X] accelerate ribbons proportionally on beat?
- [ ] lines with l/r spectrum along sides or volume history
- [ ] album art: id3lib.sourceforge.net
- [ ] use speaker output

### OVR

- [ ] show stars all around
- [ ] ovrHmd_GetFovTextureSize is too big: scale down for rendering (use display size?)
- [ ] ovr shutdown throws exception: ovrHmd_Destroy is called between ovrHmd_BeginFrame and ovrHmd_EndFrame
