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

class ParticlesDemoApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
    
	void mouseDrag( MouseEvent event );
	void update();
	void resize();
	void draw();
    void keyUp( KeyEvent event );
    void loadShaders();
    
    void swap();
    void reset();
    
    
    gl::Fbo                 _myDataBuffer[2];
//	gl::Fbo                _myDataBuffer[_prevIndex];
	gl::Fbo                 _myConstraintData;
	
	gl::TextureRef          _myEmitTexture;
	
    gl::GlslProgRef         _myEmitShader;
    gl::GlslProgRef         _myVelocityShader;
    gl::GlslProgRef         _myThruShader;
    gl::GlslProgRef         _myUpdateShader;
    gl::GlslProgRef         _myInitValueShader;
	
	
    gl::GlslProgRef         _myDisplayShader;
	
    gl::VboMeshRef             _myMesh;
	
	static const int        particleW = 1024;
	static const int        particleH = 1024;
    
    MayaCamUI               mMayaCam;
    
    int                     _index;
    int                     _prevIndex;
    
    Planef                  _plane;
    Vec3f                   _planePos;
    Vec3f                   _planeNorm;
    Vec3f                   _constraints[1];
    Vec3f                   _constraintsNormal[1];
    params::InterfaceGlRef  controls;
    
    float                   _prevElapsedTime;
    Vec3f                    _gravity;
    
    float                   _bounce;
};

void ParticlesDemoApp::reset()
{
    
    _bounce = 1.0;
    gl::Fbo::Format format;
    format.setColorInternalFormat(GL_RGB_FLOAT32_APPLE);
    format.enableColorBuffer(true,3);
    format.enableDepthBuffer(false,false);
    
    _myDataBuffer[0] = gl::Fbo( particleW, particleH, format );
    
    _myDataBuffer[0].bindFramebuffer();
    gl::clear(ColorA(0.0,0.0,0.0,1.0));
    _myDataBuffer[0].unbindFramebuffer();
    
    _myDataBuffer[1] = gl::Fbo( particleW, particleH, format );
    _myDataBuffer[1].bindFramebuffer();
    gl::clear(ColorA(0.0,0.0,0.0,1.0));
    _myDataBuffer[1].unbindFramebuffer();
    
    gl::Fbo::Format format2;
    format2.setColorInternalFormat(GL_RGB_FLOAT32_APPLE);
    format2.enableColorBuffer(true,1);
    format2.enableDepthBuffer(false,false);
    _myConstraintData = gl::Fbo( 5,5, format2);
    
    
    _myInitValueShader->bind();
    _myConstraintData.bindFramebuffer();
    
    gl::clear(Color(0.0,0.0,0.0));
    gl::setViewport(_myConstraintData.getBounds());
    gl::setMatricesWindow(_myConstraintData.getSize());
    
    gl::begin(GL_POINTS);
    
    gl::texCoord (0.0f, -150.0f, 0.0f);
    gl::vertex(0.0f,0.0f);
    gl::texCoord (0.5f/2.5f, 1.0f/2.5f, 0.0f);
    gl::vertex(0.0f,0.0f);
    
    gl::texCoord (0.0f, -150.0f, 0.0f);
    gl::vertex(0.0f,0.0f);
    gl::texCoord (-0.5f/2.5f, 1.0f/2.5f, 0.0f);
    gl::vertex(0.0f,0.0f);
    
    gl::end();
    
    _myConstraintData.unbindFramebuffer();
    _myInitValueShader->unbind();

}

void ParticlesDemoApp::setup()
{
    _gravity = Vec3f(0.0,-0.1,0.0);
    _prevElapsedTime = 0;
    _index = 0;
    _prevIndex = 1;
    
    _planePos = Vec3f(0.0,-10.25,0.0);
    _planeNorm = (Vec3f::zero() - Vec3f(0.0,-0.25,0.0) ).normalized();
    
    
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
	
    _myEmitTexture = gl::Texture::create(loadImage(loadAsset("emit01.png")));
    
    
    //UI
    controls = params::InterfaceGl::create("Controls", toPixels(Vec2i(200,100)));
    controls->addParam("Plane pos point1", &_planePos);
    controls->addParam("Plane norm point1", &_planeNorm);
    controls->addParam("Gravity", &_gravity);
    controls->addParam("Bounce", &_bounce);
//    TwDefine( "Controls position='10 50' ");
}

void ParticlesDemoApp::swap() {
    _index = (_index+1)%2;
    _prevIndex = (_index+1)%2;
}

