#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class ParticlesDemo_01App : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	

	void mouseDrag( MouseEvent event );
	void update();
	void resize();
	void draw();
    void keyUp( KeyEvent event );
    void loadShaders();
    

    void writeToAttractors();
    
    gl::GlslProgRef mEmit;
    gl::GlslProgRef mVelocity;
    gl::GlslProgRef mPassthru;
    gl::GlslProgRef mUpdate;
    gl::GlslProgRef mInit;
    gl::GlslProgRef mDisplay;
    
    gl::VboMeshRef  mVboMesh;
    
    static const int FBO_W = 1024;
    static const int FBO_H = 1024;
    
    MayaCamUI   mMayaCam;
    
    gl::Fbo     mDataBuffer[2];
    gl::Fbo     mEmitData;
    gl::Fbo     mForceData;
    
    u_int8_t    mIndex;
    u_int8_t    mNextIndex;
    
    float       mLastElapsedSeconds;
    
    gl::TextureRef mImg;
    
};

void ParticlesDemo_01App::loadShaders()
{
    try {
        mEmit = gl::GlslProg::create(loadAsset("passThru_vp.glsl"), loadAsset("emit_sphere_multi_fp.glsl"));
        mDisplay = gl::GlslProg::create(loadAsset("display_vp.glsl"), loadAsset("display_fp.glsl"));
        mVelocity = gl::GlslProg::create(loadAsset("passThru_vp.glsl"), loadAsset("force_attractors_fp.glsl"));
        
        mPassthru = gl::GlslProg::create(loadAsset("passThru_vp.glsl"), loadAsset("thru_fp.glsl"));
        
        mUpdate = gl::GlslProg::create(loadAsset("passThru_vp.glsl"), loadAsset("update_fp.glsl"));
        
        mInit = gl::GlslProg::create(loadAsset("passThru_vp.glsl"), loadAsset("init_fp.glsl"));
        
    } catch (...) {
        console() << "Error loading shaders" << endl;
    }
}

void ParticlesDemo_01App::setup()
{
    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
    
    mImg = gl::Texture::create( loadImage(loadAsset("particle.png")));
    loadShaders();
    
    mLastElapsedSeconds = 0.0f;
    mIndex = 0;
    mNextIndex = 1;
    
    //SETUP VBO MESH
    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    
    mVboMesh = gl::VboMesh::create( FBO_W*FBO_H, FBO_W*FBO_H, layout, GL_POINTS );
    
    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    vector<Vec3f> points;
    
    for(int i=0; i <FBO_W; i++){
        for(int j=0; j < FBO_H; j++){
            points.push_back(Vec3f((float)i/(float)FBO_W, (float)j/(float)FBO_H,0));
            indices.push_back(i * FBO_W + j);
            texCoords.push_back( Vec2f( i/(float)FBO_W, j/(float)FBO_H ) );
        }
    }
    
        mVboMesh->bufferPositions(points);
    mVboMesh->bufferIndices(indices);
    mVboMesh->bufferTexCoords2d( 0, texCoords );
    mVboMesh->unbindBuffers();
    
    //SETUP CAMERA
    CameraPersp cam = mMayaCam.getCamera();
    cam.setCenterOfInterestPoint(Vec3f::zero());
//    cam.setEyePoint(Vec3f::one());
    mMayaCam.setCurrentCam(cam);
    
    
    //SETUP FBO "BUFFERS"
    
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGBA_FLOAT32_APPLE);
    format.enableColorBuffer(true,4);
    format.enableDepthBuffer(false,false);
    mDataBuffer[0] = gl::Fbo( FBO_W, FBO_H, format );
    mDataBuffer[1] = gl::Fbo( FBO_W, FBO_H, format );
    
    mDataBuffer[0].bindFramebuffer();
    gl::clear(ColorA(0.0,0.0,0.0,1.0));
    mDataBuffer[0].unbindFramebuffer();
    mDataBuffer[1].bindFramebuffer();
    gl::clear(ColorA(0.0,0.0,0.0,1.0));
    mDataBuffer[1].unbindFramebuffer();
    
    gl::Fbo::Format format2;
    format2.setColorInternalFormat(GL_RGBA_FLOAT32_APPLE);
    format2.enableColorBuffer(true,1);
    format2.enableDepthBuffer(false,false);
    
    mEmitData = gl::Fbo( 10, 2, format2 );
    
    mForceData = gl::Fbo( 1, 2, format2 );
    
    
    //INIT PARTICLES BUFFERS
    
    mEmitData.bindFramebuffer();
    mInit->bind();
    
    gl::clear( ColorA( 0, 0, 0, 1 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mEmitData.getBounds() );
    gl::setMatricesWindow(mEmitData.getSize());
    float I = mEmitData.getWidth();
    gl::begin(GL_POINTS);
    {
		for (int i=0; i<I; i++) {
            gl::texCoord ( randFloat(-400.0f, 400), randFloat(-400.0, 400), randFloat(-400, 400));
            gl::vertex (i+0.5,0.5f, 0.0f);
			gl::texCoord (50.0f,(float)i/(float)I,(float)(i+1.0f)/(float)I);
			gl::vertex (i+0.5,1.5f, 0.0f);
		}
        
    }
    gl::end();
    
    mInit->unbind();
    mEmitData.unbindFramebuffer();
    
    mForceData.bindFramebuffer();
    mInit->bind();
    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mForceData.getBounds() );
    gl::setMatricesWindow(mForceData.getSize());
    gl::begin(GL_POINTS);
    {
        for (int i=0; i<mForceData.getWidth(); i++) {
//            //g.textureCoords (CCMath.random(-600,600), CCMath.random(-200,200), CCMath.random(-200,-400));
            gl::texCoord (0,100.0,0);
            gl::vertex(i,0.0f);
            gl::texCoord (100.0f,0.0f,0.0f);
            gl::vertex(i,.5f);
        }
    }
    gl::end();
    mInit->unbind();
    mForceData.unbindFramebuffer();
}

