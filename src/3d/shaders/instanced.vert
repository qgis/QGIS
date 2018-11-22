#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 pos;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 modelViewProjection;

uniform mat4 inst;  // transform of individual object instance
uniform mat4 instNormal;  // should be mat3 but Qt3D only supports mat4...

void main()
{
    // TODO: i think this is not entirely correct: the translation by "pos" works
    // like this only because we assume that "inst" matrix only does translation/scale/rotation
    // which all keep "w" set to 1. correctly we should use translation matrix...
    vec4 offsetPos = inst * vec4(vertexPosition, 1.0) + vec4(pos, 0.0);

    worldNormal = normalize(modelViewNormal * mat3(instNormal) * vertexNormal);
    worldPosition = vec3(modelView * offsetPos);

    gl_Position = modelViewProjection * offsetPos;
}
