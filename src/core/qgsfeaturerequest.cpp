/***************************************************************************
    qgsfeaturerequest.cpp
    ---------------------
    begin                : Mai 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturerequest.h"

#include "qgsfield.h"
#include "qgsgeometry.h"

#include <limits>
#include <QStringList>

QgsFeatureRequest::QgsFeatureRequest()
    : mFilter( FilterNone )
    , mFilterExpression( 0 )
    , mFlags( 0 )
    , mMapCoordTransform( NULL )
    , mMapToPixel( NULL )
    , mMapToPixelTol( QgsFeatureRequest::MAPTOPIXEL_THRESHOLD_DEFAULT )
{
}

QgsFeatureRequest::QgsFeatureRequest( QgsFeatureId fid )
    : mFilter( FilterFid )
    , mFilterFid( fid )
    , mFilterExpression( 0 )
    , mFlags( 0 )
    , mMapCoordTransform( NULL )
    , mMapToPixel( NULL )
    , mMapToPixelTol( QgsFeatureRequest::MAPTOPIXEL_THRESHOLD_DEFAULT )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsRectangle& rect )
    : mFilter( FilterRect )
    , mFilterRect( rect )
    , mFilterExpression( 0 )
    , mFlags( 0 )
    , mMapCoordTransform( NULL )
    , mMapToPixel( NULL )
    , mMapToPixelTol( QgsFeatureRequest::MAPTOPIXEL_THRESHOLD_DEFAULT )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsExpression& expr )
    : mFilter( FilterExpression )
    , mFilterExpression( new QgsExpression( expr.dump() ) )
    , mFlags( 0 )
    , mMapCoordTransform( NULL )
    , mMapToPixel( NULL )
    , mMapToPixelTol( QgsFeatureRequest::MAPTOPIXEL_THRESHOLD_DEFAULT )
{
}

QgsFeatureRequest::QgsFeatureRequest( const QgsFeatureRequest &rh )
{
  operator=( rh );
}

QgsFeatureRequest& QgsFeatureRequest::operator=( const QgsFeatureRequest & rh )
{
  mFlags = rh.mFlags;
  mFilter = rh.mFilter;
  mFilterRect = rh.mFilterRect;
  mFilterFid = rh.mFilterFid;
  mFilterFids = rh.mFilterFids;
  if ( rh.mFilterExpression )
  {
    mFilterExpression = new QgsExpression( rh.mFilterExpression->dump() );
  }
  else
  {
    mFilterExpression = 0;
  }
  mAttrs = rh.mAttrs;
  mMapCoordTransform = rh.mMapCoordTransform;
  mMapToPixel = rh.mMapToPixel;
  mMapToPixelTol = rh.mMapToPixelTol;
  return *this;
}

QgsFeatureRequest::~QgsFeatureRequest()
{
  delete mFilterExpression;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterRect( const QgsRectangle& rect )
{
  mFilter = FilterRect;
  mFilterRect = rect;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterFid( QgsFeatureId fid )
{
  mFilter = FilterFid;
  mFilterFid = fid;
  return *this;
}

QgsFeatureRequest&QgsFeatureRequest::setFilterFids( QgsFeatureIds fids )
{
  mFilter = FilterFids;
  mFilterFids = fids;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFilterExpression( const QString& expression )
{
  mFilter = FilterExpression;
  delete mFilterExpression;
  mFilterExpression = new QgsExpression( expression );
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setFlags( QgsFeatureRequest::Flags flags )
{
  mFlags = flags;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QgsAttributeList& attrs )
{
  mFlags |= SubsetOfAttributes;
  mAttrs = attrs;
  return *this;
}


QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields )
{
  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( attrNames.contains( fields[idx].name() ) )
      mAttrs.append( idx );
  }

  return *this;
}

bool QgsFeatureRequest::acceptFeature( const QgsFeature& feature )
{
  switch ( mFilter )
  {
    case QgsFeatureRequest::FilterNone:
      return true;
      break;

    case QgsFeatureRequest::FilterRect:
      if ( feature.geometry() && feature.geometry()->intersects( mFilterRect ) )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterFid:
      if ( feature.id() == mFilterFid )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterExpression:
      if ( mFilterExpression->evaluate( feature ).toBool() )
        return true;
      else
        return false;
      break;

    case QgsFeatureRequest::FilterFids:
      if ( mFilterFids.contains( feature.id() ) )
        return true;
      else
        return false;
      break;
  }

  return true;
}

QgsFeatureRequest& QgsFeatureRequest::setCoordinateTransform( const QgsCoordinateTransform* ct ) 
{
  mMapCoordTransform = ct;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setMapToPixel( const QgsMapToPixel* mtp ) 
{
  mMapToPixel = mtp;
  return *this;
}

QgsFeatureRequest& QgsFeatureRequest::setMapToPixelTol( float map2pixelTol ) 
{
  mMapToPixelTol = map2pixelTol;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

#include "qgsrendercontext.h"
#include "qgsgeometry.h"
#include "qgsapplication.h"

//! Default Threshold of map2pixel simplification between map coordinates and device coordinates for fast rendering
const float QgsFeatureRequest::MAPTOPIXEL_THRESHOLD_DEFAULT = 1.0f;

//! Returns the squared 2D-distance of the vector defined by the two points specified
inline static float calculateLengthSquared2D( double x1, double y1, double x2, double y2 )
{
  float vx = (float)( x2 - x1 );
  float vy = (float)( y2 - y1 );

  return vx*vx + vy*vy;
}

//! Returns the MapTolerance for transform between map coordinates and device coordinates
inline static float calculateViewPixelTolerance( const QgsRectangle& boundingRect, const QgsCoordinateTransform* ct, const QgsMapToPixel* mtp )
{
  double mapUnitsPerPixel = mtp ? mtp->mapUnitsPerPixel() : 1.0;
  double mapUnitsFactor = 1;

  // Calculate one aprox factor of the size of the BBOX from the source CoordinateSystem to the target CoordinateSystem.
  if ( ct && !((QgsCoordinateTransform*)ct)->isShortCircuited() )
  {
    QgsRectangle sourceRect = boundingRect;
    QgsRectangle targetRect = ct->transform(sourceRect);

    QgsPoint minimumSrcPoint( sourceRect.xMinimum(), sourceRect.yMinimum() );
    QgsPoint maximumSrcPoint( sourceRect.xMaximum(), sourceRect.yMaximum() );
    QgsPoint minimumDstPoint( targetRect.xMinimum(), targetRect.yMinimum() );
    QgsPoint maximumDstPoint( targetRect.xMaximum(), targetRect.yMaximum() );

    double sourceHypothenuse = sqrt( calculateLengthSquared2D( minimumSrcPoint.x(), minimumSrcPoint.y(), maximumSrcPoint.x(), maximumSrcPoint.y() ) );
    double targetHypothenuse = sqrt( calculateLengthSquared2D( minimumDstPoint.x(), minimumDstPoint.y(), maximumDstPoint.x(), maximumDstPoint.y() ) );

    if (targetHypothenuse!=0) 
    mapUnitsFactor = sourceHypothenuse/targetHypothenuse;
  }
  return (float)( mapUnitsPerPixel * mapUnitsFactor );
}

//! Returns the BBOX of the specified Q-point stream
inline static QgsRectangle calculateBoundingBox( const QVector<QPointF>& points )
{
  double xmin =  std::numeric_limits<double>::max(), x,y;
  double ymin =  std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  for ( int i = 0, numPoints = points.size(); i < numPoints; ++i )
  {
    x = points[i].x();
    y = points[i].y();

    if (xmin>x) xmin = x;
    if (ymin>y) ymin = y;
    if (xmax<x) xmax = x;
    if (ymax<y) ymax = y;
  }
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

//! Returns the BBOX of the specified WKB-point stream
inline static QgsRectangle calculateBoundingBox( QGis::WkbType wkbType, unsigned char* wkb, size_t numPoints )
{
  unsigned char* wkb2 = wkb;
  
  double xmin =  std::numeric_limits<double>::max(), x,y;
  double ymin =  std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  int sizeOfDoubleX = sizeof(double);
  int sizeOfDoubleY = QGis::wkbDimensions(wkbType)==3 /*hasZValue*/ ? 2*sizeof(double) : sizeof(double);
  
  for ( size_t i = 0; i < numPoints; ++i )
  {
    memcpy( &x, wkb, sizeof( double ) ); wkb += sizeOfDoubleX;
    memcpy( &y, wkb, sizeof( double ) ); wkb += sizeOfDoubleY;

    if (xmin>x) xmin = x;
    if (ymin>y) ymin = y;
    if (xmax<x) xmax = x;
    if (ymax<y) ymax = y;
  }
  wkb = wkb2;

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

