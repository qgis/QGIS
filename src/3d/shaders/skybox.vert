#version 330

in vec3 vertexPosition;
out vec3 texCoord0;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    texCoord0 = vertexPosition.xyz;
    // Converting the viewMatrix to a mat3, then back to a mat4
    // removes the translation component from it
    gl_Position = vec4(projectionMatrix * mat4(mat3(viewMatrix)) * modelMatrix * vec4(vertexPosition, 1.0)).xyww;
}
