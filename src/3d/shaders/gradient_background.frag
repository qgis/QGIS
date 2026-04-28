#version 330

in float vHeight;
out vec4 fragColor;

uniform vec3 topColor;
uniform vec3 bottomColor;

void main()
{
    float t = vHeight * 0.5 + 0.5;
    fragColor = vec4( mix( bottomColor, topColor, t ), 1.0 );
}
