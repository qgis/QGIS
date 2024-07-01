#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec3 worldPosition;
out vec3 worldNormal;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

// uniform float texCoordScale;

void main()
{
    // Pass through scaled texture coordinates
    texCoord = vertexTexCoord;

    // Transform position, normal, and tangent to world space
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);

    gl_Position = mvp * vec4( vertexPosition, 1.0 );
}
