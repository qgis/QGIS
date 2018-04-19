#include "calcfirstder.cl"

__kernel void processNineCellWindow( __global float *scanLine1,
                                     __global float *scanLine2,
                                     __global float *scanLine3,
                                     __global float *resultLine,
<<<<<<< 16a49cddaa18cb6d0b12335fe24c68cda183e1c0
                                     __global float *rasterParams // mInputNodataValue, mOutputNodataValue, mZFactor, mCellSizeX, mCellSizeY
                                    )
{
=======
                                     __global float *rasterParams
                       ) {
>>>>>>> Use OpenCL command queue

  // Get the index of the current element
  const int i = get_global_id(0);

<<<<<<< 16a49cddaa18cb6d0b12335fe24c68cda183e1c0
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
      float res = sqrt( derX * derX + derY * derY );
      res = atanpi( res );
      resultLine[i] = res * 180.0f;
    }
=======
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


  if ( derX == rasterParams[1] || derY == rasterParams[1] )
  {
    resultLine[i] = rasterParams[1];
  }
  else
  {
    float res = sqrt( derX * derX + derY * derY );
    res = atanpi( res );
    resultLine[i] = res * 180.0;
>>>>>>> Use OpenCL command queue
  }
}
