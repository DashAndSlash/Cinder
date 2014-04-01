#version 120 

// particle data textures
uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;


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
	
	// only emit if slot empty
	if (inf.x==0) {
		float theta = rand(gl_TexCoord[0].xy)*2*3.1417;
		float phi = rand(gl_TexCoord[0].xy+vec2(100,0))*2*3.1417;
		float r = rand(gl_TexCoord[0].xy+vec2(200,0))*1.1;
		
		float x = r*sin(theta)*cos(phi);
		float y = r*sin(theta)*sin(phi);
		float z = r*cos(phi);
		
		newPos = vec4(x,y,z,1);
		newVel = vec4(x,y,y,1);
//		newVel = vec4(0.0,0.0,0.0,1.0);
		float lifeTime = rand(gl_TexCoord[0].xy+vec2(300,0))*10.0;
		newInf = vec4(lifeTime,0,0,1);
	}
	
	gl_FragData[0] = newPos;
	gl_FragData[1] = newVel;	// no initial velocity
	gl_FragData[2] = newInf;    // lifetime 1 second
}