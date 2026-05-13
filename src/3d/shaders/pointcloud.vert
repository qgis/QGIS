#version 150

uniform mat4 modelViewProjection;

uniform mat4 projectionMatrix;
uniform mat4 viewportMatrix;
uniform mat4 modelMatrix;

#if defined(STYLE_COLOR_RAMP) || defined(STYLE_SINGLE_COLOR) || defined(STYLE_RGB) || defined(STYLE_NO_RENDERING)
uniform float u_pointSize;
#endif

in vec3 vertexPosition;

#ifdef STYLE_COLOR_RAMP
in float vertexParameter;
out float parameter;
#endif

#ifdef STYLE_RGB
in vec3 vertexColor;
out vec3 pointColor;
#endif

#ifdef TRIANGULATE
in vec3 vertexNormal; //used when points are triangulated
out vec3 vertNorm; //used when points are triangulated
#endif

#ifdef STYLE_CLASSIFICATION
in float vertexParameter;
in float vertexSize; //contains overridden pointSize for classification rendering
flat out int classParameter;
#endif

out vec3 worldPosition;

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main(void)
{
    gl_Position = modelViewProjection * vec4(vertexPosition, 1);

    worldPosition = vec3 (modelMatrix * vec4 (vertexPosition,1));

#ifdef TRIANGULATE
    vertNorm = vertexNormal;
#endif

#if defined(STYLE_COLOR_RAMP) || defined(STYLE_SINGLE_COLOR) || defined(STYLE_RGB) || defined(STYLE_NO_RENDERING)
    gl_PointSize = u_pointSize;
#endif

#ifdef STYLE_COLOR_RAMP
    parameter = vertexParameter;
#endif
#ifdef STYLE_RGB
    pointColor = vertexColor;
#endif
#ifdef STYLE_CLASSIFICATION
    gl_PointSize = vertexSize;
    classParameter = int(vertexParameter);
#endif

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
