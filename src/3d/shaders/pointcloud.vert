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
in float vertexSize; //contains overridden pointSize for classification rendering

out float parameter;
flat out int classParameter;
out vec3 pointColor;
out vec3 worldPosition; //used when points are triangulated
out vec3 vertNorm; //used when points are triangulated

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main(void)
{
    gl_Position = modelViewProjection * vec4(vertexPosition, 1);

    worldPosition = vec3 (modelMatrix * vec4 (vertexPosition,1));
    vertNorm = vertexNormal;

    switch (u_renderingStyle)
    {
    case 0: //  no rendering
    case 1: // single color
      gl_PointSize = u_pointSize;
      break;
    case 2: // color ramp
      gl_PointSize = u_pointSize;
      parameter = vertexParameter;
      break;
    case 3: // RGB
      gl_PointSize = u_pointSize;
      pointColor = vertexColor;
      break;
    case 4: // classification
      gl_PointSize = vertexSize;
      classParameter = int(vertexParameter);
      break;
    }

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
