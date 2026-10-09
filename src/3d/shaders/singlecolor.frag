#version 330 core

out vec4 fragColor;

#ifdef DATA_DEFINED
in DataColor {
    vec3 base;
} vs_in;
#else
uniform vec4 color;
#endif

void main(void)
{
#ifdef DATA_DEFINED
  fragColor = vec4(vs_in.base, 1.0);
#else
  fragColor = color;
#endif
}
