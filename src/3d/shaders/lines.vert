#version 150

uniform mat4 modelViewProjection;

in vec3 vertexPosition;
//in vec3 ciColor;

//out VertexData{
//	vec3 mColor;
//} VertexOut;


void main(void)
{
    //VertexOut.mColor = ciColor;
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}
