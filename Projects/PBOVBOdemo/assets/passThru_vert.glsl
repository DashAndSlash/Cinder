
varying vec3 pos;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 eyeCoord = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * eyeCoord;
	float dist = sqrt(eyeCoord.x*eyeCoord.x + eyeCoord.y*eyeCoord.y + eyeCoord.z*eyeCoord.z);
	float att	 = 60.0 / dist;
	pos = gl_Vertex.xyz;
	gl_PointSize = 1.0/dist;//gl_Normal.x * att;
	gl_FrontColor = gl_Color;
}
