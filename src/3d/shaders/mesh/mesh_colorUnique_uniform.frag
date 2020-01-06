#version 330 core

uniform vec3 uniqueColor;

out vec4 fragColor;

void main()
{
        fragColor=vec4(uniqueColor,1.0);
}
