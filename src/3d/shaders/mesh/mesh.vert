#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

out MeshVertex {
    vec3 position;
    vec3 wordPosition;
    vec3 wordNormal;
    float magnitude;
} vs_out;


uniform mat4 modelView;
uniform mat4 mvp;
uniform mat4 modelMatrix;

void main()
{
    vs_out.position = vec3( modelView * vec4( vertexPosition, 1.0 ) );
    gl_Position = mvp * vec4( vertexPosition, 1.0 );

    vec3 wordPosition=vec3(modelMatrix*vec4(vertexPosition,1));
    vs_out.wordPosition=wordPosition;
    vs_out.magnitude=vertexPosition.y;
    vs_out.wordNormal=vertexNormal;
}
