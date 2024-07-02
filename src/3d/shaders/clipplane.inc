const int MAX_PLANE = 4;

uniform vec4 clipPlane[MAX_PLANE];

out float gl_ClipDistance[MAX_PLANE];

void setClipDistance(vec3 worldPos)
{
  for (int i = 0; i < MAX_PLANE; ++i)
    {
      gl_ClipDistance[i] = dot(vec4(worldPos, 1.0), clipPlane[i]);
    }
}
