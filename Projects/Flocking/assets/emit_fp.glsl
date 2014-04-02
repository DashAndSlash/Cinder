#version 120 

// particle data textures
uniform sampler2D positions;


uniform vec3 randomSeed;

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){

    vec2 st = gl_TexCoord[0].xy;
	vec4 pos = texture2D (positions, st);
		
	gl_FragData[0] = pos*10.0;
	gl_FragData[1] = vec4(0.0);//rand(st+200),rand(st+100),rand(st+300),1.0);	// no initial velocity
	gl_FragData[2] = vec4(0.2,1,1,1.0);    // lifetime 1 second
}