#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;

out vec4 fragColor;

uniform mat4 inverseViewMatrix;
uniform sampler2D diffuseTexture;

void main()
{
  // general idea of the fragment shader: for better 3d perception, we are darkening
  // the globe's texture a bit towards the edges of the ellipsoid. We use camera's
  // view direction and normal vector of the globe's geometry: the larger the angle
  // between them, the darker the color should be.

  vec3 cameraWorldPos = vec3(inverseViewMatrix[3]);
  vec3 viewDir = normalize(cameraWorldPos - worldPosition);

  // note: the constants below are just artistic choices to get decently looking shading

  // we apply shading only when we're far from the globe, so that closer
  // views do not get the globe's texture darkened (e.g. when looking towards horizon)
  float distFromCamera = length(cameraWorldPos - worldPosition);
  float shadingFactor = smoothstep(2e5, 5e5, distFromCamera);

  // dot product of the normal and the view direction is the cosine of the angle between them
  float diff = max(dot(worldNormal, viewDir), 0.0);

  fragColor = texture(diffuseTexture, texCoord) * (0.3 + 0.7 * mix(1.0, diff, shadingFactor));
}
