__kernel void processNineCellWindow( __global float *scanLine1,
                                     __global float *scanLine2,
                                     __global float *scanLine3,
                                     __global float *resultLine,
                                     __global float *rasterParams )
{

  // Get the index of the current element
  const int i = get_global_id(0);

  float x11 = scanLine1[i];
  float x21 = scanLine1[i+1];
  float x31 = scanLine1[i+2];
  float x12 = scanLine2[i];
  float x22 = scanLine2[i+1];
  float x32 = scanLine2[i+2];
  float x13 = scanLine3[i];
  float x23 = scanLine3[i+1];
  float x33 = scanLine3[i+2];

  if ( x22 == rasterParams[0] )
  {
     resultLine[i] = rasterParams[1];
  }
  else
  {
    // Nodata handling
    if ( x11 == rasterParams[0] ) x11 = x22;
    if ( x12 == rasterParams[0] ) x12 = x22;
    if ( x13 == rasterParams[0] ) x13 = x22;
    if ( x21 == rasterParams[0] ) x21 = x22;
    if ( x23 == rasterParams[0] ) x23 = x22;
    if ( x31 == rasterParams[0] ) x31 = x22;
    if ( x32 == rasterParams[0] ) x32 = x22;
    if ( x33 == rasterParams[0] ) x33 = x22;

    float diff1 = x11 - x22;
    float diff2 = x21 - x22;
    float diff3 = x31 - x22;
    float diff4 = x12 - x22;
    float diff5 = x32 - x22;
    float diff6 = x13 - x22;
    float diff7 = x23 - x22;
    float diff8 = x33 - x22;

    resultLine[i] = sqrt(diff1 * diff1 + diff2 * diff2 +
                         diff3 * diff3 + diff4 * diff4 +
                         diff5 * diff5 + diff6 * diff6 +
                         diff7 * diff7 + diff8 * diff8);
  }
}
