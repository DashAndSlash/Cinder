#version 120 

uniform sampler2D velocities;

void main(){
	
	vec4 newVelocity = texture2D (velocities, gl_TexCoord[0].xy);
	gl_FragData[1] = newVelocity + vec4(0,-2,0,1);
}