void ParticlesDemo_01App::writeToAttractors () {
	
    
    mInit->bind();
    mForceData.bindFramebuffer();
    gl::begin(GL_POINTS);
    {
        gl::texCoord( getMousePos() );
        gl::vertex (0.5f,0.5f);
    }
    gl::end();
    mForceData.unbindFramebuffer();
    mInit->unbind();
}

void ParticlesDemo_01App::update()
{
//    writeToAttractors();
    mEmit->bind();
    mDataBuffer[mNextIndex].bindFramebuffer();
    
    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mDataBuffer[mNextIndex].getBounds() );
    gl::setMatricesWindow(mDataBuffer[mNextIndex].getSize());
    
    mDataBuffer[mIndex].bindTexture(0,0);
    mDataBuffer[mIndex].bindTexture(1,1);
    mDataBuffer[mIndex].bindTexture(2,2);
    mEmitData.bindTexture(3,0);
    
    mEmit->uniform("positions", 0);
    mEmit->uniform("velocities", 1);
    mEmit->uniform("infos", 2);
    mEmit->uniform("clusterCenters", 3);
    
    Vec3f rand = Vec3f( randFloat(100.0f), randFloat(100.0f), randFloat(3000.0f,10000.0f) );
    
    mEmit->uniform("randomSeed", rand );
    mEmit->uniform("dataW", mEmitData.getWidth());
    mEmit->uniform("dataH", mEmitData.getHeight());
    
    gl::drawSolidRect(mDataBuffer[mNextIndex].getBounds());

    mDataBuffer[mNextIndex].unbindFramebuffer();
    mEmit->unbind();
    mEmitData.unbindTexture();
    
    mIndex = (mIndex+1)%2;
    mNextIndex = (mIndex+1)%2;

    
    mDataBuffer[mNextIndex].bindFramebuffer();
    mPassthru->bind();
    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mDataBuffer[mNextIndex].getBounds() );
    gl::setMatricesWindow(mDataBuffer[mNextIndex].getSize());
    mDataBuffer[mIndex].bindTexture(0,0);
    mDataBuffer[mIndex].bindTexture(1,1);
    mDataBuffer[mIndex].bindTexture(2,2);
    mPassthru->uniform("positions", 0);
    mPassthru->uniform("velocities", 1);
    mPassthru->uniform("infos", 2);
    
    gl::drawSolidRect(mDataBuffer[mNextIndex].getBounds());
    mDataBuffer[mNextIndex].unbindFramebuffer();
    mPassthru->unbind();
    
    mIndex = (mIndex+1)%2;
    mNextIndex = (mIndex+1)%2;
    
    
    mDataBuffer[mNextIndex].bindFramebuffer();
    mVelocity->bind();
    
    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mDataBuffer[mNextIndex].getBounds() );
    gl::setMatricesWindow(mDataBuffer[mNextIndex].getSize());
    mDataBuffer[mIndex].bindTexture(0,0);
    mDataBuffer[mIndex].bindTexture(1,1);
    mDataBuffer[mIndex].bindTexture(2,2);
    mForceData.bindTexture(3,0);

    mVelocity->uniform("positions", 0);
    mVelocity->uniform("velocities", 1);
    mVelocity->uniform("attractors", 2);
    mVelocity->uniform("infos", 2);
    
    mVelocity->uniform("dataW", mForceData.getWidth());
    mVelocity->uniform("dataH", mForceData.getHeight());
    
    gl::drawSolidRect(mDataBuffer[mNextIndex].getBounds());
    mVelocity->unbind();
    mDataBuffer[mNextIndex].unbindFramebuffer();
    
    mIndex = (mIndex+1)%2;
    mNextIndex = (mIndex+1)%2;

    
    mDataBuffer[mNextIndex].bindFramebuffer();
    mUpdate->bind();
    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( mDataBuffer[mNextIndex].getBounds() );
    gl::setMatricesWindow(mDataBuffer[mNextIndex].getSize());
    mDataBuffer[mIndex].bindTexture(0,0);
    mDataBuffer[mIndex].bindTexture(1,1);
    mDataBuffer[mIndex].bindTexture(2,2);
    
    mUpdate->uniform("positions", 0);
    mUpdate->uniform("velocities", 1);
    mUpdate->uniform("infos", 2);
    float deltaTime = getElapsedSeconds()-mLastElapsedSeconds;
    mUpdate->uniform("deltaTime", deltaTime );
    
    
    gl::drawSolidRect(mDataBuffer[mNextIndex].getBounds());
    mUpdate->unbind();
    mDataBuffer[mNextIndex].unbindFramebuffer();
    mDataBuffer[mIndex].unbindTexture();
    mLastElapsedSeconds=getElapsedSeconds();
    
    mIndex = (mIndex+1)%2;
    mNextIndex = (mIndex+1)%2;
}

