#include "calcfirstder.cl"

__constant sampler_t sampler =
      CLK_NORMALIZED_COORDS_FALSE
    | CLK_ADDRESS_CLAMP_TO_EDGE
    | CLK_FILTER_NEAREST;

__kernel void processNineCellWindow(
                                     __read_only image2d_t inputImage,
                                     image2d_t outputImage,
                                     __global float *rasterParams
                       ) {


  // 0=width, 1=height
  int2 currentPosition = (int2)(get_global_id(0), get_global_id(1));
  float4 currentPixel = (ufloat4)(0.0f);
  float4 calculatedPixel = (float4)(1.0f);

  currentPixel = read_imageuf(inputImage, sampler, currentPosition);
  calculatedPixel = currentPixel;
  write_imageuf(outputImage, currentPosition, calculatedPixel);
}


const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

void kernel copy(__read_only image2d_t in, image2d_t out)
{
  int x = get_global_id(0);
  int y = get_global_id(1);
  int2 pos = (int2)(x, y);
  uint4 pixel = read_imageui(in, smp, pos);
  write_imageui(out, pos, pixel);
}
