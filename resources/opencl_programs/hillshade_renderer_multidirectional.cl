__kernel void processNineCellWindow( __global float *scanLine1,
                                       __global float *scanLine2,
                                       __global float *scanLine3,
                                       __global uchar4 *resultLine, // This is an image!
                                       __global float *rasterParams //  [ mInputNodataValue, mOutputNodataValue, mZFactor, mCellSizeX, mCellSizeY,
                                                                    //    cos_az_mul_cos_alt_mul_z_mul_254, sin_az_mul_cos_alt_mul_z_mul_254, square_z, sin_altRadians_mul_254]

                         ) {

    // Get the index of the current element
    const int i = get_global_id(0);

    float x11 = scanLine1[i];
    float x12 = scanLine1[i+1];
    float x13 = scanLine1[i+2];
    float x21 = scanLine2[i];
    float x22 = scanLine2[i+1];
    float x23 = scanLine2[i+2];
    float x31 = scanLine3[i];
    float x32 = scanLine3[i+1];
    float x33 = scanLine3[i+2];

    float res;
    if ( x22 == rasterParams[1] )
    {
       res = rasterParams[2];
    }
    else
    {
    if ( x11 == rasterParams[0] ) x11 = x22;
    if ( x12 == rasterParams[0] ) x12 = x22;
    if ( x13 == rasterParams[0] ) x13 = x22;
    if ( x21 == rasterParams[0] ) x21 = x22;
    // Note: skip central cell x22
    if ( x23 == rasterParams[0] ) x23 = x22;
    if ( x31 == rasterParams[0] ) x31 = x22;
    if ( x32 == rasterParams[0] ) x32 = x22;
    if ( x33 == rasterParams[0] ) x33 = x22;

    float derX = ( ( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 * rasterParams[3] );
    float derY = ( ( x31 + x32 + x32 + x33 ) - ( x11 + x12 + x12 + x13 ) ) / ( 8 * -rasterParams[4]);

    if ( derX == rasterParams[1] || derY == rasterParams[1] )
    {
      res = rasterParams[2];
    }
    else
    {
      // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
      // Fast formula from GDAL DEM
      const float xx = derX * derX;
      const float yy = derY * derY;
      const float xx_plus_yy = xx + yy;
      // Flat? -> white
      if ( xx_plus_yy == 0.0f )
      {
        res = clamp( 1.0f + rasterParams[12], 0.0f, 255.0f );
      }
      else
      {
        // ... then the shade value from different azimuth
        float val225_mul_127 = rasterParams[13] +
                               ( derX - derY ) * rasterParams[14];
        val225_mul_127 = ( val225_mul_127 <= 0.0 ) ? 0.0 : val225_mul_127;
        float val270_mul_127 = rasterParams[13] -
                               derX * rasterParams[15];
        val270_mul_127 = ( val270_mul_127 <= 0.0 ) ? 0.0 : val270_mul_127;
        float val315_mul_127 = rasterParams[13] +
                               ( derX + derY ) * rasterParams[14];
        val315_mul_127 = ( val315_mul_127 <= 0.0 ) ? 0.0 : val315_mul_127;
        float val360_mul_127 = rasterParams[13] -
                               derY * rasterParams[15];
        val360_mul_127 = ( val360_mul_127 <= 0.0 ) ? 0.0 : val360_mul_127;
        // ... then the weighted shading
        const float weight_225 = 0.5 * xx_plus_yy - derX * derY;
        const float weight_270 = xx;
        const float weight_315 = xx_plus_yy - weight_225;
        const float weight_360 = yy;
        const float cang_mul_127 = (
                                     ( weight_225 * val225_mul_127 +
                                       weight_270 * val270_mul_127 +
                                       weight_315 * val315_mul_127 +
                                       weight_360 * val360_mul_127 ) / xx_plus_yy ) /
                                   ( 1 + rasterParams[11] * xx_plus_yy );

        res = clamp( 1.0f + cang_mul_127, 0.0f, 255.0f );
      }
    }
  }

  // Opacity
  res = res * rasterParams[7];

  resultLine[i] = (uchar4)(res, res, res, 255 * rasterParams[7]);

}
