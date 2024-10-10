#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

in vec3 dataDefinedDiffuseColor;
in vec3 dataDefinedWarmColor;
in vec3 dataDefinedCoolColor;
in vec3 dataDefinedSpecularColor;

out vec3 worldPosition;
out vec3 worldNormal;

out DataColor {
    vec3 diffuse;
    vec3 warm;
    vec3 cool;
    vec3 specular;
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
    vs_out.diffuse=dataDefinedDiffuseColor;
    vs_out.warm=dataDefinedWarmColor;
    vs_out.cool=dataDefinedCoolColor;
    vs_out.specular=dataDefinedSpecularColor;

    gl_Position = mvp * vec4( vertexPosition, 1.0 );

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
