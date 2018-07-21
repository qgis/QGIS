#include "calcfirstder.cl"
<<<<<<< 30a62e1addd2541bdbecda4398381772eb8034d4

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
=======
>>>>>>> [opencl] Fix small OpenCL alg issues

__kernel void processNineCellWindow( __global float *scanLine1,
                                     __global float *scanLine2,
                                     __global float *scanLine3,
                                     __global float *resultLine,
                                     __global float *rasterParams // mInputNodataValue, mOutputNodataValue, mZFactor, mCellSizeX, mCellSizeY, zenith_rad, azimuth_rad
                                     )
{

    // Get the index of the current element
    const int i = get_global_id(0);

    if ( scanLine2[i+1] == rasterParams[0] )
    {
       resultLine[i] = rasterParams[1];
    }
    else
    {

      float derX = calcFirstDer( scanLine1[i], scanLine1[i+1], scanLine1[i+2],
                                 scanLine2[i], scanLine2[i+1], scanLine2[i+2],
                                 scanLine3[i], scanLine3[i+1], scanLine3[i+2],
                                 rasterParams[0], rasterParams[1], rasterParams[2], rasterParams[3] );

      float derY = calcFirstDer( scanLine3[i],   scanLine2[i],   scanLine1[i],
                                 scanLine3[i+1], scanLine2[i+1], scanLine1[i+1],
                                 scanLine3[i+2], scanLine2[i+2], scanLine1[i+2],
                                 rasterParams[0], rasterParams[1], rasterParams[2], rasterParams[4]);

      if ( derX == rasterParams[1] || derY == rasterParams[1] )
      {
        resultLine[i] = rasterParams[1];
      }
      else
      {

<<<<<<< 30a62e1addd2541bdbecda4398381772eb8034d4
        float slope_rad = sqrt( derX * derX + derY * derY );
        slope_rad = atan( slope_rad );
        float aspect_rad;
        if ( derX == 0.0f && derY == 0.0f)
          aspect_rad = rasterParams[7] / 2.0f;
        else
          aspect_rad = M_PI + atan2( derX, derY );

        resultLine[i] = max(0.0f, 255.0f * ( ( rasterParams[5] * cos( slope_rad ) ) +
                                            (  rasterParams[6] * sin( slope_rad ) *
                                              cos( rasterParams[7] - aspect_rad ) ) ) );
=======
        float zenith_rad = rasterParams[5];
        float azimuth_rad = rasterParams[6];
        float slope_rad = sqrt( derX * derX + derY * derY );


        slope_rad = atan( slope_rad );
        float aspect_rad;
        if ( derX == 0.0f && derY == 0.0f)
          aspect_rad = azimuth_rad / 2.0f;
        else
          aspect_rad = M_PI + atan2( derX, derY );

        resultLine[i] = max(0.0f, 255.0f * ( ( cos( zenith_rad ) * cos( slope_rad ) ) +
                                            ( sin( zenith_rad ) * sin( slope_rad ) *
                                              cos( azimuth_rad - aspect_rad ) ) ) );
>>>>>>> [opencl] Fix small OpenCL alg issues

      }
    }
}
