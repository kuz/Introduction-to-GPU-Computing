// This fragment shader does NOT perform sorting (yet) but only demonstrates
// simple re-arrangements of data, reverts data order in groups of 8

uniform sampler2D texUnit;
void main(void)
{
   const float texsize = 4.0; // 4 pixels high and wide
   const float offset = 1.0/texsize; // one step in texture
   
   float x = 1.0 - (mod(gl_TexCoord[0].x * texsize, 2.0) - 0.5) * 2.0; // -1 or 1
   vec4 texVal  = texture2D(texUnit, gl_TexCoord[0].xy + vec2(x*offset, 0.0));
   
   gl_FragColor = texVal.abgr; // swizzling
}
