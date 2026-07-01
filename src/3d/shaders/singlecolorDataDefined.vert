#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;

in vec3 dataDefinedBaseColor;

out vec3 worldPosition;
out vec3 worldNormal;

out DataColor {
    vec3 base;
} vs_out;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main()
{
    worldNormal = normalize( modelNormalMatrix * vertexNormal );
    worldPosition = vec3( modelMatrix * vec4( vertexPosition, 1.0 ) );

    // colors defined data
    vs_out.base = dataDefinedBaseColor;

    gl_Position = mvp * vec4( vertexPosition, 1.0 );

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
