#version 150 core

uniform vec4 ka;            // Ambient reflectivity
uniform vec4 ks;            // Specular reflectivity
uniform float shininess;    // Specular shininess factor

uniform vec3 eyePosition;

#ifdef DIFFUSE_TEXTURE
uniform sampler2D diffuseTexture;
#else
uniform vec4 diffuse;
#endif

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;

out vec4 fragColor;

#pragma include phong.inc.frag

void main()
{
    #ifdef DIFFUSE_TEXTURE
        vec4 diffuseTextureColor = texture( diffuseTexture, texCoord );
    #else
        vec4 diffuseTextureColor = diffuse;
    #endif

    if ( diffuseTextureColor.a < 1.0 )
      discard;

    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(ka, diffuseTextureColor, ks, shininess, worldPosition, worldView, worldNormal);
}
