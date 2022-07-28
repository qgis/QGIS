#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexTangent;
in vec2 vertexTexCoord;

in vec3 dataDefinedAmbiantColor;
in vec3 dataDefinedDiffuseColor;
in vec3 dataDefinedSpecularColor;

out vec3 worldPosition;
out vec3 worldNormal;

out DataColor {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} vs_out;


uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 modelViewProjection;

uniform float texCoordScale;
uniform float opacity;

void main()
{
    // Transform position, normal, and tangent to world space
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);


    // colors defined data
    vs_out.ambient = vec4(dataDefinedAmbiantColor, opacity);
    vs_out.diffuse = vec4(dataDefinedDiffuseColor, opacity);
    vs_out.specular = vec4(dataDefinedSpecularColor, opacity);

    // Calculate vertex position in clip coordinates
    gl_Position = modelViewProjection * vec4(vertexPosition, 1.0);
}
