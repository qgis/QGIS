#version 330 core

out vec4 fragColor;

uniform vec3 color;
uniform float opacity;

void main(void)
{
  fragColor = vec4(color, opacity);
}
