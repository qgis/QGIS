#version 150

in vec2 vertexPosition;
in vec3 instancePosition;

// billboard size in pixels or world units (for perspective scaled billboards)
#ifdef PER_INSTANCE_SIZE
in vec2 instanceSize;
#else
uniform vec2 BB_SIZE;
#endif

#ifdef PERSPECTIVE_SCALE
uniform mat4 modelView;
uniform mat4 projectionMatrix;
#else
uniform mat4 modelViewProjection;
uniform vec2 WIN_SCALE;	 // the size of the viewport in pixels
#endif

#ifdef VERTICAL_OFFSET
uniform float VERT_OFFSET;
#endif

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
#ifdef PER_INSTANCE_SIZE
    vec2 bbSize = instanceSize;
#else
    vec2 bbSize = BB_SIZE;
#endif

    vec2 offsetPos = vertexPosition;
#ifdef VERTICAL_OFFSET
    offsetPos.y += VERT_OFFSET;
#endif

#ifdef PERSPECTIVE_SCALE
    // transform instance position into view space
    vec4 viewPos = modelView * vec4(instancePosition, 1.0);

    vec2 size = bbSize;

    // offset quad vertices directly in view space using world units
    viewPos.xy += offsetPos * size;

    gl_Position = projectionMatrix * viewPos;
#else
    vec2 spritePixelSize = 2 * bbSize / WIN_SCALE; // multiply by 2 to adjust for -1, 1 range for display coordinates

    #ifdef TEXTURE_ATLAS
      vec2 textureOffset = atlasOffset;
      vec2 textureSize = atlasSize;
      // scale sprite size by the texture size, so that billboard sizes are scaled by the relative size of their associated textures
      spritePixelSize *= textureSize;
    #endif

    vec4 P = modelViewProjection * vec4(instancePosition, 1);
    P /= P.w;
    P.xy += offsetPos * spritePixelSize;

    #ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
      // convert the pixel offset to display coordinates, multiplying by 2 to adjust for -1, 1 range for display coordinates
      P.xy += 2.0 * vec2(pixelOffset) / WIN_SCALE;
    #endif

    gl_Position = P;
#endif

#ifdef TEXTURE_ATLAS
  UV = textureOffset + (vertexPosition + vec2(0.5)) * textureSize;
#else
  UV = vertexPosition + vec2(0.5);
#endif
}