//! Generalize the WKB-geometry using the BBOX of the original geometry
inline static bool generalizeGeometry( QGis::WkbType wkbType, unsigned char* sourceWkb, size_t sourceWkbSize, unsigned char* targetWkb, size_t& targetWkbSize, const QgsRectangle& envelope, bool writeHeader )
{
  unsigned char* wkb2 = targetWkb;
  unsigned int geometryType = QGis::singleType( QGis::flatType( wkbType ) );

  int sizeOfDoubleX = sizeof(double);
  int sizeOfDoubleY = QGis::wkbDimensions(wkbType)==3 /*hasZValue*/ ? 2*sizeof(double) : sizeof(double);

  // Skip the unnecesary generalization because of is a very single geometry
  size_t minimumSize = ( geometryType==QGis::WKBLineString ? 4 + 2*(sizeOfDoubleX+sizeOfDoubleY) : 8 + 5*(sizeOfDoubleX+sizeOfDoubleY) );
  if ( writeHeader ) minimumSize += 5;
  if ( sourceWkbSize <= minimumSize )
  {
    targetWkbSize = 0;
    return false;
  }

  double x1 = envelope.xMinimum();
  double y1 = envelope.yMinimum();
  double x2 = envelope.xMaximum();
  double y2 = envelope.yMaximum();

  // Write the main header of the geometry
  if ( writeHeader )
  {
    char byteOrder = QgsApplication::endian(); // byteOrder
    memcpy( targetWkb, &byteOrder, 1 );
    targetWkb += 1;

    memcpy( targetWkb, &geometryType, 4 ); // type
    targetWkb += 4;

    if ( geometryType == QGis::WKBPolygon ) // numRings
    {
      int numRings = 1;
      memcpy( targetWkb, &numRings, 4 );
      targetWkb += 4; 
    }
  }

  // Write the generalized geometry
  if ( geometryType == QGis::WKBLineString )
  {
    int numPoints = 2;
    memcpy( targetWkb, &numPoints, 4 ); // numPoints; 
    targetWkb += 4;

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );
  }
  else
  {
    int numPoints = 5;
    memcpy( targetWkb, &numPoints, 4 ); // numPoints; 
    targetWkb += 4;

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );
  }
  targetWkbSize += targetWkb - wkb2;
  targetWkb = wkb2;
  return true;
}

