//
//  vao.h
//  gl4Demo
//
//  Created by luca lolli on 24/12/2013.
//
//

#ifndef __gl4Demo__Vao__
#define __gl4Demo__Vao__

namespace gl4{
    
    class Vao {
    
    public:
        Vao();
        
        void        generate();
        void		bind();
        void		unbind();
        
        void		vertexAttribPointer( GLuint attribLoc, size_t size, GLenum type, GLsizei stride, GLsizei offset, GLboolean normalized = GL_FALSE);
        
        GLuint		getId() const { return mObj->mId; }
        
    protected:
        struct Obj {
            Obj( );
            ~Obj();
            
            GLuint			mId;
        };
        
        std::shared_ptr<Obj>	mObj;
        
    public:
        //@{
        //! Emulates shared_ptr-like behavior
        typedef std::shared_ptr<Obj> Vao::*unspecified_bool_type;
        operator unspecified_bool_type() const { return ( mObj.get() == 0 ) ? 0 : &Vao::mObj; }
        void reset() { mObj.reset(); }
        //@}
    };
    
    class VaoExc : public std::exception {
    public:
        virtual const char* what() const throw() { return "OpenGL Vao exception"; }
    };
    
    class VaoInvalidTargetExc : public VaoExc {
    public:
        virtual const char* what() const throw() { return "OpenGL Vao exception: Invalid Target"; }
    };
    
    class VaoFailedMapExc : public VaoExc {
    public:
        virtual const char* what() const throw() { return "OpenGL Vao exception: Map failure"; }
    };
    
    class VaoFailedUnmapExc : public VaoExc {
    public:
        virtual const char* what() const throw() { return "OpenGL Vao exception: Unmap failure"; }
    };

}

#endif /* defined(__gl4Demo__Quad__) */