/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterviewport.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




struct QgsRasterViewPort
{
%TypeHeaderCode
#include <qgsrasterviewport.h>
%End

  QgsPointXY mTopLeftPoint;
%Docstring
 Coordinate (in output device coordinate system) of top left corner
   of the part of the raster that is to be rendered.*
%End

  QgsPointXY mBottomRightPoint;
%Docstring
 Coordinate (in output device coordinate system) of bottom right corner
   of the part of the raster that is to be rendered.*
%End

  int mWidth;
%Docstring
 Width, number of columns to be rendered
%End

  int mHeight;
%Docstring
 Height, number of rows to be rendered
%End

  QgsRectangle mDrawnExtent;
%Docstring
 Intersection of current map extent and layer extent
%End

  QgsCoordinateReferenceSystem mSrcCRS;
%Docstring
 Source coordinate system
%End

  QgsCoordinateReferenceSystem mDestCRS;
%Docstring
 Target coordinate system
%End

  int mSrcDatumTransform;
  int mDestDatumTransform;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterviewport.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
