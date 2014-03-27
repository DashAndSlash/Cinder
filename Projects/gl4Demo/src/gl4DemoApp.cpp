#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/MayaCamUI.h"
#include "Vao.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "Quad.h"

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

    void initFbo();
    void setTriangle();
    void setScreenQuad();
    void setTexQuad();
    
    void renderToFbo();
    void renderTex();
    void renderTriangle();
    
    
	void update();
	void resize();
	void draw();
    
    
    gl::GlslProgRef     mShaderScene;
    gl::GlslProgRef     mShaderScreen;

    gl::Vbo             mVboTriangle[2];
    gl::Vbo             mVboScreenQuad[2];
    gl::Vbo             mVboTexQuad[2];
    
    gl::TextureRef       mTexture;
    
    shared_ptr<gl::Fbo> testFbo;
    
    gl4::Vao            mVaoTriangle;
    gl4::Vao            mVaoScreenQuad;
    gl4::Vao            mVaoTextureQuad;
    
    MayaCamUI           mMayaCam;
    
    MayaCamUI           mScreenCam;
    
    Color               color;
    params::InterfaceGlRef mParams;
    
    string              fps;
    
    gl4::Quad           mRenderQuad;
    
    Matrix44f           mIdentity;

};

void gl4DemoApp::prepareSettings( Settings *settings){
    
//    settings->enableHighDensityDisplay();
    settings->setWindowSize(640, 640);
    settings->setFullScreen();
//    settings->setBorderless();
    
}

void gl4DemoApp::setup()
{
    try {
        gl::Texture::Format format;
        format.enableMipmapping();
        format.setInternalFormat(GL_RGBA32F);
        format.setMinFilter(GL_LINEAR);
        format.setMagFilter(GL_LINEAR);
        mTexture = gl::Texture::create(loadImage(loadAsset("img.png")),format);

    } catch ( ... ) {
        console() << "error loading image" <<  endl;
    }
    
    console() << "GL Vendor             : " << glGetString( GL_RENDERER ) << endl;
    console() << "GL Renderer           : " << glGetString( GL_VENDOR ) << endl;
    console() << "GL Version (String)   : " << glGetString( GL_VERSION ) << endl;
    
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    console() << "GL Version (Int)      : " << major<<"."<<minor << endl;
    console() << "GLSL Version          : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;
    
    mShaderScene = gl::GlslProg::create( loadAsset("shader.vert"), loadAsset("shader.frag"));
    
    mShaderScreen = gl::GlslProg::create( loadAsset("shaderScreen.vert"), loadAsset("shaderScreen.frag") );
    
    mVaoScreenQuad.generate();
    mVaoTextureQuad.generate();
    
    setTriangle();

    CameraPersp cam = mMayaCam.getCamera();
    
    cam.setAspectRatio(getWindowAspectRatio());
    cam.setEyePoint(Vec3f(0.0,0.0,-1.0));
    cam.setCenterOfInterestPoint(Vec3f::zero());
    
    mMayaCam.setCurrentCam(cam);

    
    cam = mMayaCam.getCamera();
    
    cam.setAspectRatio(getWindowAspectRatio());
    cam.setEyePoint(Vec3f(0.0,0.0,-1.0));
    cam.setCenterOfInterestPoint(Vec3f::zero());
    
    mScreenCam.setCurrentCam(cam);

    
    
    mParams = params::InterfaceGl::create("ANTTWEAKBAR OGL4", Vec2i(200,100));

    mParams->addText("GL Version: " + toString(major)+"."+toString(minor));
    mParams->addText("GLSL Version: " + toString(glGetString( GL_SHADING_LANGUAGE_VERSION )));
    mParams->addParam("FPS: ", &fps);
    mParams->addParam("clear color", &color);
    

//    mRenderQuad.generate();

}
void gl4DemoApp::setTriangle()
{
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
    
    int attribLoc = mShaderScene->getAttribLocation("VertexPosition");
    
    mVaoTriangle.generate();
    
    mVboTriangle[0] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboTriangle[0].bind();
    mVboTriangle[0].bufferData(9*sizeof(float), points, GL_STATIC_DRAW);
    
    mVaoTriangle.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
    
    attribLoc = mShaderScene->getAttribLocation("VertexColor");
    
    mVboTriangle[1] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboTriangle[1].bind();
    mVboTriangle[1].bufferData(9*sizeof(float), colors, GL_STATIC_DRAW);
    
    mVaoTriangle.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
    
    
    mVaoTriangle.unbind();

}

void gl4DemoApp::setScreenQuad()
{
    
    //    mRenderQuad.setPoints( Area(-1.0f*getWindowAspectRatio(),-1.0f,1.0f*getWindowAspectRatio(),1.0f) );
    //    mRenderQuad.setPoints( Area(0.0,0.0,1.0,1.0) );
    //    mRenderQuad.initShader( loadAsset("shaderScreen.vert"), loadAsset("shaderScreen.frag"));
    //    mRenderQuad.update();
    
    float pointsScreenQuad[] = {
        -1.0f,  -1.0f,  0.0f,
        -1.0f, 1.0f,  0.0f,
        1.0f, -1.0f,  0.0f,
        1.0f, 1.0f,  0.0f
    };
    
    float texCoordScreenQuad[] = {
        0.0f,  0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };
    
    //    Create and set-up the vertex array object
    
    int attribLoc = mShaderScreen->getAttribLocation("VertexPosition");
    mVboScreenQuad[0] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboScreenQuad[0].bind();
    mVboScreenQuad[0].bufferData(12*sizeof(float), pointsScreenQuad, GL_STATIC_DRAW);
    
    mVaoScreenQuad.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
    
    attribLoc = mShaderScreen->getAttribLocation("TexCoord");
    mVboScreenQuad[1] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboScreenQuad[1].bind();
    mVboScreenQuad[1].bufferData(8*sizeof(float), texCoordScreenQuad, GL_STATIC_DRAW);
    
    mVaoScreenQuad.vertexAttribPointer(attribLoc, 2, GL_FLOAT, 0, 0);
    
    mVaoScreenQuad.unbind();
    
}

