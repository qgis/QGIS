#version 150

uniform sampler2D tex0;

in vec2 UV;

void main(void)
{
  //gl_FragColor = vec4(0.5,0.1,0.1,1);
  gl_FragColor = texture(tex0, UV);
}
