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




class FBOVBOdemoApp : public AppNative {
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

    void keyUp( KeyEvent event );
    
    static const int   FBO_W = 1024;
    static const int   FBO_H = 1024;
    static const int   DATA_SIZE = FBO_W*FBO_H*3;
    
    gl::Fbo     mFBO;
    
    gl::VboMeshRef	mVboMesh;
    
    gl::GlslProgRef shader;
    
    gl::GlslProgRef displaceShader;
    
    
    MayaCamUI   mMayaCam;
    
    
    bool        mRenderTexture;
    gl::TextureRef mTexture;
    
    params::InterfaceGlRef  controls;
    float       timeMult;
};

void FBOVBOdemoApp::prepareSettings( Settings *settings){
    
    settings->enableHighDensityDisplay();
    gl::disableVerticalSync();
    
}


void FBOVBOdemoApp::setup()
{
    timeMult=0.5;
    
    console() << glGetString(GL_VERSION) << endl;
    
    mRenderTexture = false;
    
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGB_FLOAT32_APPLE);
    
    mFBO = gl::Fbo( FBO_W, FBO_H, format );

    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();

    
    mVboMesh = gl::VboMesh::create( FBO_W*FBO_H, FBO_W*FBO_H, layout, GL_POINTS );

    
    vector<Vec3f> points;

    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    
    for(int i=0; i <FBO_W; i++){
        for(int j=0; j < FBO_H; j++){
//            points.push_back(Vec3f(i,i,i));
            indices.push_back(i * FBO_W + j);
            texCoords.push_back( Vec2f( i/(float)FBO_W, j/(float)FBO_H ) );
        }
    }

//    mVboMesh->bufferPositions(points);
    mVboMesh->bufferIndices(indices);
    mVboMesh->bufferTexCoords2d( 0, texCoords );
    mVboMesh->unbindBuffers();

    //SETUP CAMERA
    CameraPersp cam = mMayaCam.getCamera();
    cam.setCenterOfInterestPoint(Vec3f::zero());
    mMayaCam.setCurrentCam(cam);
    
    //SHADERS
    loadShaders();
    
    //MAKING LOOKS BETTER
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    //UI
    controls = params::InterfaceGl::create("Controls", toPixels(Vec2i(200,100)));
    controls->addParam("Time multiplier", &timeMult, "min=0.0 max=5.0 step=0.001");
    TwDefine( "Controls position='10 50' ");
        
}

void FBOVBOdemoApp::loadShaders(){
    
    try {
        
        displaceShader = gl::GlslProg::create( loadAsset("displaceVBO_vert.glsl"), loadAsset("passThru_frag.glsl"));
        
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

void FBOVBOdemoApp::update()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();

    calcShaderNoise();
}

void FBOVBOdemoApp::calcShaderNoise()
{
    
    mFBO.bindFramebuffer();
    shader->bind();
    

    shader->uniform("time", (float)getElapsedSeconds()*timeMult);
    Vec2f res = Vec2f((FBO_W),(FBO_H));
    shader->uniform("resolution", res);
    Vec2f mousenorm = Vec2f(float(getMousePos().x)/res.x, float(getMousePos().y)/res.y);
    shader->uniform("mouse", mousenorm);
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatricesWindow((Vec2i(FBO_W, FBO_H)));
    gl::setViewport( (mFBO.getBounds()));
    
    gl::drawSolidRect( (mFBO.getBounds()) );
    
    shader->unbind();
    
    mFBO.unbindFramebuffer();
}

void FBOVBOdemoApp::draw()
{
	// clear out the window with black
	gl::clear( ColorA( 0, 0, 0, 0.0 ) );
    
    gl::color(1.0, 1.0, 1.0, 1.0);
    
    gl::setMatrices(mMayaCam.getCamera());
    gl::setViewport(toPixels(getWindowBounds()));
    gl::enableAlphaBlending();
    
    mFBO.bindTexture();
    
    displaceShader->bind();
    displaceShader->uniform("tex", 0);

    gl::draw( mVboMesh );
    
    displaceShader->unbind();
    
    mFBO.unbindTexture();
    
    gl::drawCoordinateFrame(1.0f*0.05, 0.2f*0.05, 0.05f*0.05);

//    glDisable(GL_POINT_SPRITE);
    
    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    if (mRenderTexture)
    {
        gl::color(1.0, 1.0, 1.0, 1.0);
        gl::draw(mFBO.getTexture());
    }
    
    gl::drawString( toString(getAverageFps()) , Vec2f(10.0,15.0));
    gl::drawString("Particles: " + toString(FBO_H*FBO_W), Vec2f(10.0,30.0));
//
    controls->draw();
}


void FBOVBOdemoApp::keyUp( KeyEvent event ) {
    
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

void FBOVBOdemoApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}
void FBOVBOdemoApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void FBOVBOdemoApp::resize( )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio(getWindowAspectRatio());
    mMayaCam.setCurrentCam(cam);
}

void FBOVBOdemoApp::shutdown(){
    console() << "GOODBYE!" << endl;
}

CINDER_APP_NATIVE( FBOVBOdemoApp, RendererGl(RendererGl::AA_MSAA_8) )
