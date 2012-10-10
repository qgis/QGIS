
struct QgsRasterViewPort
{
%TypeHeaderCode
#include <qgsrasterviewport.h>
%End

  // NOT IN MAP SPACE BUT DEVICE SPACE
  /** \brief Coordinate (in geographic coordinate system) of top left corner of the part of the raster that
   * is to be rendered.*/
  QgsPoint topLeftPoint;
  /** \brief Coordinate (in geographic coordinate system) of bottom right corner of the part of the raster that
   * is to be rendered.*/
  QgsPoint bottomRightPoint;
  /** \brief Distance in map units from left edge to right edge for the part of the raster that
   * is to be rendered.*/

  int drawableAreaXDim;
  /** \brief Distance in map units from bottom edge to top edge for the part of the raster that
   * is to be rendered.*/
  int drawableAreaYDim;

  // intersection of current map extent and layer extent
  QgsRectangle mDrawnExtent;

  // Source coordinate system
  QgsCoordinateReferenceSystem mSrcCRS;

  // Target coordinate system
  QgsCoordinateReferenceSystem mDestCRS;
};
