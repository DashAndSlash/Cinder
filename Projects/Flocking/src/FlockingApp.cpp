#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/Plane.h"
#include "cinder/params/Params.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class FlockingApp : public AppNative {
  public:
    void setup();
	void mouseDown( MouseEvent event );
    
	void mouseDrag( MouseEvent event );
	void update();
	void resize();
	void draw();
    void keyUp( KeyEvent event );
    void loadShaders();
    
    void reset();
    
    void swap();

    gl::Fbo                 _myDataBuffer[2];
    gl::TextureRef          _textureNoise;
    
    gl::GlslProgRef         _myVelocityShader;
    gl::GlslProgRef         _myDisplayShader;
    gl::GlslProgRef         _myThruShader;
    gl::GlslProgRef         _myEmit;
    gl::VboMeshRef          _myMesh;
	
	static const int        particleW = 1024;
	static const int        particleH = 1024;
    
    MayaCamUI               mMayaCam;
    
    int                     _index;
    int                     _prevIndex;
    float                   _prevElapsedTime;
    params::InterfaceGlRef  controls;
    ColorA                  _color;
    Vec3f                   _avoidSphere;
};

void FlockingApp::swap() {
    _index = (_index+1)%2;
    _prevIndex = (_index+1)%2;
}

void FlockingApp::reset()
{
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGB_FLOAT32_APPLE);
    format.enableColorBuffer(true,3);
    format.enableDepthBuffer(false,false);
    
    
    for (int i = 0; i<2; i++) {
        
        _myEmit->bind();
        _textureNoise->bind(0);
        _myEmit->uniform("positions", 0);
        _myDataBuffer[i] = gl::Fbo( particleW, particleH, format );
        _myDataBuffer[i].bindFramebuffer();
        gl::clear(ColorA(0.0,0.0,0.0,1.0));
        gl::setViewport(_myDataBuffer[i].getBounds());
        gl::setMatricesWindow(_myDataBuffer[i].getSize());
        gl::draw(_textureNoise, _myDataBuffer[i].getBounds());
        _myDataBuffer[i].unbindFramebuffer();

        _myEmit->unbind();
    }
    
}

void FlockingApp::setup()
{
    
    _color = ColorA::white();
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    gl::disableVerticalSync();
    _prevElapsedTime = 0;
    _index = 0;
    _prevIndex = 1;
    _textureNoise = gl::Texture::create(loadImage(loadAsset("rgb_noise.jpg")));
    
    //SETUP CAMERA
    CameraPersp cam = mMayaCam.getCamera();
    cam.setCenterOfInterestPoint(Vec3f::zero());
    mMayaCam.setCurrentCam(cam);
    
    //SETUP VBO MESH
    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    
    _myMesh = gl::VboMesh::create( particleW*particleH, particleW*particleH, layout, GL_POINTS );
    
    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    vector<Vec3f> points;
    
    for(int i=0; i <particleW; i++){
        for(int j=0; j < particleH; j++){
            points.push_back(Vec3f((float)i/(float)particleW, (float)j/(float)particleH,0));
            indices.push_back(i * particleW + j);
            texCoords.push_back( Vec2f( i/(float)particleW, j/(float)particleH ) );
        }
    }
    
    _myMesh->bufferPositions(points);
    _myMesh->bufferIndices(indices);
    _myMesh->bufferTexCoords2d( 0, texCoords );
    _myMesh->unbindBuffers();
    
    loadShaders();
    reset();
    
    controls = params::InterfaceGl::create("Controls", toPixels(Vec2i(200,100)));
    controls->addParam("Color", &_color);
}

