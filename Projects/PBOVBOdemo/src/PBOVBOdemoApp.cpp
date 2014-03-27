#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/gl/GlslProg.h"

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
    CameraPersp _cam;
    
    int         _index;
    int         _nextIndex;

    Settings *mSettings;
    
    bool    mRenderTexture;
    gl::TextureRef mTexture;
};

void PBOVBOdemoApp::prepareSettings( Settings *settings){
    
    mSettings = settings;

    settings->enableHighDensityDisplay();
    //settings->setWindowSize(1440, 900);
    //settings->setFullScreen();
    
    //settings->setAlwaysOnTop();
}


void PBOVBOdemoApp::setup()
{
    console() << glGetString(GL_VERSION) << endl;
    _index = 0;
    _nextIndex = 0;
    
    mRenderTexture = false;
    
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGB32F_ARB);
    format.setCoverageSamples(16);
    format.setSamples(16);
    format.enableDepthBuffer();
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
    // = mVBO[0].getId();
    
//    glGenBuffers(PBO_COUNT, pboVboId);
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboVboId[0]);
//    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE*sizeof(GLfloat), NULL, GL_STREAM_READ);
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboVboId[1]);
//    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE*sizeof(GLfloat), NULL, GL_STREAM_READ);
////    
//    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    
    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
    loadShaders();
    
    //gl::disableVerticalSync();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    //glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
}

void PBOVBOdemoApp::loadShaders(){
    
    const char* fragFile = "shader01_frag.glsl";
    
    
    string stringPathFrag = getAssetPath(fs::path( fragFile )).string();
    
    
    const char* vertFile = "passThru_vert.glsl";
    
    string stringPathVert = getAssetPath(fs::path( vertFile )).string();
    
    const char* fragFile2 = "pointSprite.frag";
    string stringPathFrag2 = getAssetPath(fs::path( fragFile2 )).string();
    
    try {
        pointShader = gl::GlslProg::create(loadFile( stringPathVert ), loadFile(stringPathFrag2));
		shader = gl::GlslProg::create( loadFile( stringPathVert ), loadFile( stringPathFrag ) );
	
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
    mFBO.bindFramebuffer();
    readFBOtoVBO();
    mFBO.unbindFramebuffer();
}

void PBOVBOdemoApp::draw()
{

    mFBO.bindFramebuffer();
    shader->bind();

    shader->uniform("time", (float)getElapsedSeconds());
    Vec2f res = Vec2f((mFBO.getWidth()),(mFBO.getHeight()));
    shader->uniform("resolution", res);
    Vec2f mousenorm = Vec2f(float(getMousePos().x)/res.x, float(getMousePos().y)/res.y);
    shader->uniform("mouse", mousenorm);
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatricesWindow((mFBO.getSize()));
    gl::setViewport((mFBO.getBounds()));
    
    gl::drawSolidRect( (mFBO.getBounds()));//Rectf(0.0,0.0,mFBO.getWidth(),mFBO.getHeight()));
    

    shader->unbind();
    
    mFBO.unbindFramebuffer();
    
    
	// clear out the window with black
	gl::clear( ColorA( 0, 0, 0, 0.0 ) );
    
    gl::color(1.0, 1.0, 1.0,1.0);
    


    gl::setMatrices(mMayaCam.getCamera());
    gl::setViewport(toPixels(getWindowBounds()));
    gl::enableAlphaBlending();
    
    
    
//    glEnable(GL_POINT_SPRITE);
//	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
//	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);		// Enable Vertex Points
//    
//    mTexture->bind(0);
    
    
//    pointShader->bind();
//    pointShader->uniform("tex", 0);
    gl::enableAlphaBlending();
    gl::color(ColorA(0.90,1.0,1.0,0.4));
    mVBO[_nextIndex].bind();
    


    glEnableClientState(GL_VERTEX_ARRAY);
//    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, 0);
//    glColorPointer(3, GL_FLOAT, 0, 0);

    glDrawArrays(GL_POINTS, 0, mFBO.getWidth()*mFBO.getHeight());

    glDisableClientState(GL_VERTEX_ARRAY);
//    glDisableClientState(GL_COLOR_ARRAY);


    
    mVBO[_nextIndex].unbind();

//    pointShader->unbind();

//    mTexture->unbind();
//    glDisable(GL_POINT_SPRITE);
    
    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    gl::disableDepthRead();
    gl::disableDepthWrite();

    
    if (mRenderTexture)
    {
//        gl::setMatricesWindow(toPixels(getWindowSize()));
//        gl::setViewport(toPixels(getWindowBounds()));
        gl::draw(mFBO.getTexture());
    }
    gl::drawString( toString(getAverageFps()) , Vec2f(10.0,15.0));
    gl::drawString("Particles: " + toString(FBO_H*FBO_W), Vec2f(10.0,30.0));
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::disableAlphaBlending();

}

void PBOVBOdemoApp::readFBOtoVBO(){
    
    _index = (_index + 1) % 2;
    _nextIndex = (_index + 1) % 2;
//    _index=0;
    //glReadBuffer(GL_FRONT);
    //glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mVBO[_index].getId());

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
    _cam = mMayaCam.getCamera();

    _cam.setAspectRatio(getWindowAspectRatio());
    mMayaCam.setCurrentCam(_cam);
}

void PBOVBOdemoApp::shutdown(){
    console() << "GOODBYE!" << endl;

    for(int i=0;i<PBO_COUNT;i++){
        GLuint id=mVBO[i].getId();
        glDeleteBuffers(1, &id);
        
    }
}

CINDER_APP_NATIVE( PBOVBOdemoApp, RendererGl(16) )
