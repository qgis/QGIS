#version 150 core

in vec2 texCoord;

in vec3 worldPosition;

out vec4 fragColor;

uniform sampler2D diffuseTexture;

#pragma include shadows.inc.frag

void main()
{
    fragColor = texture( diffuseTexture, texCoord );

    if (renderShadows == 1)
    {
        float visibilityFactor = calcVisibilityAfterShadowing(worldPosition);
        fragColor = vec4(fragColor.rgb * mix(0.5, 1.0, visibilityFactor), fragColor.a);
    }
}
