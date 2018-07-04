#include "calcfirstder.cl"

// Slope renderer for QGIS

__kernel void processNineCellWindow( __global float *scanLine1,
                                       __global float *scanLine2,
                                       __global float *scanLine3,
                                       __global uchar4 *resultLine, // This is an image BGRA !
                                       __global float *rasterParams //  [ mInputNodataValue, mOutputNodataValue, mZFactor, mCellSizeX, mCellSizeY ]

                         ) {

    // Get the index of the current element
    const int i = get_global_id(0);

    // Do the operation
    //return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * mCellSizeX))
    float derX = calcFirstDer(   scanLine1[i],   scanLine2[i],   scanLine3[i],
                                 scanLine1[i+1], scanLine2[i+1], scanLine3[i+1],
                                 scanLine1[i+2], scanLine2[i+2], scanLine3[i+2],
                                 rasterParams[0], rasterParams[1], rasterParams[2], rasterParams[3]
                               );
    //return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * mCellSizeY));
    float derY = calcFirstDer(   scanLine1[i+2], scanLine1[i+1], scanLine1[i],
                                 scanLine2[i+2], scanLine2[i+1], scanLine2[i],
                                 scanLine3[i+2], scanLine3[i+1], scanLine3[i],
                                 rasterParams[0], rasterParams[1], rasterParams[2], rasterParams[4]
                               );


    float res;
    if ( derX == rasterParams[1] || derY == rasterParams[1] )
    {
      res = rasterParams[1];
    }
    else
    {
      float slope = sqrt( derX * derX + derY * derY );
      slope = atan( slope );
      res = slope * 255;
    }

    resultLine[i] = (uchar4)(res, res, res, 255);

}
