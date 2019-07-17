#version 150

// input
layout (points) in;
// output
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 modelViewProjection;

uniform vec2 BB_SIZE;    // billboard size in pixels
uniform vec2 WIN_SCALE;	 // the size of the viewport in pixels

out vec2 UV;
// glsl

void main (void)
{

  vec4 P = gl_in[0].gl_Position;
  P /= P.w;

  //vec2 size = vec2(0.5,0.5);
  vec2 size = BB_SIZE / WIN_SCALE;

  gl_Position = P;
  gl_Position.xy += vec2(-0.5,-0.5) * size;
  UV = vec2(0,0);
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(0.5,-0.5) * size;
  UV = vec2(1,0);
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(-0.5,+0.5) * size;
  UV = vec2(0,1);
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(+0.5,+0.5) * size;
  UV = vec2(1,1);
  EmitVertex();

  EndPrimitive();
}
