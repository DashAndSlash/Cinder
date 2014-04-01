#version 120 

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D attractors;

uniform int dataW; // TODO: move this for all shader stages into vertex shader
uniform int dataH;

vec2 lookup (int col, int row) {
	return vec2 ((float(col)+0.5)/dataW, (float(row)+0.5)/dataH);
}

void main(){
	
	vec3 newVelocity = texture2D (velocities, gl_TexCoord[0].xy).xyz;
	vec3 position    = texture2D (positions, gl_TexCoord[0].xy).xyz;
	
	for (int i=0; i<dataW; i++) {
		vec3 v = texture2D (attractors, lookup(i,0)).xyz;
		vec3 w = texture2D (attractors, lookup(i,1)).xyz;
		
		//v = vec3(0,0,0);
		vec3 d =  v - position;
		float sl = d.x*d.x + d.y*d.y + d.z*d.z;
		newVelocity += w.x*d / pow(length(d),1);
		newVelocity *= 0.99;
		
	}
	
	gl_FragData[1] = vec4 (newVelocity,1);
}