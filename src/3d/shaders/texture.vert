#version 150 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;

uniform mat4 mvp;

#ifdef CLIPPING
    uniform mat4 modelMatrix;
    #pragma include clipplane.shaderinc
#endif

void main()
{
    // Pass through scaled texture coordinates
    texCoord = vertexTexCoord;

    gl_Position = mvp * vec4( vertexPosition, 1.0 );

#ifdef CLIPPING
    vec3 worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    setClipDistance(worldPosition);
#endif
}
