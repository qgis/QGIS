#version 330

in vec3 texCoord0;
out vec4 fragColor;
uniform samplerCube skyboxTexture;

void main()
{
    fragColor = texture(skyboxTexture, texCoord0);
}
