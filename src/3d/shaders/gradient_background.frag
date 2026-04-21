#version 330

in float vHeight;
out vec4 fragColor;

uniform vec4 topColor;
uniform vec4 bottomColor;

void main()
{
    float t = vHeight * 0.5 + 0.5;
    fragColor = mix( bottomColor, topColor, t );
}
