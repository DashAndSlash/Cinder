uniform sampler2D      tex;

void main(void)
{
    gl_FragColor.rgb = texture2D( tex, gl_TexCoord[0].st ).rgb*3.0;
    gl_FragColor.a =  1.00;
}