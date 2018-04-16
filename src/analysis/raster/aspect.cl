float calcFirstDer( float x11, float x21, float x31, float x12, float x22, float x32, float x13, float x23, float x33,
                       float mInputNodataValue, float mOutputNodataValue, float mZFactor, float mCellSize )
{
 //the basic formula would be simple, but we need to test for nodata values...
 //X: return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * mCellSizeX));
 //Y: return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * mCellSizeY));

 int weight = 0;
 float sum = 0;


 //first row
 if ( x31 != mInputNodataValue && x11 != mInputNodataValue ) //the normal case
 {
   sum += ( x31 - x11 );
   weight += 2;
 }
 else if ( x31 == mInputNodataValue && x11 != mInputNodataValue && x21 != mInputNodataValue ) //probably 3x3 window is at the border
 {
   sum += ( x21 - x11 );
   weight += 1;
 }
 else if ( x11 == mInputNodataValue && x31 != mInputNodataValue && x21 != mInputNodataValue ) //probably 3x3 window is at the border
 {
   sum += ( x31 - x21 );
   weight += 1;
 }

 //second row
 if ( x32 != mInputNodataValue && x12 != mInputNodataValue ) //the normal case
 {
   sum += 2.0f * ( x32 - x12 );
   weight += 4;
 }
 else if ( x32 == mInputNodataValue && x12 != mInputNodataValue && x22 != mInputNodataValue )
 {
   sum += 2.0f * ( x22 - x12 );
   weight += 2;
 }
 else if ( x12 == mInputNodataValue && x32 != mInputNodataValue && x22 != mInputNodataValue )
 {
   sum += 2.0f * ( x32 - x22 );
   weight += 2;
 }

 //third row
 if ( x33 != mInputNodataValue && x13 != mInputNodataValue ) //the normal case
 {
   sum += ( x33 - x13 );
   weight += 2;
 }
 else if ( x33 == mInputNodataValue && x13 != mInputNodataValue && x23 != mInputNodataValue )
 {
   sum += ( x23 - x13 );
   weight += 1;
 }
 else if ( x13 == mInputNodataValue && x33 != mInputNodataValue && x23 != mInputNodataValue )
 {
   sum += ( x33 - x23 );
   weight += 1;
 }

 if ( weight == 0 )
 {
   return mOutputNodataValue;
 }

 return sum / ( weight * mCellSize ) * mZFactor;
}


__kernel void processNineCellWindow( __global float *scanLine1,
                                     __global float *scanLine2,
                                     __global float *scanLine3,
                                     __global float *resultLine,
                                     __global float *rasterParams
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


  if ( derX == rasterParams[1] || derY == rasterParams[1] ||
       ( derX == 0.0f && derY == 0.0f) )
  {
    resultLine[i] = rasterParams[1];
  }
  else
  {
    // 180.0 / M_PI = 57.29577951308232
    float aspect = atan2( derX, derY ) * 57.29577951308232;
    if ( aspect < 0 )
        resultLine[i] = 90.0f - aspect;
    else if (aspect > 90.0f)
        // 360 + 90 = 450
        resultLine[i] = 450.0f - aspect;
    else
        resultLine[i] = 90.0 - aspect;
  }
}
