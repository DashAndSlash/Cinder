#version 120 

// particle data textures
uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;

// emit input texture
uniform sampler2D emitTexture;


uniform vec3 randomSeed;

float rand (vec2 n){
  return fract(sin(dot(n.xy, randomSeed.xy))* randomSeed.z);
}

void main(){

	vec4 pos = texture2D (positions, gl_TexCoord[0].xy);
	vec4 vel = texture2D (velocities, gl_TexCoord[0].xy);
	vec4 inf = texture2D (infos, gl_TexCoord[0].xy);
	
	vec4 newPos = pos;
	vec4 newVel = vel;
	vec4 newInf = inf;
	
	int w = 1000;
	int h = 1000;
	
	// we only emit, if lifetime for this particle slot is expired, otherwise we just forward the old data
	if (inf.x==0) {
		// do a random lookup in texture and emit, if pixel value is above threshold		
		vec2 lookupPos = vec2(w*rand(gl_TexCoord[0].xy), h*rand(gl_TexCoord[0].xy+vec2(100,0)));
		
		vec4 emit = texture2D (emitTexture, lookupPos);
		if (emit.x>0) {
			newPos = vec4(lookupPos,0,1);
			newVel = vec4(0,0,0,0);	// no initial velocity
			newInf = vec4(1,0,0,0); // lifetime 1 second
		}
		//newPos = vec4(rand(gl_TexCoord[0].xy), rand(gl_TexCoord[0].xy+vec2(100,0)), 0, 1);
		//newInf  = inf + vec4(0.8,0,0,1);	
	}
	
	gl_FragData[0] = newPos;
	gl_FragData[1] = newVel;	// no initial velocity
	gl_FragData[2] = newInf;    // lifetime 1 second
}