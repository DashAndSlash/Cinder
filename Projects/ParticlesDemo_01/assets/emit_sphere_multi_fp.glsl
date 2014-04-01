#version 120 

// particle data textures
uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;
uniform sampler2D clusterCenters;

uniform int dataW;
uniform int dataH;


vec2 lookup (int col, int row) {
	return vec2 ((float(col)+0.5)/dataW, (float(row)+0.5)/dataH);
}

uniform vec3 randomSeed;

float rand (vec2 n){
  return fract(sin(dot(n.xy, randomSeed.xy))* randomSeed.z);
}

void main(){

	vec3 pos = texture2D (positions, gl_TexCoord[0].xy).xyz;
	vec3 vel = texture2D (velocities, gl_TexCoord[0].xy).xyz;
	vec4 inf = texture2D (infos, gl_TexCoord[0].xy);
	
	vec3 newPos = pos;
	vec3 newVel = vel;
	vec4 newInf = inf;
	
	for (int i=0; i<dataW; i++) {
	
		vec3 clusterCoords = texture2D (clusterCenters, lookup(i,0)).xyz;
		vec3 clusterInfo   = texture2D (clusterCenters, lookup(i,1)).xyz;
		
		if (inf.x==0 && gl_TexCoord[0].x>clusterInfo.y && gl_TexCoord[0].x<=clusterInfo.z) {
			float theta = rand(gl_TexCoord[0].xy)*2*3.1417;
			float phi = rand(gl_TexCoord[0].xy+vec2(100,0))*2*3.1417;
			float r = rand(gl_TexCoord[0].xy+vec2(200,0))*clusterInfo.x;
			
			float x = clusterCoords.x + r*sin(theta)*cos(phi);
			float y = clusterCoords.y + r*sin(theta)*sin(phi);
			float z = clusterCoords.z + r*cos(phi);
			
			newPos = vec3(x,y,z);
			newVel = vec3(0,0,0);
			float lifeTime = rand(gl_TexCoord[0].xy+vec2(300,0))*10+10;
			float mass = 0;
			newInf = vec4(lifeTime, mass, 0, 1);
		}
	}
	
	gl_FragData[0] = vec4 (newPos,1);
	gl_FragData[1] = vec4 (newVel,1);	// no initial velocity
	gl_FragData[2] = newInf;    // lifetime 1 second
}