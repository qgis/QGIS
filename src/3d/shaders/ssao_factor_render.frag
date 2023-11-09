#version 330

uniform sampler2D depthTexture;

uniform float farPlane;
uniform float nearPlane;
uniform mat4 origProjMatrix;      // Perspective projection matrix used for forward rendering

uniform vec3 ssaoKernel[64];  // Random sample vectors in a unit sphere
const int kernelSize = 64;
uniform vec3 ssaoNoise[16];  // 4x4 random noise pattern

uniform float	intensity;  // Amplification of shading
uniform float	radius;     // Radius of neighborhood sphere (in world units)
uniform float   threshold;  // At what amount of occlusion to start darkening

noperspective in vec3 vViewRay;   // Ray to far plane

out vec4 fragColor;


float linearizeDepth(float depth)
{
  float ndc = depth * 2.0 - 1.0;
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
}

vec3 rotate_x(vec3 vct, float angle)
{
  return vec3(vct.x*cos(angle)-vct.y*sin(angle), vct.x*sin(angle)+vct.y*cos(angle), vct.z);
}
vec3 rotate_y(vec3 vct, float angle)
{
  return vec3(vct.x*cos(angle)+vct.z*sin(angle), vct.y, -vct.x*sin(angle)+vct.z*cos(angle));
}

float ssao(vec3 originPos, vec3 noise)
{
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        //	get sample position:
        vec3 samplePos = rotate_y(rotate_x(ssaoKernel[i], noise.x), noise.y);
        samplePos = samplePos * radius + originPos;

        //	project sample position:
        vec4 offset = origProjMatrix * vec4(samplePos, 1.0);
        offset.xy /= offset.w;               // only need xy   (range [-1,1])
        offset.xy = offset.xy * 0.5 + 0.5;   // scale/bias to texcoords  (range [0,1])

        //	get sample depth:
        float sampleDepth = texture(depthTexture, offset.xy).r;
        sampleDepth = linearizeDepth(sampleDepth);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(originPos.z - sampleDepth));
        occlusion += rangeCheck * step(sampleDepth, samplePos.z);
    }

    return min(1.0, (1.0 - (occlusion / float(kernelSize))) / (1.0 - threshold));
}


void main()
{
    // calculate view-space 3D coordinates of this pixel
    vec2 texelSize = 1.0 / vec2(textureSize(depthTexture, 0));
    vec2 screenTexCoords = gl_FragCoord.xy * texelSize;
    float originDepth = linearizeDepth(texture(depthTexture, screenTexCoords).r);
    vec3 originPos = vViewRay * originDepth;

    int a_idx = int(gl_FragCoord.x) % 4 + 4 * (int(gl_FragCoord.y) % 4);
    vec3 noise = ssaoNoise[a_idx] * 2*3.14;

    float ssao_res = ssao(originPos, noise);

    fragColor = vec4(pow(ssao_res,intensity));
}
