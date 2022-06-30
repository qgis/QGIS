#version 330

uniform sampler2D texture;

in vec2 texCoord;

out vec4 fragColor;

in vec2 v_texcoord;

const float sigmaS = 1.f;
const float sigmaL = 1.f;

const float EPS = 1e-5;

float lum(in vec4 color) {
    return length(color.xyz);
}

void main()
{
  float sigS = max(sigmaS, EPS);
  float sigL = max(sigmaL, EPS);

  float facS = -1./(2.*sigS*sigS);
  float facL = -1./(2.*sigL*sigL);

  float sumW = 0.;
  vec4  sumC = vec4(0.);
  float halfSize = sigS * 2;
  ivec2 textureSize2 = textureSize(texture, 0);

  float l = lum( texture2D( texture, texCoord ) );

  for (float i = -halfSize; i <= halfSize; i ++){
    for (float j = -halfSize; j <= halfSize; j ++){
      vec2 pos = vec2(i, j);
      vec4 offsetColor = texture2D(texture, texCoord + pos / textureSize2);

      float distS = length(pos);
      float distL = lum(offsetColor)-l;

      float wS = exp(facS*float(distS*distS));
      float wL = exp(facL*float(distL*distL));
      float w = wS*wL;

      sumW += w;
      sumC += offsetColor * w;
    }
  }

  fragColor = sumC/sumW;
}
