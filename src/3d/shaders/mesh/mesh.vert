#version 330 core

uniform mat4 modelView;
uniform mat4 mvp;
uniform mat4 modelMatrix;

uniform bool isScalarMagnitude;

in vec3 vertexPosition;
in vec3 vertexNormal;
in float scalarMagnitude;

out vec3 worldPosition;
out vec3 worldNormal;
out float magnitude;
out vec3 barycentric;

const vec3 BARYCENTRIC[3] = vec3[3]( vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) );

#ifdef CLIPPING
    #pragma include ../clipplane.shaderinc
#endif

void main()
{
    gl_Position = mvp * vec4( vertexPosition, 1.0 );

    worldPosition = vec3(modelMatrix*vec4(vertexPosition,1));
    worldNormal = vertexNormal;
    barycentric = BARYCENTRIC[gl_VertexID % 3];

    if ( isScalarMagnitude )
        magnitude = scalarMagnitude;
    else
        magnitude = worldPosition.z;

#ifdef CLIPPING
    setClipDistance( worldPosition );
#endif
}
