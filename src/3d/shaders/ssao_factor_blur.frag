#version 330

uniform sampler2D texture;

in vec2 texCoord;

out float fragColor;

void main()
{
    // simple 4x4 box blur which smoothes the 4x4 noise pattern
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            result += texelFetch(texture, ivec2(gl_FragCoord) + ivec2(x,y), 0).r;
        }
    }
    fragColor = result / (4.0 * 4.0);
}
