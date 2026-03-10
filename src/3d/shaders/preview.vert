#version 330

in vec3 vertexPosition;

out vec2 texCoord;

uniform mat4 modelMatrix;

uniform vec2 sizeTexCoords;
uniform vec2 centerTexCoords;
uniform bool flipTextureY;

void main()
{
  float texCoordY = flipTextureY ? 1 - (vertexPosition.y + 1) / 2 : (vertexPosition.y + 1) / 2;
  texCoord = vec2( (vertexPosition.x + 1) / 2, texCoordY );

  vec2 vertexTexCoord = centerTexCoords + vec2( vertexPosition.x * sizeTexCoords.x, vertexPosition.y * sizeTexCoords.y );
  gl_Position = vec4( 2 * vertexTexCoord.x - 1, 1 - 2 * vertexTexCoord.y, 1.0f, 1.0f);
}
