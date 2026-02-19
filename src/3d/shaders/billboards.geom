#version 150

// input
layout (points) in;
// output
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 modelViewProjection;

uniform vec2 BB_SIZE;    // billboard size in pixels
uniform vec2 WIN_SCALE;	 // the size of the viewport in pixels

#ifdef TEXTURE_ATLAS
in vec2 gsAtlasOffset[];
in vec2 gsAtlasSize[];
#endif

#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
in ivec2 gsPixelOffset[];
#endif

out vec2 UV;
// glsl

void main (void)
{

  vec4 P = gl_in[0].gl_Position;
  P /= P.w;

  vec2 spritePixelSize = 2 * BB_SIZE / WIN_SCALE; // multiply by 2 to adjust for -1, 1 range for display coordinates

#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
  // convert the pixel offset to display coordinates, multiplying by 2 to adjust for -1, 1 range for display coordinates
  P.xy += 2.0 * gsPixelOffset[0] / WIN_SCALE;
#endif

#ifdef TEXTURE_ATLAS
  vec2 textureOffset = gsAtlasOffset[0];
  vec2 textureSize = gsAtlasSize[0];
  // scale sprite size by the texture size, so that billboard sizes are scaled by the relative size of their associated textures
  spritePixelSize *= textureSize;
#endif

  gl_Position = P;
  gl_Position.xy += vec2(-0.5,-0.5) * spritePixelSize;
#ifdef TEXTURE_ATLAS
  UV = textureOffset;
#else
  UV = vec2(0,0);
#endif
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(0.5,-0.5) * spritePixelSize;
#ifdef TEXTURE_ATLAS
  UV = textureOffset + vec2(textureSize.x,0);
#else
  UV = vec2(1,0);
#endif
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(-0.5,+0.5) * spritePixelSize;
#ifdef TEXTURE_ATLAS
  UV = textureOffset + vec2(0, textureSize.y);
#else
  UV = vec2(0,1);
#endif
  EmitVertex();

  gl_Position = P;
  gl_Position.xy += vec2(+0.5,+0.5) * spritePixelSize;
#ifdef TEXTURE_ATLAS
  UV = textureOffset + textureSize;
#else
  UV = vec2(1,1);
#endif
  EmitVertex();

  EndPrimitive();
}
