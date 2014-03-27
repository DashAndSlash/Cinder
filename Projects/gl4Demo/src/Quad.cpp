//
//  Quad.cpp
//  gl4Demo
//
//  Created by luca lolli on 22/03/2014.
//
//

#include "Quad.h"

namespace gl4 {
    
    Quad::Quad()
    {
        
    }

    Quad::Quad(Area area) : mArea(area)
    {
        points = shared_ptr<float>(new float[12]);
        
        points.get()[0] = area.x2; points.get()[1] = area.y1; points.get()[2] = 0.0f ;
        points.get()[3] = area.x2; points.get()[4] = area.y2; points.get()[5] = 0.0f ;
        points.get()[6] = area.x1; points.get()[7] = area.y1; points.get()[8] = 0.0f ;
        points.get()[9] = area.x1; points.get()[10] = area.y2; points.get()[11] = 0.0f;
        
        
    }
    
    void Quad::initShader(DataSourceRef vert, DataSourceRef frag)
    {
        try {
        mShader = gl::GlslProg::create( vert, frag);
        } catch ( ... ) {
            console() << "error" << endl;
        }

    }
    
    void Quad::generate()
    {
        mVaoQuad.generate();
        update();
    }
    
    void Quad::update()
    {
        int attribLoc = mShader->getAttribLocation("VertexPosition");
        mVboQuad[0] = gl::Vbo(GL_ARRAY_BUFFER);
        mVboQuad[0].bind();
        mVboQuad[0].bufferData(16*sizeof(float), &points, GL_STATIC_DRAW);
        
        mVaoQuad.vertexAttribPointer(attribLoc, 3, GL_FLOAT, 0, 0);
        
        attribLoc = mShader->getAttribLocation("TexCoord");
        mVboQuad[1] = gl::Vbo(GL_ARRAY_BUFFER);
        mVboQuad[1].bind();
        mVboQuad[1].bufferData(8*sizeof(float), &texCoord, GL_STATIC_DRAW);
        
        mVaoQuad.vertexAttribPointer(attribLoc, 2, GL_FLOAT, 0, 0);
        
        mVaoQuad.unbind();
    }
    
    void Quad::bind(Matrix44f projection, Matrix44f modelView, gl::Texture tex)
    {
        tex.bind(0);
        mShader->bind();
        
        mShader->uniform("projectionMatrix", &projection, 1);
        mShader->uniform("modelViewMatrix", &modelView, 1);
        mShader->uniform("tex", 0);
        
        mVaoQuad.bind();
        
    }
    
    void Quad::unbind()
    {
        mVaoQuad.unbind();
        
        mShader->unbind();

    }
    
    void Quad::setPoints(Area area)
    {
        
        points = shared_ptr<float>(new float[12]);
        
        points.get()[0] = area.x2; points.get()[1] = area.y1; points.get()[2] = 0.0f ;
        points.get()[3] = area.x2; points.get()[4] = area.y2; points.get()[5] = 0.0f ;
        points.get()[6] = area.x1; points.get()[7] = area.y1; points.get()[8] = 0.0f ;
        points.get()[9] = area.x1; points.get()[10] = area.y2; points.get()[11] = 0.0f;
        
    }
    
    void Quad::setTexCoord(Area coord)
    {
        texCoord = shared_ptr<float>(new float[8]);
        
        texCoord.get()[0] = coord.x1; texCoord.get()[1] = coord.y1;
        texCoord.get()[2] = coord.x1; texCoord.get()[3] = coord.y2;
        texCoord.get()[4] = coord.x2; texCoord.get()[5] = coord.y1;
        texCoord.get()[6] = coord.x2; texCoord.get()[7] = coord.y2;
        
    }
    
    
}
