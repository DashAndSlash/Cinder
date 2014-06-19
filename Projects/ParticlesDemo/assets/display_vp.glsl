#version 120 

#define e 2.7182

uniform sampler2D positions;

void main(){

	vec4 position = texture2D(positions, gl_Vertex.xy);
	gl_Position = gl_ModelViewProjectionMatrix * position;
	gl_TexCoord[0] = vec4(gl_Vertex.xy,0,0);
	
	
	//float c = pow(e, gl_Position.y);
	//float c = gl_Position.y / 1000.0;
	
	gl_FrontColor.rgb = vec3(1.0);

}           