void FlockingApp::update()
{
    _avoidSphere = Vec3f( sin(getElapsedSeconds())*5.0, cos(getElapsedSeconds())*5.0,0.0);
    gl::disableAlphaBlending();
    _myDataBuffer[_index].bindFramebuffer();
//    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( _myDataBuffer[_index].getBounds() );
    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
    _myVelocityShader->bind();

    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDataBuffer[_prevIndex].bindTexture(1,1);
    _myDataBuffer[_prevIndex].bindTexture(2,2);
    _textureNoise->bind(3);
    _myVelocityShader->uniform("positions", 0);
    _myVelocityShader->uniform("velocities", 1);
    _myVelocityShader->uniform("infos", 2);
    _myVelocityShader->uniform("initPos", 3);
    _myVelocityShader->uniform("time", (float)getElapsedSeconds());
    
    _myVelocityShader->uniform("avoid", _avoidSphere);
    float deltaT = getElapsedSeconds() - _prevElapsedTime;
    _myVelocityShader->uniform("deltaT", deltaT);
    
    
    gl::drawSolidRect(_myDataBuffer[_index].getBounds());
    _myVelocityShader->unbind();
    _myDataBuffer[_prevIndex].unbindTexture();
    _myDataBuffer[_index].unbindFramebuffer();
    
    swap();
    
//    _myDataBuffer[_index].bindFramebuffer();
//    //    gl::clear( Color( 0, 0, 0 ) );
//    gl::color(1.0, 1.0, 1.0, 1.0);
//    gl::setViewport( _myDataBuffer[_index].getBounds() );
//    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
//    _myThruShader->bind();
//    _myDataBuffer[_prevIndex].bindTexture(0,0);
//    _myDataBuffer[_prevIndex].bindTexture(1,1);
//    _myDataBuffer[_prevIndex].bindTexture(2,2);
//    
//
//    _myThruShader->uniform("positions", 0);
//    _myThruShader->uniform("velocities", 1);
//    _myThruShader->uniform("infos", 2);
//    
//    gl::drawSolidRect(_myDataBuffer[_index].getBounds());
//    _myThruShader->unbind();
//    _myDataBuffer[_index].unbindFramebuffer();
//    _myDataBuffer[_prevIndex].unbindTexture();
//    swap();
    _prevElapsedTime = getElapsedSeconds();
}

void FlockingApp::draw()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
    //	clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::color(1.0, 1.0, 1.0, 1.0);
    
    gl::setMatrices(mMayaCam.getCamera());
    gl::setViewport(toPixels(getWindowBounds()));
    gl::enableAlphaBlending();
    
    _myDisplayShader->bind();
    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDisplayShader->uniform("positions", 0);
    _myDisplayShader->uniform("color", _color);
    gl::draw(_myMesh);
    _myDisplayShader->unbind();
    _myDataBuffer[_prevIndex].unbindTexture();
    
    gl::drawSphere(_avoidSphere, 1.0);
    
    gl::drawCoordinateFrame(1.0f*0.05, 0.2f*0.05, 0.05f*0.05);
    
    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::color(1.0, 1.0, 1.0, 1.0);
//    gl::draw( _myDataBuffer[_prevIndex].getTexture(2), toPixels(Area(0,0,100,100)));
    gl::drawString("FPS: " + toString(getAverageFps()), Vec2f(10.0f,10.0f));
    
    controls->draw();
}


void FlockingApp::loadShaders()
{
    try {
        
        _myDisplayShader = gl::GlslProg::create( loadAsset("display_vp.glsl"), loadAsset("display_fp.glsl"));
        _myVelocityShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("force_fp.glsl"));
        _myThruShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("thru_fp.glsl"));
        _myEmit = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("emit_fp.glsl"));
        } catch ( gl::GlslProgCompileExc &exc ) {
        console()<<"Error compiling shader" << endl;
        console() << exc.what();
    }
    
}

void FlockingApp::keyUp( KeyEvent event )
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
        case KeyEvent::KEY_r:
            reset();
            break;
        default:
            break;
    }
}

void FlockingApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}
void FlockingApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void FlockingApp::resize( )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio(getWindowAspectRatio());
    
    mMayaCam.setCurrentCam(cam);
}

CINDER_APP_NATIVE( FlockingApp, RendererGl(16) )
