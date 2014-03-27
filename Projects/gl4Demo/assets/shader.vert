
#version 400
in vec3 VertexPosition;
in vec3 VertexColor;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

out vec3 Color;
void main() {
    Color = VertexColor;
    gl_Position = projectionMatrix * modelViewMatrix * vec4( VertexPosition, 1.0 );
}