// Fragment shader

uniform sampler2D texUnit;
void main(void)
{
    vec4 texVal  = texture2D(texUnit, gl_TexCoord[0].xy);
    gl_FragColor = sqrt(texVal);
}
