#version 150

uniform mat4 modelViewProjection;

in vec3 vertexPosition;
#ifdef TEXTURE_ATLAS
in vec2 atlasOffset; // The top-left corner of the sprite in the atlas (normalized, 0-1)
in vec2 atlasSize;   // The size of the sprite in the atlas (normalized, 0-1)

out vec2 gsAtlasOffset;
out vec2 gsAtlasSize;
#endif
#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
in ivec2 pixelOffset;
out ivec2 gsPixelOffset;
#endif

void main(void)
{
#ifdef TEXTURE_ATLAS
    gsAtlasOffset = atlasOffset;
    gsAtlasSize = atlasSize;
#endif
#ifdef TEXTURE_ATLAS_PIXEL_OFFSETS
    gsPixelOffset = pixelOffset;
#endif
    gl_Position = modelViewProjection * vec4(vertexPosition, 1);
}
