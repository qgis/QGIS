#version 150 core

in vec3 vertexPosition;

out vec3 position;
out vec2 texCoord;

void main()
{
  texCoord = (vertexPosition.xy + vec2(1.0f, 1.0f)) / 2.0f;
  gl_Position = vec4(vertexPosition.xy, 1.0f, 1.0f);
}
