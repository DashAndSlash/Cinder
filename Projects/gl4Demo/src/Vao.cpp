//
//  vao.cpp
//  gl4Demo
//
//  Created by luca lolli on 24/12/2013.
//
//

#include "Vao.h"
#include <sstream>

using namespace std;

Vao::Obj::Obj()
{
	glGenVertexArrays( 1, &mId );
}

Vao::Obj::~Obj()
{
	glDeleteVertexArrays( 1, &mId );
}

Vao::Vao( )
{

}

void Vao::generate()
{
	mObj = shared_ptr<Vao::Obj>( new Obj( ) );
}

void Vao::bind()
{
	glBindVertexArray( mObj->mId );
}

void Vao::unbind()
{
	glBindVertexArray( 0 );

}


void Vao::vertexAttribPointer( GLuint attribLoc, size_t size, GLenum type, GLsizei stride, GLsizei offset, GLboolean normalized)
{
    bind();
    glEnableVertexAttribArray(attribLoc);
	glVertexAttribPointer( attribLoc, (GLint)size, type, normalized, stride, (void *) offset);

}




/// Binds the VAO and sets the vertex attrib pointer.
/// Params:
/// type = Specifies the data type of each component in the array.
/// size = Specifies the number of components per generic vertex attribute.
/// offset = Specifies a offset of the first component of the first generic vertex attribute in the array in the data store of the buffer.
/// stride = Specifies the byte offset between consecutive generic vertex attributes.
/// normalized = Specifies whether fixed-point data values should be normalized (GL_TRUE) or
///                converted directly as fixed-point values (GL_FALSE = default) when they are accessed.


/// ditto
