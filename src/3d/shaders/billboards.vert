#version 150

uniform mat4 modelViewProjection;

in vec2 vertexPosition;
in vec3 instancePosition;

uniform vec2 BB_SIZE;    // billboard size in pixels
uniform vec2 WIN_SCALE;	 // the size of the viewport in pixels

out vec2 UV;

#ifdef TEXTURE_ATLAS
in vec2 atlasOffset; // The top-left corner of the sprite in the atlas (normalized, 0-1)
in vec2 atlasSize;   // The size of the sprite in the atlas (normalized, 0-1)
#endif

#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
in ivec2 pixelOffset;
#endif

void main(void)
{
  vec2 spritePixelSize = 2 * BB_SIZE / WIN_SCALE; // multiply by 2 to adjust for -1, 1 range for display coordinates

#ifdef TEXTURE_ATLAS
  vec2 textureOffset = atlasOffset;
  vec2 textureSize = atlasSize;
  // scale sprite size by the texture size, so that billboard sizes are scaled by the relative size of their associated textures
  spritePixelSize *= textureSize;
#endif

  vec4 P = modelViewProjection * vec4(instancePosition, 1);
  P /= P.w;
  P.xy += vertexPosition * spritePixelSize;

#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
  // convert the pixel offset to display coordinates, multiplying by 2 to adjust for -1, 1 range for display coordinates
  P.xy += 2.0 * vec2(pixelOffset) / WIN_SCALE;
#endif

  gl_Position = P;

#ifdef TEXTURE_ATLAS
  UV = textureOffset + (vertexPosition + vec2(0.5)) * textureSize;
#else
  UV = vertexPosition + vec2(0.5);
#endif
}
