#version 150 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

#ifdef INSTANCING
in vec3 instanceTranslation;
in vec4 instanceRotation;
in vec3 instanceScale;

vec3 rotateByQuat(vec3 v, vec4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}
#endif

out vec2 texCoord;
out vec3 worldPosition;

uniform mat4 mvp;
uniform mat4 modelMatrix;

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main()
{
#ifdef INSTANCING
    vec3 pos = vertexPosition * instanceScale;
    pos = rotateByQuat(pos, instanceRotation);
    pos += instanceTranslation;
#else
    vec3 pos = vertexPosition;
#endif

    // Pass through scaled texture coordinates
    texCoord = vertexTexCoord;

    gl_Position = mvp * vec4( pos, 1.0 );

    worldPosition = vec3(modelMatrix * vec4(pos, 1.0));

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
