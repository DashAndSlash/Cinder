#version 120 

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D infos;

uniform vec3    gravity;

void main(){
	
	vec4 newVelocity = texture2D (velocities, gl_TexCoord[0].xy);
    vec4 pos = texture2D (positions, gl_TexCoord[0].xy);
    
    vec3 force = vec3(gravity);
    vec4 attractor = vec4(10,0,0,1);
    
    vec3 diff = attractor.xyz - pos.xyz;
    
    vec3 dir = normalize(diff);
    
    float mag = length(diff);
    
    
    if(mag!=0.0)
        force += 3.0 * dir *(1.0/(mag*mag));
    
    

    
    gl_FragData[1].xyz = newVelocity.xyz + force;
    gl_FragData[0] = texture2D(positions,gl_TexCoord[0].xy);
    gl_FragData[2] = texture2D(infos,gl_TexCoord[0].xy);
}