#version 330

in vec3 texCoord0;
out vec4 fragColor;
uniform samplerCube skyboxTexture;

// Gamma correction
uniform float gamma = 2.2;

uniform float gammaStrength;

vec3 gammaCorrect(const in vec3 color)
{
    return pow(color, vec3(1.0 / gamma));
}

void main()
{
    vec4 baseColor = texture(skyboxTexture, texCoord0);
    vec4 gammaColor = vec4(gammaCorrect(baseColor.rgb), 1.0);
    // This is an odd way to enable or not gamma correction,
    // but this is a way to avoid branching until we can generate shaders
    fragColor = mix(baseColor, gammaColor, gammaStrength);
}
