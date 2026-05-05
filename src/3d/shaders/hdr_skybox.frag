#version 330

in vec3 texCoord0;
out vec4 fragColor;
uniform sampler2D skyboxTexture;

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
  fragColor = texture(skyboxTexture, texCoords);
}
