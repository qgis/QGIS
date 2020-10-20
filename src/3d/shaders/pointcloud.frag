#version 150

in float clsid;

out vec4 color;

void main(void)
{
  if ( abs(clsid-2) < 0.1 )         // ground
      color = vec4(1,1,0,1);
  else if ( abs( clsid - 3 ) < 0.1 )      // low vegetation
      color = vec4(0,0.4,0,1);
  else if ( abs( clsid - 4 ) < 0.1 )      // medium vegetation
      color = vec4(0,0.6,0,1);
  else if ( abs( clsid - 5 ) < 0.1 )     // high vegetation
      color = vec4(0,1,0,1);
  else if ( abs( clsid - 12 ) < 0.1 )   // overlaps
  {
      color = vec4(1,0,0,1);
      //discard;
  }
  else
  {
      color = vec4(0,1,1,1);
      //discard;
  }
}
