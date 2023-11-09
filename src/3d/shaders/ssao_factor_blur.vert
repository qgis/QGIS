#version 150 core

in vec3 vertexPosition;

out vec2 texCoord;

void main()
{
  gl_Position = vec4(vertexPosition, 1.0f);
  texCoord = (vertexPosition.xy + vec2(1.0f)) / 2.0f;
}
