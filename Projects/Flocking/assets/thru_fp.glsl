#version 120 

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;


void main(){
	
	gl_FragData[0] = texture2D (positions, gl_TexCoord[0].xy);
	gl_FragData[1] = texture2D (velocities, gl_TexCoord[0].xy);
	gl_FragData[2] = texture2D (infos, gl_TexCoord[0].xy); 
}