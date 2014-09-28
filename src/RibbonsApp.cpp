#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RibbonsApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void RibbonsApp::setup()
{
}

void RibbonsApp::mouseDown( MouseEvent event )
{
}

void RibbonsApp::update()
{
}

void RibbonsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( RibbonsApp, RendererGl )
