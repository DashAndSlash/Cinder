#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"



using namespace ci;
using namespace ci::app;
using namespace std;

class GpuFlowApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

    gl::GlslProgRef mFlowSh;
    
    CaptureRef mCamCapture;
    
    gl::TextureRef mTexture[2];
    
    int frameNum;
    
    params::InterfaceGlRef mParam;
    Vec2f scale;
    Vec2f offset;
    float lambda;


};

void GpuFlowApp::setup()
{
    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
    
    string spathVShader = getAssetPath(fs::path( "flow.vs" )).string();
    string spathFShader = getAssetPath(fs::path( "flow.fs" )).string();
    
    try {
        
		mFlowSh = gl::GlslProg::create( loadFile( spathVShader ), loadFile( spathFShader ) );
        
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}

    mCamCapture = Capture::create(640, 480);
    mCamCapture->start();
    
    mTexture[0] = gl::Texture::create(640, 480);
    mTexture[1] = gl::Texture::create(640, 480);

    frameNum = 0;
    scale= Vec2f::one();
    offset=Vec2f::zero();
    lambda=0.3f;
    mParam = params::InterfaceGl::create("Param Flow", Vec2i(200,200));
    mParam->addParam("Scale X", &scale.x, "min=0.0 max=10.0 step=0.01");
    mParam->addParam("Scale Y", &scale.y, "min=0.0 max=10.0 step=0.01");
    mParam->addParam("offset X", &offset.x, "min=0.0 max=10.0 step=0.001");
    mParam->addParam("offset Y", &offset.y, "min=0.0 max=10.0 step=0.001");
    mParam->addParam("Lambda", &lambda);

    
    
}

void GpuFlowApp::mouseDown( MouseEvent event )
{
}

void GpuFlowApp::update()
{
    //
    if(mCamCapture->checkNewFrame())
    {
        frameNum++;
        Surface srfc = mCamCapture->getSurface();
        Channel8u chn(srfc);
        mTexture[frameNum%2]->update(chn, Area(chn.getBounds()));
    }

}

void GpuFlowApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    gl::color(1, 1, 1);
    mFlowSh->bind();
//    mTexture->enableAndBind();
    mTexture[frameNum%2]->bind(1);
    mTexture[(frameNum+1)%2]->bind(0);
    mFlowSh->uniform("tex0", 0);
    mFlowSh->uniform("tex1", 1);
    mFlowSh->uniform("scale", scale);
    mFlowSh->uniform("offset", offset);
    mFlowSh->uniform("lambda", lambda);
    
    
    gl::drawSolidRect( (mTexture[0]->getBounds()));
    
    mFlowSh->unbind();
    mTexture[0]->unbind();
    mTexture[1]->unbind();
    mParam->draw();
    gl::drawString("Fps: " + toString(getAverageFps()), Vec2f(15.0,15.0));
}

CINDER_APP_NATIVE( GpuFlowApp, RendererGl )
