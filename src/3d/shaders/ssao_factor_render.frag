#version 330

uniform sampler2D depthTexture;

uniform float farPlane;
uniform float nearPlane;

uniform vec3 ssaoKernel[64];
const int kernelSize = 32;

uniform float	shadingFactor; // Amplification of shading
uniform float	distanceAttenuationFactor; // distance attenuation factor
uniform float	radiusParameter; // Radius of neighborhood sphere

in vec2 texCoord;

out vec4 fragColor;

float linearizeDepth(float depth)
{
  float ndc = depth * 2.0 - 1.0;
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
}

vec3 computeSSAO()
{
  float	z	= linearizeDepth(	texture2D( depthTexture, texCoord ).x ) / farPlane;
  vec3 fragPos = vec3( texCoord, z );

  float zs, dz;
  vec3 samplePos;
  float occlusion = 0.0;
  for(int i = 0; i < kernelSize; ++i)
  {
    vec3 samplePos = fragPos + ssaoKernel[i] * radiusParameter;
    zs      = linearizeDepth(	texture2D( depthTexture, samplePos.xy ).r ) / farPlane;
    dz      =	max( 0.0, min( samplePos.z, 1.0 ) - zs );
    occlusion    +=	dz * shadingFactor * 1.0 / ( 1.0 + distanceAttenuationFactor * dz * dz ) / float(kernelSize);
  }
  return vec3( max(1.0 - occlusion ,0.0) );
}

void main()
{
  fragColor = vec4( computeSSAO(), fragColor.a );
}
