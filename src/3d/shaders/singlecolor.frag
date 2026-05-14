#version 330 core

out vec4 fragColor;

uniform vec4 color;

void main(void)
{
  fragColor = color;
}
