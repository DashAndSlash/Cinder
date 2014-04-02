#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"

#include "../src/AntTweakBar/AntTweakBar.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PBOVBOdemoApp : public AppNative {
  public:
    void prepareSettings( Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void resize();
    
	void draw();
    void calcShaderNoise();
    
    void shutdown();
    void loadShaders();
    void readFBOtoVBO();
    void keyUp( KeyEvent event );
    
    static const int   PBO_COUNT = 2;
    static const int   FBO_W = 1024;
    static const int   FBO_H = 1024;
    static const int   DATA_SIZE = FBO_W*FBO_H*3;

    gl::Fbo     mFBO;
    gl::Vbo     mVBO[2];
    gl::GlslProgRef shader;
    gl::GlslProgRef pointShader;
    int         vCount;

    MayaCamUI   mMayaCam;
    int         _index;
    int         _nextIndex;
    
    bool        mRenderTexture;
    gl::TextureRef mTexture;
    
    params::InterfaceGlRef  controls;
    float       timeMult;
};

void PBOVBOdemoApp::prepareSettings( Settings *settings){

    settings->enableHighDensityDisplay();
    gl::disableVerticalSync();
    
}

void PBOVBOdemoApp::setup()
{

    timeMult=1.0;
    
    console() << glGetString(GL_VERSION) << endl;
    _index = 0;
    _nextIndex = 0;
    
    mRenderTexture = false;
    
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGB_FLOAT32_APPLE);

    mFBO = gl::Fbo( FBO_W, FBO_H, format );
    
    GLuint pboVboId[PBO_COUNT];
    for(int i=0; i <PBO_COUNT; i++){
        mVBO[i] = gl::Vbo(GL_ARRAY_BUFFER);
        
        //vertices2 =  new GLfloat[DATA_SIZE]();// = new GLfloat[vCount*3];
        
        mVBO[i].bind();
        mVBO[i].bufferData(DATA_SIZE*sizeof(GLfloat), NULL, GL_STREAM_COPY);
        mVBO[i].unbind();
        pboVboId[i] = mVBO[i].getId();
    }
    
    CameraPersp cam = mMayaCam.getCamera();
    
    cam.setCenterOfInterestPoint(Vec3f::zero());
    
    mMayaCam.setCurrentCam(cam);
    
    // = mVBO[0].getId();
    
//    glGenBuffers(PBO_COUNT, pboVboId);
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboVboId[0]);
//    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE*sizeof(GLfloat), NULL, GL_STREAM_READ);
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboVboId[1]);
//    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE*sizeof(GLfloat), NULL, GL_STREAM_READ);
////    
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    
//    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
//    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
//    glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
    
    loadShaders();
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    //glEnable( GL_MULTISAMPLE_ARB );
    
    controls = params::InterfaceGl::create("Controls", toPixels(Vec2i(200,100)));
    controls->addParam("Time multiplier", &timeMult, "min=0.0 max=5.0 step=0.001");
    TwDefine( "Controls position='10 50' ");
    
}

void PBOVBOdemoApp::loadShaders(){
    
    try {
        pointShader = gl::GlslProg::create( loadAsset("passThru_vert.glsl"), loadAsset("shader03_frag.glsl"));
		shader = gl::GlslProg::create( loadAsset("passThru_vert.glsl"), loadAsset( "shader01_frag.glsl" ) );
	
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
    mTexture = gl::Texture::create( loadImage(  getAssetPath( fs::path( "particle.png" ) ).string() ));

}

void PBOVBOdemoApp::update()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mFBO.bindFramebuffer();
    readFBOtoVBO();
    mFBO.unbindFramebuffer();
    
    calcShaderNoise();
}

void PBOVBOdemoApp::calcShaderNoise()
{

    mFBO.bindFramebuffer();
    shader->bind();
    
    shader->uniform("time", (float)getElapsedSeconds()*timeMult);
    Vec2f res = Vec2f((mFBO.getWidth()),(mFBO.getHeight()));
    shader->uniform("resolution", res);
    Vec2f mousenorm = Vec2f(float(getMousePos().x)/res.x, float(getMousePos().y)/res.y);
    shader->uniform("mouse", mousenorm);
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatricesWindow((mFBO.getSize()));
    gl::setViewport((mFBO.getBounds()));
    
    gl::drawSolidRect( (mFBO.getBounds()) );
    
    shader->unbind();
    
    mFBO.unbindFramebuffer();
    
}

void PBOVBOdemoApp::draw()
{

	// clear out the window with black
	gl::clear( ColorA( 0, 0, 0, 0.0 ) );
    
    gl::color(1.0, 1.0, 1.0, 1.0);
    
    gl::setMatrices(mMayaCam.getCamera());
    gl::setViewport(toPixels(getWindowBounds()));
    gl::enableAlphaBlending();
    
//    glEnable(GL_POINT_SPRITE);
//    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
//    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);		// Enable Vertex Points
//
//    mTexture->bind(0);

//    
//    pointShader->bind();
//    pointShader->uniform("tex0", 0);
    
    gl::enableAlphaBlending();
//    gl::color(ColorA(1.0,1.0,1.0,1.0));
    
    mVBO[_nextIndex].bind();
    
    glEnableClientState(GL_VERTEX_ARRAY);
//    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, 0);
//    glColorPointer(3, GL_FLOAT, 0, 0);

    glDrawArrays(GL_POINTS, 0, mFBO.getWidth()*mFBO.getHeight());

    glDisableClientState(GL_VERTEX_ARRAY);
//    glDisableClientState(GL_COLOR_ARRAY);

    mVBO[_index].unbind();

//    pointShader->unbind();

//    mTexture->unbind();
//    glDisable(GL_POINT_SPRITE);
    
    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    if (mRenderTexture)
    {
        gl::draw(mFBO.getTexture());
    }
    
    gl::drawString( toString(getAverageFps()) , Vec2f(10.0,15.0));
    gl::drawString("Particles: " + toString(FBO_H*FBO_W), Vec2f(10.0,30.0));
    
    controls->draw();
}

void PBOVBOdemoApp::readFBOtoVBO(){
    
    _index = (_index + 1) % 2;
    _nextIndex = (_index + 1) % 2;
//    _index=0;
    //glReadBuffer(GL_FRONT);
    //glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mVBO[_nextIndex].getId());

    glReadPixels(0, 0, FBO_W, FBO_H, GL_RGB, GL_FLOAT, NULL);
   
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, mVBO[_nextIndex].getId());
//
//    void *src = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//
//    if(src)
//    {
//
//
//        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void PBOVBOdemoApp::keyUp( KeyEvent event ) {

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
        case KeyEvent::KEY_t:
            mRenderTexture=!mRenderTexture;
            break;
        default:
            break;
    }
    
}

void PBOVBOdemoApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}
void PBOVBOdemoApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void PBOVBOdemoApp::resize( )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio(getWindowAspectRatio());
    mMayaCam.setCurrentCam(cam);
}

void PBOVBOdemoApp::shutdown(){
    console() << "GOODBYE!" << endl;

    for(int i=0;i<PBO_COUNT;i++){
        GLuint id=mVBO[i].getId();
        glDeleteBuffers(1, &id);
    }
}

CINDER_APP_NATIVE( PBOVBOdemoApp, RendererGl(RendererGl::AA_NONE) )
