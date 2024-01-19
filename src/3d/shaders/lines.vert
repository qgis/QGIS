#version 150

uniform mat4 modelViewProjection;
uniform bool useVertexColors;

in vec3 vertexPosition;
in vec3 dataDefinedColor;

out DataColor {
    vec3 mColor;
} vs_out;

void main(void)
{
    if ( useVertexColors )
    {
      vs_out.mColor=dataDefinedColor;
    }

    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}
