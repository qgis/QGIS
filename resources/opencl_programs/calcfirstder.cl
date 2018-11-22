// Calculate the first derivative from a 3x3 cell matrix
float calcFirstDer( float x11, float x21, float x31, float x12, float x22, float x32, float x13, float x23, float x33,
                       float inputNodataValue, float outputNodataValue, float zFactor, float mCellSize )
{
 //the basic formula would be simple, but we need to test for nodata values...
 //X: return (( (x31 - x11) + 2 * (x32 - x12) + (x33 - x13) ) / (8 * cellSizeX));
 //Y: return (((x11 - x13) + 2 * (x21 - x23) + (x31 - x33)) / ( 8 * cellSizeY));

 int weight = 0;
 float sum = 0;


 //first row
 if ( x31 != inputNodataValue && x11 != inputNodataValue ) //the normal case
 {
   sum += ( x31 - x11 );
   weight += 2;
 }
 else if ( x31 == inputNodataValue && x11 != inputNodataValue && x21 != inputNodataValue ) //probably 3x3 window is at the border
 {
   sum += ( x21 - x11 );
   weight += 1;
 }
 else if ( x11 == inputNodataValue && x31 != inputNodataValue && x21 != inputNodataValue ) //probably 3x3 window is at the border
 {
   sum += ( x31 - x21 );
   weight += 1;
 }

 //second row
 if ( x32 != inputNodataValue && x12 != inputNodataValue ) //the normal case
 {
   sum += 2.0f * ( x32 - x12 );
   weight += 4;
 }
 else if ( x32 == inputNodataValue && x12 != inputNodataValue && x22 != inputNodataValue )
 {
   sum += 2.0f * ( x22 - x12 );
   weight += 2;
 }
 else if ( x12 == inputNodataValue && x32 != inputNodataValue && x22 != inputNodataValue )
 {
   sum += 2.0f * ( x32 - x22 );
   weight += 2;
 }

 //third row
 if ( x33 != inputNodataValue && x13 != inputNodataValue ) //the normal case
 {
   sum += ( x33 - x13 );
   weight += 2;
 }
 else if ( x33 == inputNodataValue && x13 != inputNodataValue && x23 != inputNodataValue )
 {
   sum += ( x23 - x13 );
   weight += 1;
 }
 else if ( x13 == inputNodataValue && x33 != inputNodataValue && x23 != inputNodataValue )
 {
   sum += ( x33 - x23 );
   weight += 1;
 }

 if ( weight == 0 )
 {
   return outputNodataValue;
 }

 return sum / ( weight * mCellSize ) * zFactor;
}