void ParticlesDemoApp::update()
{
    _myDataBuffer[_index].bindFramebuffer();
//    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( _myDataBuffer[_index].getBounds() );
    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
    _myEmitShader->bind();
    //g.texture (0, _myEmitTexture);
    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDataBuffer[_prevIndex].bindTexture(1,1);
    _myDataBuffer[_prevIndex].bindTexture(2,2);
    
    //_myEmitShader.uniform1i("emitTexture", 0);
    _myEmitShader->uniform("positions", 0);
    _myEmitShader->uniform("velocities", 1);
    _myEmitShader->uniform("infos", 2);
    
    Vec3f rnd = Vec3f(randFloat(100.0f),randFloat(100.0f),randFloat(100.0f));
    _myEmitShader->uniform("randomSeed", rnd);
    
    gl::drawSolidRect(_myDataBuffer[_index].getBounds());
    _myEmitShader->unbind();
    _myDataBuffer[_prevIndex].unbindTexture();
    _myDataBuffer[_index].unbindFramebuffer();
    
    swap();
    
    
    _myDataBuffer[_index].bindFramebuffer();
//    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( _myDataBuffer[_index].getBounds() );
    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
    _myThruShader->bind();
    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDataBuffer[_prevIndex].bindTexture(1,1);
    _myDataBuffer[_prevIndex].bindTexture(2,2);
    
    _myThruShader->uniform("positions", 0);
    _myThruShader->uniform("velocities", 1);
    _myThruShader->uniform("infos", 2);
    
    gl::drawSolidRect(_myDataBuffer[_index].getBounds());
    _myThruShader->unbind();
    _myDataBuffer[_index].unbindFramebuffer();
    _myDataBuffer[_prevIndex].unbindTexture();
    
    swap();

    _myDataBuffer[_index].bindFramebuffer();
//    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( _myDataBuffer[_index].getBounds() );
    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
    _myVelocityShader->bind();
    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDataBuffer[_prevIndex].bindTexture(1,1);
    _myDataBuffer[_prevIndex].bindTexture(2,2);
    _myVelocityShader->uniform("positions", 0);
    _myVelocityShader->uniform("velocities", 1);
    _myVelocityShader->uniform("infos", 2);
    _myVelocityShader->uniform("gravity", _gravity);

    gl::drawSolidRect(_myDataBuffer[_index].getBounds());

    _myVelocityShader->unbind();
    _myDataBuffer[_index].unbindFramebuffer();
    _myDataBuffer[_prevIndex].unbindTexture();
    
    swap();

    
    _plane.set(_planePos, _planeNorm);
    
    _constraints[0] = _plane.getPoint();
    _constraintsNormal[0] = _plane.getNormal();
    
    _myDataBuffer[_index].bindFramebuffer();
//    gl::clear( Color( 0, 0, 0 ) );
    gl::color(1.0, 1.0, 1.0, 1.0);
    gl::setViewport( _myDataBuffer[_index].getBounds() );
    gl::setMatricesWindow(_myDataBuffer[_index].getSize());
    _myUpdateShader->bind();
    _myDataBuffer[_prevIndex].bindTexture(0,0);
    _myDataBuffer[_prevIndex].bindTexture(1,1);
    _myDataBuffer[_prevIndex].bindTexture(2,2);
    _myConstraintData.bindTexture(3,0);
    
    _myUpdateShader->uniform("positions", 0);
    _myUpdateShader->uniform("velocities", 1);
    _myUpdateShader->uniform("infos", 2);
    _myUpdateShader->uniform("constraints", 3);
//    _myUpdateShader->uniform("constraintsPos", _constraints, 1);
//    _myUpdateShader->uniform("constraintsNorm", _constraintsNormal, 1);
//    _myUpdateShader->uniform("bounce", _bounce);
    
    float deltaT = getElapsedSeconds()-_prevElapsedTime;
    _myUpdateShader->uniform("deltaTime", deltaT);
    _myUpdateShader->uniform("dataW", _myConstraintData.getWidth());
    _myUpdateShader->uniform("dataH", _myConstraintData.getHeight());
    
    gl::drawSolidRect(_myDataBuffer[_index].getBounds());
    _myUpdateShader->unbind();
    _myDataBuffer[_prevIndex].unbindTexture();
    _myDataBuffer[_index].unbindFramebuffer();
    
    swap();
    _prevElapsedTime = getElapsedSeconds();
}

void ParticlesDemoApp::draw()
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
    gl::draw(_myMesh);
    _myDisplayShader->unbind();
    _myDataBuffer[_prevIndex].unbindTexture();
    
    gl::drawCoordinateFrame(1.0f*0.05, 0.2f*0.05, 0.05f*0.05);
    
    gl::pushMatrices();
    gl::color(0.25, 0.1, 0.1, 1.0);

    Vec3f p0;
    Vec3f p1;
    
    
    p0 = _plane.getNormal().cross(Vec3f::yAxis());
    
    if (p0==Vec3f::zero()) {
        p0 = _plane.getNormal().cross(Vec3f::xAxis());
    }
    
    p1 = _plane.getNormal().cross(Vec3f::zAxis());
    
    
    gl::begin(GL_QUADS);
    {
        gl::vertex(_plane.getPoint()+p0*100);
        gl::vertex(_plane.getPoint()+p1*100);
        gl::vertex(_plane.getPoint()-p0*100);
        gl::vertex(_plane.getPoint()-p1*100);

    }
    gl::end();
    
    
    
    
    
    
    
    gl::popMatrices();
    
    gl::setMatricesWindow(toPixels(getWindowSize()));
    gl::setViewport(toPixels(getWindowBounds()));
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
//    gl::draw( _myDataBuffer[_prevIndex].getTexture(0), toPixels(Area(0,0,100,100)));
    gl::drawString("FPS: " + toString(getAverageFps()), Vec2f(10.0f,10.0f));
    controls->draw();
}

void ParticlesDemoApp::loadShaders()
{
    try {
        _myEmitShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("emit_sphere_fp.glsl"));
        _myDisplayShader = gl::GlslProg::create( loadAsset("display_vp.glsl"), loadAsset("display_fp.glsl"));
        _myVelocityShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("force_fp.glsl"));
        _myThruShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("thru_fp.glsl"));
        _myUpdateShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("update_constraints_fp.glsl"));
        _myInitValueShader = gl::GlslProg::create( loadAsset("passThru_vp.glsl"), loadAsset("init_fp.glsl"));
    } catch ( gl::GlslProgCompileExc &exc ) {
        console()<<"Error compiling shader" << endl;
        console() << exc.what();
    }
    
}

void ParticlesDemoApp::keyUp( KeyEvent event )
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

void ParticlesDemoApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown( event.getPos() );
}
void ParticlesDemoApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void ParticlesDemoApp::resize( )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio(getWindowAspectRatio());
    
    mMayaCam.setCurrentCam(cam);
}

CINDER_APP_NATIVE( ParticlesDemoApp, RendererGl )
