#version 330

uniform sampler2D previewTexture;
uniform bool isDepth;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
  // Warning:
  // When trying to display a depth texture make sure to linearize the depth value
  // if you are using a perspective projection
  if (isDepth)
    fragColor = vec4(vec3(texture(previewTexture, texCoord).r), 1.0f);
  else
    fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0f);
}
