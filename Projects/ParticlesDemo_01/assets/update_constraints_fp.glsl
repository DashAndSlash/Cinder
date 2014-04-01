#version 120 

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;
uniform sampler2D constraints;

uniform float deltaTime;

uniform int dataW;
uniform int dataH;


float getLookup (int pos, int width) {
	return (float (pos)+0.5) / width;
}

vec2 lookup (int col, int row) {
	return vec2  ((float(col)+0.5)/dataW, (float(row)+0.5)/dataH);
}

void main(){
	
	vec3 position    = texture2D (positions, gl_TexCoord[0].xy).xyz;
	vec3 newVelocity = texture2D (velocities, gl_TexCoord[0].xy).xyz;
	vec4 newInfo     = texture2D (infos, gl_TexCoord[0].xy);

	// move
	vec3 newPosition = position + newVelocity*deltaTime;
	
	// constraints
	for (int i=0; i<2; i++) {
		vec3 p = texture2D (constraints, lookup(i,0)).xyz;
		vec3 n = texture2D (constraints, lookup(i,1)).xyz;
		
		// check distance to plane
		float d = dot (newPosition-p, n);
		
		// reflect
		if (d<0) {
			//newVelocity = newVelocity*0;
			newVelocity = newVelocity - 2*n*dot (n, newVelocity);
		}
		
		//if (newPosition.y < p.y) newVelocity*=0;
	}
	
	newPosition = position + newVelocity*deltaTime;

	// lifetime
	if (newInfo.x>0) {
		newInfo.x -= deltaTime;
	}
	else {
		newInfo.x = 0;
	}
	
	gl_FragData[0] = vec4(newPosition,1);
	gl_FragData[1] = vec4(newVelocity,1);
	gl_FragData[2] = newInfo; 
}