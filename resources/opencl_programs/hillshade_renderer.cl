__kernel void processNineCellWindow( __global float *scanLine1,
                                     __global float *scanLine2,
                                     __global float *scanLine3,
                                     __global uchar4 *resultLine, // This is an image!
                                     __global float *rasterParams

                                    )
{

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

    if ( x22 == rasterParams[0] )
    {
      float alpha = rasterParams[8] * rasterParams[16];
      resultLine[i] = (uchar4)(rasterParams[13] * alpha,
                               rasterParams[14] * alpha,
                               rasterParams[15] * alpha, 255 * alpha);
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

        if ( derX == rasterParams[0] ||
             derX == rasterParams[0] )
        {
          float alpha = rasterParams[5] * rasterParams[16];
          resultLine[i] = (uchar4)(rasterParams[13] * alpha,
                                   rasterParams[14] * alpha,
                                   rasterParams[15] * alpha, 255 * alpha);
        }
        else
        {
          float res;
          if ( rasterParams[17] ) // Multi directional?
          {
            // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
            // Fast formula from GDAL DEM
            const float xx = derX * derX;
            const float yy = derY * derY;
            const float xx_plus_yy = xx + yy;
            // Flat?
            if ( xx_plus_yy == 0.0f )
            {
              res = clamp( 1.0f + rasterParams[9], 0.0f, 255.0f );
            }
            else
            {
              // ... then the shade value from different azimuth
              float val225_mul_127 = rasterParams[10] +
                                     ( derX - derY ) * rasterParams[11];
              val225_mul_127 = ( val225_mul_127 <= 0.0f ) ? 0.0f : val225_mul_127;
              float val270_mul_127 = rasterParams[10] -
                                     derX * rasterParams[12];
              val270_mul_127 = ( val270_mul_127 <= 0.0f ) ? 0.0f : val270_mul_127;
              float val315_mul_127 = rasterParams[10] +
                                     ( derX + derY ) * rasterParams[11];
              val315_mul_127 = ( val315_mul_127 <= 0.0f ) ? 0.0f : val315_mul_127;
              float val360_mul_127 = rasterParams[10] -
                                     derY * rasterParams[12];
              val360_mul_127 = ( val360_mul_127 <= 0.0f ) ? 0.0f : val360_mul_127;
              // ... then the weighted shading
              const float weight_225 = 0.5f * xx_plus_yy - derX * derY;
              const float weight_270 = xx;
              const float weight_315 = xx_plus_yy - weight_225;
              const float weight_360 = yy;
              const float cang_mul_127 = (
                                           ( weight_225 * val225_mul_127 +
                                             weight_270 * val270_mul_127 +
                                             weight_315 * val315_mul_127 +
                                             weight_360 * val360_mul_127 ) / xx_plus_yy ) /
                                         ( 1 + rasterParams[8] * xx_plus_yy );
              res = clamp( 1.0f + cang_mul_127, 0.0f, 255.0f );
            }
          }
          else
          {
            res = ( rasterParams[9] -
                    ( derY * rasterParams[6] -
                      derX * rasterParams[7] )) /
                      sqrt( 1 + rasterParams[8] *
                      ( derX * derX + derY * derY ) );
            res = res <= 0.0f ? 1.0f : 1.0f + res;
          }
          res = res * rasterParams[5];
          resultLine[i] = (uchar4)(res, res, res, 255 * rasterParams[5]);
        }
    }
}
