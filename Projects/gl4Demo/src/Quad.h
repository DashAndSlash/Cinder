//
//  Quad.h
//  gl4Demo
//
//  Created by luca lolli on 22/03/2014.
//
//

#ifndef __gl4Demo__Quad__
#define __gl4Demo__Quad__

#include <iostream>
#include "cinder/app/AppNative.h"
#include "cinder/Utilities.h"
#include "cinder/gl/Vbo.h"
#include "Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"


#define GLSL(version, shader)  "#version " #version "\n" #shader


using namespace ci;
using namespace ci::app;
using namespace std;

namespace gl4 {
    class Quad{
    public:

        Quad();
        Quad(Area area);
        
        void setPoints(Area area);
        void setTexCoord(Area coord);
        
        void generate();
        
        void initShader(DataSourceRef vert, DataSourceRef frag);
        
        void update();
        
        void bind(Matrix44f projection, Matrix44f modelView, gl::Texture tex);
        void unbind();
        
    private:
        Area        mArea;
        shared_ptr<float>       points;
        shared_ptr<float>       texCoord;
        vector<Vec3f>       color;
        gl::Vbo             mVboQuad[2];
        gl4::Vao            mVaoQuad;
        
        gl::GlslProgRef     mShader;
        };
    
}

#endif /* defined(__gl4Demo__Quad__) */
