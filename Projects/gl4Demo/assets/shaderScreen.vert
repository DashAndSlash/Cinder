
#version 400
layout (location = 9) in vec3 VertexPosition;
layout (location = 10) in vec2 TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
out vec2 st;

void main() {

    st = TexCoord;
    gl_Position = modelViewMatrix * vec4( VertexPosition, 1.0 );
}