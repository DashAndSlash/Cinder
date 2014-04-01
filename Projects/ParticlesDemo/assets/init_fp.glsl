#version 120 

void main(){
	
	gl_FragColor = vec4(gl_TexCoord[0].xyz, 1);
}