void ParticlesDemo_01App::draw()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
//	clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::color(1.0, 1.0, 1.0, 1.0);
    
    gl::setMatrices(mMayaCam.getCamera());
    gl::setViewport(toPixels(getWindowBounds()));
    gl::enableAlphaBlending();
    
    mDisplay->bind();
    
    mDataBuffer[mIndex].bindTexture(0,0);
    mDisplay->uniform("positions", 0);
    gl::draw(mVboMesh);
    
    mDisplay->unbind();

    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    
    gl::disableDepthRead();
    gl::disableDepthWrite();

    gl::draw( mEmitData.getTexture(0), toPixels(Area(0,0,100,100)));
    gl::drawString("FPS: " + toString(getAverageFps()), Vec2f(10.0f,10.0f));
}

void ParticlesDemo_01App::keyUp( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_SPACE:
        loadShaders();
        break;
        case KeyEvent::KEY_ESCAPE:
        quit();
        break;
        case KeyEvent::KEY_f:
        setFullScreen(!isFullScreen());
        break;
        default:
        break;
    }
}

void ParticlesDemo_01App::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}
void ParticlesDemo_01App::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void ParticlesDemo_01App::resize( )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio(getWindowAspectRatio());

    mMayaCam.setCurrentCam(cam);
}

CINDER_APP_NATIVE( ParticlesDemo_01App, RendererGl )
