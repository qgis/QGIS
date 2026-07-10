#version 330
uniform sampler2D srcTexture;
uniform float filterRadius;
uniform float aspectRatio;
in vec2 texCoord;
out vec4 fragColor;

void main() {
    float x = filterRadius;
    // as per https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom#comment-6311220794
    float y = filterRadius * aspectRatio;

    // 9-Tap tent filter (following CoD: Advanced Warfare approach, from ACM Siggraph 2014)
    // see also https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(texCoord.x,     texCoord.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(texCoord.x - x, texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(texCoord.x,     texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(texCoord.x + x, texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(texCoord.x,     texCoord.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // From https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    // "Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |" (Jorge Jimenez)
    vec3 bloom = e * 4.0;
    bloom += (b + d + f + h) * 2.0;
    bloom += (a + c + g + i);
    bloom *= (1.0 / 16.0);

    fragColor = vec4(bloom, 1.0);
}
