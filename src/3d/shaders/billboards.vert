#version 150

uniform mat4 modelViewProjection;

in vec3 vertexPosition;

void main(void)
{
    gl_Position = modelViewProjection * vec4(vertexPosition, 1);
}
