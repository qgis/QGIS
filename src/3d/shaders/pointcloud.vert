#version 150

uniform mat4 modelViewProjection;

uniform mat4 projectionMatrix;
uniform mat4 viewportMatrix;
uniform mat4 modelMatrix;
uniform float u_pointSize;
// used parameter to choose point cloud points color: 0 for height, 1 for classID
uniform int u_renderingParameter;

uniform int u_renderingStyle;

in vec3 vertexPosition;
in float vertexParameter;
in vec3 vertexColor;
in vec3 vertexNormal; //used when points are triangulated

out float parameter;
flat out int classParameter;
out vec3 pointColor;
out vec3 worldPosition; //used when points are triangulated
out vec3 vertNorm; //used when points are triangulated

void main(void)
{
    gl_Position = modelViewProjection * vec4(vertexPosition, 1);

    gl_PointSize = u_pointSize;

    worldPosition = vec3 (modelMatrix * vec4 (vertexPosition,1));
    vertNorm = vertexNormal;

    switch (u_renderingStyle)
    {
    case 0: //  no rendering
    case 1: // single color
      break;
    case 2: // color ramp
      parameter = vertexParameter;
      break;
    case 3: // RGB
      pointColor = vertexColor;
      break;
    case 4: // classification
      classParameter = int(vertexParameter);
      break;
    }
}
