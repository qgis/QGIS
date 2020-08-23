#version 150 core

uniform sampler2D previewTexture;
uniform bool isDepth;

in vec3 position;
in vec2 texCoord;

out vec4 fragColor;

const float near_plane = 0.1f;
const float far_plane = 100.0f;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
//  fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0f);
  float depthValue = texture(previewTexture, texCoord).r;
//  depthValue = depthValue * 30.0f;
//  if (isDepth) {
//    fragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
  fragColor = vec4(vec3(depthValue), 1.0);
//  } else {
//    fragColor = vec4(vec3(((depthValue + 15.0f) * 30.0f)), 1.0f);
//fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0f);
//    fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);//vec4(vec3(depthValue), 1.0f);
//  }
}
