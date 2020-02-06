#version 330 core

uniform mat4 modelView;
uniform mat4 mvp;
uniform mat4 modelMatrix;

uniform float verticaleScale;

in vec3 vertexPosition;
in vec3 vertexNormal;

out MeshVertex {
    vec3 worldPosition;
    vec3 worldNormal;
    float magnitude;
} vs_out;

void main()
{
    gl_Position = mvp * vec4( vertexPosition, 1.0 );

    vec3 worldPosition=vec3(modelMatrix*vec4(vertexPosition,1));
    vs_out.worldPosition=worldPosition;
    vs_out.worldNormal=vertexNormal;
    vs_out.magnitude=worldPosition.y;
}
