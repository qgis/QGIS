
// Calculate the first derivative from a 3x3 cell matrix
float calcFirstDer( float x11, float x21, float x31, float x12, float x22, float x32, float x13, float x23, float x33,
                    float inputNodataValue, float zFactor, float mCellSize )
{
 //the basic formula would be simple, but we need to test for nodata values...
 //X: return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * cellSizeX));
 //Y: return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * cellSizeY));


 if ( x11 == inputNodataValue )
 {
   x11 = x22;
 }
 if ( x12 == inputNodataValue )
 {
   x12 = x22;
 }
 if ( x13 == inputNodataValue )
 {
   x13 = x22;
 }
 if ( x21 == inputNodataValue )
 {
   x21 = x22;
 }
 if ( x23 == inputNodataValue )
 {
   x23 = x22;
 }
 if ( x31 == inputNodataValue )
 {
   x31 = x22;
 }
 if ( x32 == inputNodataValue )
 {
   x32 = x22;
 }
 if ( x33 == inputNodataValue )
 {
   x33 = x22;
 }
 return ( ( ( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 *  mCellSize )) * zFactor;
}



__kernel void processNineCellWindow( __global float *scanLine1,
                                       __global float *scanLine2,
                                       __global float *scanLine3,
                                       __global uchar4 *resultLine, // This is an image!
                                       __global float *rasterParams //  [ mInputNodataValue, mOutputNodataValue, mZFactor, mCellSizeX, mCellSizeY,
                                                                    //    azimuthRad, cosZenithRad, sinZenithRad, mOpacity ]

                         ) {

    // Get the index of the current element
    const int i = get_global_id(0);

    float res;
    if ( scanLine2[i+1] == rasterParams[1] )
    {
       res = rasterParams[2];
    }
    else
    {

      // Do the operation
      //return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * mCellSizeX))
      float derX = calcFirstDer(   scanLine1[i],   scanLine2[i],   scanLine3[i],
                                   scanLine1[i+1], scanLine2[i+1], scanLine3[i+1],
                                   scanLine1[i+2], scanLine2[i+2], scanLine3[i+2],
                                   rasterParams[0], rasterParams[2], rasterParams[3]
                                 );
      //return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * mCellSizeY));
      float derY = calcFirstDer(   scanLine1[i+2], scanLine1[i+1], scanLine1[i],
                                   scanLine2[i+2], scanLine2[i+1], scanLine2[i],
                                   scanLine3[i+2], scanLine3[i+1], scanLine3[i],
                                   rasterParams[0], rasterParams[2], rasterParams[4]
                                 );


      if ( derX == rasterParams[1] || derY == rasterParams[1] )
      {
        res = rasterParams[2];
      }
      else
      {
        float aspect = atan2( derX, - derY );
        // aspect = aspect * M_PI * 255; // for display

        float slope = sqrt( derX * derX + derY * derY );
        slope = atan( slope );
        // res = slope * M_PI * 255; for display

        // Final
        res = 255.0f * ( rasterParams[6] * cos( slope )
                                           + rasterParams[7] * sin( slope )
                                           * cos( rasterParams[5] - aspect ) );

      }
    }

    // Opacity
    res = res * rasterParams[7];

    resultLine[i] = (uchar4)(res, res, res, 255 * rasterParams[7]);

}
