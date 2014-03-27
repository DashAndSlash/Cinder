#version 400
in vec2 st;
out vec4 FragColor;
uniform sampler2D tex;
void main() {
    vec4 imgColor = texture(tex,st);
    FragColor = vec4(1.0,1.0,1.0, 1.0)*imgColor;
}