//! Simplify the WKB-geometry using the specified tolerance
inline static bool simplifyWkbGeometry( const QgsFeatureRequest::Flags& requestFlags, QGis::WkbType wkbType, unsigned char* sourceWkb, size_t sourceWkbSize, unsigned char* targetWkb, size_t& targetWkbSize, const QgsRectangle& envelope, float map2pixelTol, bool writeHeader = true, bool isaLinearRing = false )
{
  bool canbeGeneralizable = true;
  bool hasZValue = QGis::wkbDimensions(wkbType)==3;
  bool result = false;

  // Can replace the geometry by its BBOX ?
  if ( ( requestFlags & QgsFeatureRequest::SimplifyEnvelope ) && (envelope.xMaximum()-envelope.xMinimum()) < map2pixelTol && (envelope.yMaximum()-envelope.yMinimum()) < map2pixelTol )
  {
    canbeGeneralizable = generalizeGeometry( wkbType, sourceWkb, sourceWkbSize, targetWkb, targetWkbSize, envelope, writeHeader );
    if (canbeGeneralizable) return true;
  }
  if (!( requestFlags & QgsFeatureRequest::SimplifyGeometry ) ) canbeGeneralizable = false;

  // Write the main header of the geometry
  if ( writeHeader )
  {
    memcpy( targetWkb, sourceWkb, 1 ); // byteOrder
    sourceWkb += 1;
    targetWkb += 1;

    int geometryType;
    memcpy( &geometryType, sourceWkb, 4 );
    int flatType = QGis::flatType( (QGis::WkbType)geometryType );
    memcpy( targetWkb, &flatType, 4 ); // type    
    sourceWkb += 4;
    targetWkb += 4;

    targetWkbSize += 5;
  }

  unsigned char* wkb1 = sourceWkb;
  unsigned char* wkb2 = targetWkb;
  unsigned int flatType = QGis::flatType( wkbType );

  // Write the geometry
  if ( flatType == QGis::WKBLineString || isaLinearRing )
  {
    double x,y, lastX=0,lastY=0;

    int sizeOfDoubleX = sizeof(double);
    int sizeOfDoubleY = QGis::wkbDimensions(wkbType)==3 /*hasZValue*/ ? 2*sizeof(double) : sizeof(double);

    int numPoints;
    memcpy( &numPoints, sourceWkb, 4);
    sourceWkb += 4;
    if (numPoints <= (isaLinearRing ? 5 : 2)) canbeGeneralizable = false;

    int numTargetPoints = 0;
    memcpy( targetWkb, &numTargetPoints, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    double* ptr = (double*)targetWkb;
    map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

    // Process each vertex...
    for ( int i = 0, numPoints_i = (isaLinearRing ? numPoints-1 : numPoints); i < numPoints_i; ++i )
    {
      memcpy( &x, sourceWkb, sizeof( double ) ); sourceWkb += sizeOfDoubleX;
      memcpy( &y, sourceWkb, sizeof( double ) ); sourceWkb += sizeOfDoubleY;
	  
      if ( i==0 || !canbeGeneralizable || calculateLengthSquared2D(x,y,lastX,lastY)>map2pixelTol )
      {
        memcpy( ptr, &x, sizeof( double ) ); lastX = x; ptr++;
        memcpy( ptr, &y, sizeof( double ) ); lastY = y; ptr++;
        numTargetPoints++;
      }
    }
    targetWkb = wkb2+4;
	
    // Fix the topology of the geometry
    if ( isaLinearRing )
    {
      memcpy( &x, targetWkb+0, sizeof( double ) );
      memcpy( &y, targetWkb+8, sizeof( double ) );
      memcpy( ptr, &x, sizeof( double ) ); ptr++;
      memcpy( ptr, &y, sizeof( double ) ); ptr++;
      numTargetPoints++;
    }
    targetWkbSize += numTargetPoints * 16;
    targetWkb = wkb2;	

    memcpy( targetWkb, &numTargetPoints, 4 );
    result = numPoints!=numTargetPoints;
  }
  else
  if ( flatType == QGis::WKBPolygon )
  {
    int numRings;
    memcpy( &numRings, sourceWkb, 4 ); 
    sourceWkb += 4;

    memcpy( targetWkb, &numRings, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    for ( int i = 0; i < numRings; ++i )
    {
      int numPoints_i;
      memcpy( &numPoints_i, sourceWkb, 4 );
      QgsRectangle envelope_i = numRings==1 ? envelope : calculateBoundingBox( wkbType, sourceWkb+4, numPoints_i );

      size_t sourceWkbSize_i = 4 + numPoints_i * (hasZValue ? 3 : 2) * sizeof(double);
      size_t targetWkbSize_i = 0;

      result |= simplifyWkbGeometry( requestFlags, wkbType, sourceWkb, sourceWkbSize_i, targetWkb, targetWkbSize_i, envelope_i, map2pixelTol, false, true );
      sourceWkb += sourceWkbSize_i;
      targetWkb += targetWkbSize_i;

      targetWkbSize += targetWkbSize_i;
    }
  }
  else
  if ( flatType == QGis::WKBMultiLineString || flatType == QGis::WKBMultiPolygon )
  {
    int numGeoms;
    memcpy( &numGeoms, sourceWkb, 4 ); 
    sourceWkb += 4;
    wkb1 += 4;

    memcpy( targetWkb, &numGeoms, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    for ( int i = 0; i < numGeoms; ++i )
    {
      size_t sourceWkbSize_i = 0;
      size_t targetWkbSize_i = 0;

      // ... calculate the wkb-size of the current child complex geometry
      if ( flatType == QGis::WKBMultiLineString )
      {
        int numPoints_i;
        memcpy( &numPoints_i, wkb1+5, 4 );
        int wkbSize_i = 4 + numPoints_i * (hasZValue ? 3 : 2) * sizeof(double);

        sourceWkbSize_i += 5 + wkbSize_i;
        wkb1 += 5 + wkbSize_i;
      }
      else
      {
        int numPrings_i;
        memcpy( &numPrings_i, wkb1+5, 4 );
        sourceWkbSize_i = 9;
        wkb1 += 9;

        for (int j = 0; j < numPrings_i; ++j)
        {
          int numPoints_i;
          memcpy( &numPoints_i, wkb1, 4);
          int wkbSize_i = 4 + numPoints_i * (hasZValue ? 3 : 2) * sizeof(double);

          sourceWkbSize_i += wkbSize_i;
          wkb1 += wkbSize_i;
        }
      }
      result |= simplifyWkbGeometry( requestFlags, QGis::singleType(wkbType), sourceWkb, sourceWkbSize_i, targetWkb, targetWkbSize_i, envelope, map2pixelTol, true, false );
      sourceWkb += sourceWkbSize_i;
      targetWkb += targetWkbSize_i;

      targetWkbSize += targetWkbSize_i;
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Returns whether the device-geometry can be replaced by its BBOX when is applied the specified map2pixel tolerance
bool QgsFeatureRequest::canbeGeneralizedByWndBoundingBox( const QgsRectangle& envelope, float mapToPixelTol )
{
  return (envelope.xMaximum()-envelope.xMinimum()) < mapToPixelTol && (envelope.yMaximum()-envelope.yMinimum()) < mapToPixelTol;
}
//! Returns whether the device-geometry can be replaced by its BBOX when is applied the specified map2pixel tolerance
bool QgsFeatureRequest::canbeGeneralizedByWndBoundingBox( const QVector<QPointF>& points, float mapToPixelTol )
{
  QgsRectangle env = calculateBoundingBox( points );
  return canbeGeneralizedByWndBoundingBox( env, mapToPixelTol);
}

//! Returns whether the envelope can be replaced by its BBOX when is applied the map2pixel context
bool QgsFeatureRequest::canbeGeneralizedByMapBoundingBox( const QgsRectangle& envelope, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mtp, float mapToPixelTol )
{
  double map2pixelTol = mapToPixelTol * calculateViewPixelTolerance( envelope, coordinateTransform, mtp );

  // Can replace the geometry by its BBOX ?
  if ( (envelope.xMaximum()-envelope.xMinimum()) < map2pixelTol && (envelope.yMaximum()-envelope.yMinimum()) < map2pixelTol )
  {
    return true;
  }
  return false;
}

//! Simplify the specified geometry (Removing duplicated points) when is applied the map2pixel context
bool QgsFeatureRequest::simplifyGeometry( const QgsFeatureRequest::Flags& requestFlags, QgsGeometry* geometry, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mtp, float mapToPixelTol )
{
  size_t targetWkbSize = 0;

  // Check whether the geometry can be simplified using the map2pixel context
  QGis::GeometryType geometryType = geometry->type();
  if ( !(geometryType==QGis::Line || geometryType==QGis::Polygon) ) return false;

  QgsRectangle envelope = geometry->boundingBox();
  QGis::WkbType wkbType = geometry->wkbType();
  double map2pixelTol = mapToPixelTol * calculateViewPixelTolerance( envelope, coordinateTransform, mtp );

  unsigned char* wkb = (unsigned char*)geometry->asWkb( );
  size_t wkbSize = geometry->wkbSize( );

  // Simplify the geometry rewriting temporally its WKB-stream for saving calloc's.
  if ( simplifyWkbGeometry( requestFlags, wkbType, wkb, wkbSize, wkb, targetWkbSize, envelope, map2pixelTol ) )
  {
    unsigned char* targetWkb = (unsigned char*)malloc( targetWkbSize );
    memcpy( targetWkb, wkb, targetWkbSize );
    geometry->fromWkb( targetWkb, targetWkbSize );
    return true;
  }
  return false;
}

//! Simplify the specified point stream (Removing duplicated points) when is applied the map2pixel context
bool QgsFeatureRequest::simplifyGeometry( const QgsFeatureRequest::Flags& requestFlags, QGis::GeometryType geometryType, const QgsRectangle& envelope, double* xptr, int xStride, double* yptr, int yStride, int pointCount, int& pointSimplifiedCount, const QgsCoordinateTransform* coordinateTransform, const QgsMapToPixel* mtp, float mapToPixelTol )
{
  bool canbeGeneralizable = ( requestFlags & QgsFeatureRequest::SimplifyGeometry );

  pointSimplifiedCount = pointCount;
  if ( geometryType == QGis::Point || geometryType == QGis::UnknownGeometry ) return false;
  pointSimplifiedCount = 0;

  double map2pixelTol = mapToPixelTol * calculateViewPixelTolerance( envelope, coordinateTransform, mtp );
  map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.
  double x,y, lastX=0, lastY=0;

  char* xsourcePtr = (char*)xptr;
  char* ysourcePtr = (char*)yptr;
  char* xtargetPtr = (char*)xptr;
  char* ytargetPtr = (char*)yptr;

  for ( int i = 0, numPoints = geometryType==QGis::Polygon ? pointCount-1 : pointCount; i < numPoints; ++i )
  {
    memcpy( &x, xsourcePtr, sizeof( double ) ); xsourcePtr += xStride;
    memcpy( &y, ysourcePtr, sizeof( double ) ); ysourcePtr += yStride;

    if ( i==0 || !canbeGeneralizable || calculateLengthSquared2D(x,y,lastX,lastY)>map2pixelTol )
    {
      memcpy( xtargetPtr, &x, sizeof( double ) ); lastX = x; xtargetPtr += xStride;
      memcpy( ytargetPtr, &y, sizeof( double ) ); lastY = y; ytargetPtr += yStride;
      pointSimplifiedCount++;
    }
  }
  if ( geometryType == QGis::Polygon )
  {
    memcpy( xtargetPtr, xptr, sizeof( double ) );
    memcpy( ytargetPtr, yptr, sizeof( double ) );
    pointSimplifiedCount++;
  }
  return pointSimplifiedCount!=pointCount;
}
