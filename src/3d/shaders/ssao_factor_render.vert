#version 150 core

in vec3 vertexPosition;

out vec2 texCoord;

noperspective out vec3 vViewRay; // ray to far plane

//	view frustum parameters:
uniform float uTanHalfFov;
uniform float uAspectRatio;

void main()
{
  gl_Position = vec4(vertexPosition, 1.0f);
  texCoord = (vertexPosition.xy + vec2(1.0f)) / 2.0f;

  vViewRay = vec3(
        -vertexPosition.x * uTanHalfFov * uAspectRatio,
        -vertexPosition.y * uTanHalfFov,
        1.0 // since we'll be multiplying by linear depth, leave z as 1
    );
}
