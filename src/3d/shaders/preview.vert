#version 150 core
//#version 450 core

//in vec3 vertexPosition;

//out vec2 texCoords;

//void main()
//{
//    gl_Position = vec4(vertexPosition, 1.0);
//    texCoords = vertexPosition.xy;
//}


in vec3 vertexPosition;
//in vec2 vertexTexCoord;

out vec3 position;
out vec2 texCoord;

//uniform mat4 modelView;
//uniform mat4 mvp;
//uniform mat3 texCoordTransform;

void main()
{
//    vec3 tt = texCoordTransform * vec3(vertexTexCoord, 1.0);
//    texCoord = (tt / tt.z).xy;
//    position = vec3( modelView * vec4( vertexPosition, 1.0 ) );
  texCoord = (vertexPosition.xy + vec2(1.0f, 1.0f)) / 2.0f;
  gl_Position = vec4(vertexPosition.xy, -1.0f, 1.0f);//mvp * vec4( vertexPosition, 1.0 );
}
