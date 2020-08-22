#version 330

in vec3 texCoord0;
out vec4 fragColor;
uniform sampler2D skyboxTexture;

// Gamma correction
uniform float gamma = 2.2;

uniform float gammaStrength;

vec3 gammaCorrect(const in vec3 color)
{
    return pow(color, vec3(1.0 / gamma));
}

const vec2 invAtan = vec2(0.1591f, 0.3183f);
vec2 SampleSphericalMap(vec3 v)
{
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main()
{
  vec2 texCoords = SampleSphericalMap( normalize( texCoord0 ) );
  vec4 baseColor = texture(skyboxTexture, texCoords);
  vec4 gammaColor = vec4(gammaCorrect(baseColor.rgb), 1.0);
  // This is an odd way to enable or not gamma correction,
  // but this is a way to avoid branching until we can generate shaders
  fragColor = mix(baseColor, gammaColor, gammaStrength);
}
