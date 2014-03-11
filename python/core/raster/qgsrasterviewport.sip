
struct QgsRasterViewPort
{
%TypeHeaderCode
#include <qgsrasterviewport.h>
%End
  /** \brief Coordinate (in output device coordinate system) of top left corner
   *   of the part of the raster that is to be rendered.*/
  QgsPoint mTopLeftPoint;
  /** \brief Coordinate (in output device coordinate system) of bottom right corner
   *   of the part of the raster that is to be rendered.*/
  QgsPoint mBottomRightPoint;

  /** \brief Width, number of columns to be rendered */
  int mWidth;
  /** \brief Distance in map units from bottom edge to top edge for the part of
   *  the raster that is to be rendered.*/
  /** \brief Height, number of rows to be rendered */
  int mHeight;

  /** \brief Intersection of current map extent and layer extent */
  QgsRectangle mDrawnExtent;

  /** \brief Source coordinate system */
  QgsCoordinateReferenceSystem mSrcCRS;

  /** \brief Target coordinate system */
  QgsCoordinateReferenceSystem mDestCRS;

  int mSrcDatumTransform;
  int mDestDatumTransform;
};
