#version 330

in vec3 vertexPosition;
out float vHeight;

void main()
{
    gl_Position = vec4( vertexPosition.x, vertexPosition.y, 1.0, 1.0 );
    vHeight = gl_Position.y;
}