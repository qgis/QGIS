#version 150

uniform mat4 modelViewProjection;

uniform mat4 projectionMatrix;
uniform mat4 viewportMatrix;
uniform mat4 modelMatrix;
uniform float u_pointSize;
// used parameter to choose point cloud points color: 0 for height, 1 for classID
uniform int u_renderingParameter;

in vec3 vertexPosition;
in float vertexParameter;
in vec3 vertexColor;
in vec3 vertexNormal;

out float parameter;
out vec3 pointColor;
out vec3 worldPosition;
out vec3 vertNorm;

void main(void)
{
    //if (abs(cls-5) < 0.1)
    //    gl_Position = vec4(0,0,0,0);
    //else
        gl_Position = modelViewProjection * vec4(vertexPosition, 1);

    gl_PointSize = u_pointSize; //5 + vertexPosition.x * 10 + vertexPosition.y * 10;
    //gl_PointSize = viewportMatrix[1][1] * projectionMatrix[1][1] * 1.0 / gl_Position.w;
    //gl_PointSize = 100.0;

    worldPosition = vec3 (modelMatrix * vec4 (vertexPosition,1));
    vertNorm = vertexNormal;
    parameter = vertexParameter;
    pointColor = vertexColor;
}
