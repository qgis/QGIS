#version 150 core

in vec3 vertexPosition;

out vec3 position;
out vec2 texCoord;

uniform mat4 modelMatrix;

void main()
{
  texCoord = (vertexPosition.xy + vec2(1.0f, 1.0f)) / 2.0f;
  gl_Position = modelMatrix * vec4(vertexPosition.xy, 1.0f, 1.0f);
}
