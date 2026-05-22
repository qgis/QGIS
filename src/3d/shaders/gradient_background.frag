#version 330

in float vHeight;
out vec4 fragColor;

uniform vec3 topColor;
uniform vec3 bottomColor;

void main()
{
    float t = vHeight * 0.5 + 0.5;

    // convert colors to srgb, for better looking gradient ramp
    vec3 topColorSrgb = pow(topColor, vec3(1.0 / 2.2));
    vec3 bottomColorSrgb = pow(bottomColor, vec3(1.0 / 2.2));

    vec3 mixedColorSrgb = mix( bottomColorSrgb, topColorSrgb, t );

    // convert back to linear for output color
    fragColor = vec4(pow(mixedColorSrgb, vec3(2.2)), 1.0);
}