void gl4DemoApp::setTexQuad()
{
    
    float pointsTexQuad[] = {
        -1.0f,  1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,
        1.0f, 1.0f,  0.0f,
        1.0f, -1.0f,  0.0f
    };
    
    float texCoordQuad[] = {
        
        0.0f,  0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
        
    };
    
    //    Create and set-up the vertex array object
    
    //By using layout on the shader instead of querying the value
    int attribLoc = 9;//mShaderScreen->getAttribLocation("VertexPosition");
    mVboTexQuad[0] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboTexQuad[0].bind();
    mVboTexQuad[0].bufferData(12*sizeof(float), pointsTexQuad, GL_STATIC_DRAW);
    
    mVaoTextureQuad.vertexAttribPointer(9, 3, GL_FLOAT, 0, 0);
    
    //By using layout on the shader instead of querying the value
    attribLoc = 10;//mShaderScreen->getAttribLocation("TexCoord");
    mVboTexQuad[1] = gl::Vbo(GL_ARRAY_BUFFER);
    mVboTexQuad[1].bind();
    mVboTexQuad[1].bufferData(8*sizeof(float), texCoordQuad, GL_STATIC_DRAW);
    
    mVaoTextureQuad.vertexAttribPointer(10, 2, GL_FLOAT, 0, 0);
    
    mVaoTextureQuad.unbind();
    
}

void gl4DemoApp::initFbo()
{
    gl::Fbo::Format format;
    format.enableMipmapping();
    format.enableDepthBuffer();
    format.setCoverageSamples(4);
    format.setSamples(4);
    format.setColorInternalFormat(GL_RGBA32F);
    format.setMinFilter(GL_LINEAR);
    format.setMagFilter(GL_LINEAR);
    testFbo = shared_ptr<gl::Fbo>(new gl::Fbo( getWindowWidth(), getWindowHeight(), format ));
    
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
    
    cam = mScreenCam.getCamera();
    cam.setAspectRatio((getWindowAspectRatio()));
    mScreenCam.setCurrentCam(cam);
    
    initFbo();
}

void gl4DemoApp::update()
{

    fps = toString(getAverageFps());
}

void gl4DemoApp::renderTriangle()
{
    
    mShaderScene->bind();
    
    mShaderScene->uniform("projectionMatrix", &mMayaCam.getCamera().getProjectionMatrix(), 1);
    mShaderScene->uniform("modelViewMatrix", &mMayaCam.getCamera().getModelViewMatrix(), 1);

    mVaoTriangle.bind();
    
    glDrawArrays (GL_TRIANGLES, 0, 3);
    
    mVaoTriangle.unbind();

    mShaderScene->unbind();

}

void gl4DemoApp::renderToFbo()
{
    testFbo->bindFramebuffer();
    
    gl::clear( ColorA(32.0/255.0,32.0/255.0,32.0/255.0,1.0) );
    
    renderTriangle();
    
    testFbo->unbindFramebuffer();
}

void gl4DemoApp::renderTex()
{
    mTexture->bind(1);
    
    mShaderScreen->bind();
    
    Matrix44f imgMatrix;
    
    
    imgMatrix.scale(Vec3f( (mTexture->getWidth()/((float)getWindowWidth())),
                          (mTexture->getHeight()/((float)getWindowHeight())),1.0f));
    
//    mShaderScreen->uniform("projectionMatrix", &mIdentity, 1);
    mShaderScreen->uniform("modelViewMatrix", &imgMatrix, 1);
    mShaderScreen->uniform("tex", 1);
    
    setTexQuad();
    mVaoTextureQuad.bind();
    
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    
    mVaoTextureQuad.unbind();
    
    mTexture->unbind();
}

void gl4DemoApp::draw()
{
//    gl::enableDepthWrite();
    
	gl::clear( color );
    gl::setViewport((getWindowBounds()));
    
    renderToFbo();
    
    renderTriangle();
    
    gl::setViewport((Area(0.0, 0.0, getWindowWidth()*0.25, getWindowHeight()*0.25)));
    
    testFbo->bindTexture(0);
    
    mShaderScreen->bind();
    
    mShaderScreen->uniform("modelViewMatrix", &mIdentity, 1);
    mShaderScreen->uniform("tex", 0);

    setScreenQuad();
    mVaoScreenQuad.bind();

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

    mVaoScreenQuad.unbind();

    testFbo->unbindTexture();

    mShaderScreen->unbind();

    gl::setViewport((Area(getWindowWidth(), 0.0, getWindowWidth()*0.75, getWindowHeight()*0.25)));
    renderTex();
    
    mParams->draw();

}

CINDER_APP_NATIVE( gl4DemoApp, RendererGl(8) )
