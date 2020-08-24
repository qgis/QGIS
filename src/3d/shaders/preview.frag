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
  float depthValue = texture(previewTexture, texCoord).r;
//  fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0);
  fragColor = vec4(vec3(depthValue), 1.0f);
}
