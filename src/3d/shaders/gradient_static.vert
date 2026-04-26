#version 330

in vec3 vertexPosition;
out float vHeight;

void main()
{
    gl_Position = vec4( vertexPosition.x * 2.0, vertexPosition.y * 2.0, 0.9999, 1.0 );
    vHeight = gl_Position.y;
}