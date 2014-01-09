#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"
#include "Vao.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gl4DemoApp : public AppNative {
  public:
	void setup();
    void prepareSettings( Settings *settings);
	void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyDown( KeyEvent event );

	void update();
	void resize();
	void draw();
    
    
    gl::GlslProgRef     mShaderProg;
    
    gl::Vbo             mVboHandles[2];
    
    Vao                 mVaoHandle;
    MayaCamUI           mMayaCam;
    Color               color;
    params::InterfaceGlRef mParams;
    
    string              fps;
};

void gl4DemoApp::prepareSettings( Settings *settings){
    
//    settings->enableHighDensityDisplay();
    settings->setWindowSize(640, 480);
    settings->setFullScreen();
//    settings->setBorderless();
}

void gl4DemoApp::setup()
{

    console() << "GL Vendor             : " << glGetString( GL_RENDERER ) << endl;
    console() << "GL Renderer           : " << glGetString( GL_VENDOR ) << endl;
    console() << "GL Version (String)   : " << glGetString( GL_VERSION ) << endl;
    
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    console() << "GL Version (Int)      : " << major<<"."<<minor << endl;
    console() << "GLSL Version          : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;
    
    mShaderProg = gl::GlslProg::create( loadAsset("shader.vert"), loadAsset("shader.frag"));
    
    
    float points[] = {
        0.0f,  0.1f,  0.0f,
        0.1f, -0.1f,  0.0f,
        -0.1f, -0.1f,  0.0f
    };
    
    float colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    
//    Create and set-up the vertex array object

    int attribLoc = mShaderProg->getAttribLocation("VertexPosition");
    
    mVaoHandle.generate();

    
    mVboHandles[0] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboHandles[0].bind();
    mVboHandles[0].bufferData(9*sizeof(float), points, GL_STATIC_DRAW);
    
    mVaoHandle.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
    
    
    attribLoc = mShaderProg->getAttribLocation("VertexColor");
    
    mVboHandles[1] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboHandles[1].bind();
    mVboHandles[1].bufferData(9*sizeof(float), colors, GL_STATIC_DRAW);
    
    mVaoHandle.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
    
    
    mVaoHandle.unbind();

    CameraPersp cam = mMayaCam.getCamera();
    
    cam.setAspectRatio(getWindowAspectRatio());
    cam.setEyePoint(Vec3f(0.0,0.0,-1.0));
    cam.setCenterOfInterestPoint(Vec3f::zero());

    mMayaCam.setCurrentCam(cam);
    
    mParams = params::InterfaceGl::create("ANTTWEAKBAR OGL4", Vec2i(200,100));

    mParams->addText("GL Version: " + toString(major)+"."+toString(minor));
    mParams->addText("GLSL Version: " + toString(glGetString( GL_SHADING_LANGUAGE_VERSION )));
    mParams->addParam("FPS: ", &fps);
    mParams->addParam("clear color", &color);
}

void gl4DemoApp::mouseDown( MouseEvent event )
{
    mMayaCam.mouseDown(event.getPos());
}

void gl4DemoApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
}

void gl4DemoApp::keyDown( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_f:
            setFullScreen(!isFullScreen());
        break;
        case KeyEvent::KEY_ESCAPE:
        quit();
        default:
        break;
    }
}

void gl4DemoApp::resize()
{
    CameraPersp cam = mMayaCam.getCamera();
    
    cam.setAspectRatio((getWindowAspectRatio()));
    mMayaCam.setCurrentCam(cam);
}

void gl4DemoApp::update()
{
    fps = toString(getAverageFps());
}

void gl4DemoApp::draw()
{
	// clear out the window with black
	gl::clear( color );
    gl::setViewport(toPixels(getWindowBounds()));

    gl::color(color);

    mShaderProg->bind();
    
    mShaderProg->uniform("projectionMatrix", &mMayaCam.getCamera().getProjectionMatrix(), 1);
    mShaderProg->uniform("modelViewMatrix", &mMayaCam.getCamera().getModelViewMatrix(), 1);

    mVaoHandle.bind();
    

    glDrawArrays (GL_TRIANGLES, 0, 3);

    mVaoHandle.unbind();

    mParams->draw();
    
    mShaderProg->unbind();
}

CINDER_APP_NATIVE( gl4DemoApp, RendererGl )
