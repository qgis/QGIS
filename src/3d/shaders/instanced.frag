#version 150 core

// copy of phong.frag from qt3d extras

uniform vec3 ka;                            // Ambient reflectivity
uniform vec3 kd;                            // Diffuse reflectivity
uniform vec3 ks;                            // Specular reflectivity
uniform float shininess;                    // Specular shininess factor
uniform float opacity;                      // Opacity

uniform vec3 eyePosition;

in vec3 worldPosition;
in vec3 worldNormal;

out vec4 fragColor;

#pragma include phong.inc.frag

void main()
{
    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(
                    vec4(ka, opacity),
                    vec4(kd, opacity),
                    vec4(ks, opacity),
                    shininess,
                    worldPosition,
                    worldView,
                    worldNormal);
}
