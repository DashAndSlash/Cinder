//#extension GL_ARB_gpu_shader_fp64 : enable

uniform sampler2D	tex0;
//uniform sampler2D	tex1;

varying vec3 pos;

void main()
{

    vec4 color0 = texture2D( tex0, pos.yz );
    // do the same for the top image
//    vec4 color1 = texture2D( tex1, gl_TexCoord[0].st );
    
	gl_FragColor.rgb = vec3(1.0,1.0,1.0);//color0.rgb;//-mix(color0,color1,0.5).rgb;
	gl_FragColor.a = .5;
    //gl_FragColor.r=0.0;
}