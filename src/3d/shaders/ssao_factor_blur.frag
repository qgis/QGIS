#version 330

uniform sampler2D texture;

in vec2 texCoord;

out float fragColor;

void main()
{
    // simple 4x4 box blur which smoothes the 4x4 noise pattern
    float result = 0.0;
    ivec2 minCoord = ivec2(0);
    ivec2 maxCoord = textureSize(texture, 0) - ivec2(1);
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            ivec2 fetchCoord = ivec2(gl_FragCoord) + ivec2(x, y);

            // clamp to valid bounds, otherwise we get edge effects (darkened borders around scene)
            fetchCoord = clamp(fetchCoord, minCoord, maxCoord);

            result += texelFetch(texture, fetchCoord, 0).r;
        }
    }
    fragColor = result / (4.0 * 4.0);
}
