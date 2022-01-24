/***************************************************************************
                         qgsmeshtracerenderer.cpp
                         -------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshtracerenderer.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsrendercontext.h"

#include <QPointer>

///@cond PRIVATE

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif


QgsVector QgsMeshVectorValueInterpolator::vectorValue( const QgsPointXY &point ) const
{
  if ( mCacheFaceIndex != -1 && mCacheFaceIndex < mTriangularMesh.triangles().count() )
  {
    QgsVector res = interpolatedValuePrivate( mCacheFaceIndex, point );
    if ( isVectorValid( res ) )
    {
      activeFaceFilter( res, mCacheFaceIndex );
      return res;
    }
  }

  //point is not on the face associated with mCacheIndex --> search for the face containing the point
  QList<int> potentialFaceIndexes = mTriangularMesh.faceIndexesForRectangle( QgsRectangle( point, point ) );
  mCacheFaceIndex = -1;
  for ( const int faceIndex : potentialFaceIndexes )
  {
    QgsVector res = interpolatedValuePrivate( faceIndex, point );
    if ( isVectorValid( res ) )
    {
      mCacheFaceIndex = faceIndex;
      activeFaceFilter( res, mCacheFaceIndex );
      return res;
    }
  }

  //--> no face found return non valid vector
  return ( QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() ) );

}

QgsMeshVectorValueInterpolator &QgsMeshVectorValueInterpolator::operator=( const QgsMeshVectorValueInterpolator &other )
{
  mTriangularMesh = other.mTriangularMesh;
  mDatasetValues = other.mDatasetValues;
  mActiveFaceFlagValues = other.mActiveFaceFlagValues;
  mFaceCache = other.mFaceCache;
  mCacheFaceIndex = other.mCacheFaceIndex;
  mUseScalarActiveFaceFlagValues = other.mUseScalarActiveFaceFlagValues;

  return *this;
}

QgsMeshVectorValueInterpolatorFromVertex::
QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &datasetVectorValues )
  : QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues )
{

}

QgsMeshVectorValueInterpolatorFromVertex::
QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetVectorValues,
    const QgsMeshDataBlock &scalarActiveFaceFlagValues )
  : QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues, scalarActiveFaceFlagValues )
{

}

QgsMeshVectorValueInterpolatorFromVertex::QgsMeshVectorValueInterpolatorFromVertex( const QgsMeshVectorValueInterpolatorFromVertex &other ):
  QgsMeshVectorValueInterpolator( other )
{}

QgsMeshVectorValueInterpolatorFromVertex *QgsMeshVectorValueInterpolatorFromVertex::clone()
{
  return new QgsMeshVectorValueInterpolatorFromVertex( *this );
}

QgsMeshVectorValueInterpolatorFromVertex &QgsMeshVectorValueInterpolatorFromVertex::
operator=( const QgsMeshVectorValueInterpolatorFromVertex &other )
{
  QgsMeshVectorValueInterpolator::operator=( other );
  return ( *this );
}

QgsVector QgsMeshVectorValueInterpolatorFromVertex::interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const
{
  QgsMeshFace face = mTriangularMesh.triangles().at( faceIndex );

  QgsPoint p1 = mTriangularMesh.vertices().at( face.at( 0 ) );
  QgsPoint p2 = mTriangularMesh.vertices().at( face.at( 1 ) );
  QgsPoint p3 = mTriangularMesh.vertices().at( face.at( 2 ) );

  QgsVector v1 = QgsVector( mDatasetValues.value( face.at( 0 ) ).x(),
                            mDatasetValues.value( face.at( 0 ) ).y() );

  QgsVector v2 = QgsVector( mDatasetValues.value( face.at( 1 ) ).x(),
                            mDatasetValues.value( face.at( 1 ) ).y() );

  QgsVector v3 = QgsVector( mDatasetValues.value( face.at( 2 ) ).x(),
                            mDatasetValues.value( face.at( 2 ) ).y() );

  return QgsMeshLayerUtils::interpolateVectorFromVerticesData(
           p1,
           p2,
           p3,
           v1,
           v2,
           v3,
           point );
}

QgsMeshVectorValueInterpolator::QgsMeshVectorValueInterpolator( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetVectorValues ):
  mTriangularMesh( triangularMesh ),
  mDatasetValues( datasetVectorValues ),
  mUseScalarActiveFaceFlagValues( false )
{}

QgsMeshVectorValueInterpolator::QgsMeshVectorValueInterpolator( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetVectorValues,
    const QgsMeshDataBlock &scalarActiveFaceFlagValues ):
  mTriangularMesh( triangularMesh ),
  mDatasetValues( datasetVectorValues ),
  mActiveFaceFlagValues( scalarActiveFaceFlagValues ),
  mUseScalarActiveFaceFlagValues( true )
{}

QgsMeshVectorValueInterpolator::QgsMeshVectorValueInterpolator( const QgsMeshVectorValueInterpolator &other ):
  mTriangularMesh( other.mTriangularMesh ),
  mDatasetValues( other.mDatasetValues ),
  mActiveFaceFlagValues( other.mActiveFaceFlagValues ),
  mFaceCache( other.mFaceCache ),
  mCacheFaceIndex( other.mCacheFaceIndex ),
  mUseScalarActiveFaceFlagValues( other.mUseScalarActiveFaceFlagValues )
{}

void QgsMeshVectorValueInterpolator::updateCacheFaceIndex( const QgsPointXY &point ) const
{
  if ( ! QgsMeshUtils::isInTriangleFace( point, mFaceCache, mTriangularMesh.vertices() ) )
  {
    mCacheFaceIndex = mTriangularMesh.faceIndexForPoint_v2( point );
  }
}

bool QgsMeshVectorValueInterpolator::isVectorValid( const QgsVector &v ) const
{
  return !( std::isnan( v.x() ) || std::isnan( v.y() ) );

}

void QgsMeshVectorValueInterpolator::activeFaceFilter( QgsVector &vector, int faceIndex ) const
{
  if ( mUseScalarActiveFaceFlagValues && ! mActiveFaceFlagValues.active( mTriangularMesh.trianglesToNativeFaces()[faceIndex] ) )
    vector = QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() ) ;
}

QSize QgsMeshStreamField::size() const
{
  return mFieldSize;
}

QPoint QgsMeshStreamField::topLeft() const
{
  return mFieldTopLeftInDeviceCoordinates;
}

int QgsMeshStreamField::resolution() const
{
  return mFieldResolution;
}

QgsPointXY QgsMeshStreamField::positionToMapCoordinates( const QPoint &pixelPosition, const QgsPointXY &positionInPixel )
{
  QgsPointXY mapPoint = mMapToFieldPixel.toMapCoordinates( pixelPosition );
  mapPoint = mapPoint + QgsVector( positionInPixel.x() * mMapToFieldPixel.mapUnitsPerPixel(),
                                   positionInPixel.y() * mMapToFieldPixel.mapUnitsPerPixel() );
  return mapPoint;
}

QgsMeshStreamField::QgsMeshStreamField(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &dataSetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  const QgsRectangle &layerExtent,
  double magnitudeMaximum, bool dataIsOnVertices,
  const QgsRenderContext &rendererContext,
  const QgsInterpolatedLineColor &vectorColoring,
  int resolution ):
  mFieldResolution( resolution ),
  mVectorColoring( vectorColoring ),
  mLayerExtent( layerExtent ),
  mMaximumMagnitude( magnitudeMaximum ),
  mRenderContext( rendererContext )
{
  if ( dataIsOnVertices )
  {
    if ( scalarActiveFaceFlagValues.isValid() )
      mVectorValueInterpolator.reset( new QgsMeshVectorValueInterpolatorFromVertex( triangularMesh,
                                      dataSetVectorValues,
                                      scalarActiveFaceFlagValues ) );
    else
      mVectorValueInterpolator.reset( new QgsMeshVectorValueInterpolatorFromVertex( triangularMesh,
                                      dataSetVectorValues ) );
  }
  else
  {
    if ( scalarActiveFaceFlagValues.isValid() )
      mVectorValueInterpolator.reset( new QgsMeshVectorValueInterpolatorFromFace( triangularMesh,
                                      dataSetVectorValues,
                                      scalarActiveFaceFlagValues ) );
    else
      mVectorValueInterpolator.reset( new QgsMeshVectorValueInterpolatorFromFace( triangularMesh,
                                      dataSetVectorValues ) );
  }
}

QgsMeshStreamField::QgsMeshStreamField( const QgsMeshStreamField &other ):
  mFieldSize( other.mFieldSize ),
  mFieldResolution( other.mFieldResolution ),
  mPen( other.mPen ),
  mTraceImage( other.mTraceImage ),
  mMapToFieldPixel( other.mMapToFieldPixel ),
  mVectorColoring( other.mVectorColoring ),
  mPixelFillingCount( other.mPixelFillingCount ),
  mMaxPixelFillingCount( other.mMaxPixelFillingCount ),
  mLayerExtent( other.mLayerExtent ),
  mMapExtent( other.mMapExtent ),
  mFieldTopLeftInDeviceCoordinates( other.mFieldTopLeftInDeviceCoordinates ),
  mValid( other.mValid ),
  mMaximumMagnitude( other.mMaximumMagnitude ),
  mPixelFillingDensity( other.mPixelFillingDensity ),
  mMinMagFilter( other.mMinMagFilter ),
  mMaxMagFilter( other.mMaxMagFilter ),
  mRenderContext( other.mRenderContext ),
  mMinimizeFieldSize( other.mMinimizeFieldSize )
{
  mPainter.reset( new QPainter( &mTraceImage ) );
  mVectorValueInterpolator =
    std::unique_ptr<QgsMeshVectorValueInterpolator>( other.mVectorValueInterpolator->clone() );
}

QgsMeshStreamField::~QgsMeshStreamField()
{
  if ( mPainter )
    mPainter->end();
}

void QgsMeshStreamField::updateSize( const QgsRenderContext &renderContext )
{
  mMapExtent = renderContext.mapExtent();
  const QgsMapToPixel &deviceMapToPixel = renderContext.mapToPixel();
  QgsRectangle layerExtent;
  try
  {
    QgsCoordinateTransform extentTransform = renderContext.coordinateTransform();
    extentTransform.setBallparkTransformsAreAppropriate( true );
    layerExtent = extentTransform.transformBoundingBox( mLayerExtent );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    //if the transform fails, consider the whole map
    layerExtent = mMapExtent;
  }

  QgsRectangle interestZoneExtent;
  if ( mMinimizeFieldSize )
    interestZoneExtent = layerExtent.intersect( mMapExtent );
  else
    interestZoneExtent = mMapExtent;

  if ( interestZoneExtent == QgsRectangle() )
  {
    mValid = false;
    mFieldSize = QSize();
    mFieldTopLeftInDeviceCoordinates = QPoint();
    initField();
    return;
  }


  QgsRectangle fieldInterestZoneInDeviceCoordinates = QgsMeshLayerUtils::boundingBoxToScreenRectangle( deviceMapToPixel, interestZoneExtent );
  mFieldTopLeftInDeviceCoordinates = QPoint( int( fieldInterestZoneInDeviceCoordinates.xMinimum() ), int( fieldInterestZoneInDeviceCoordinates.yMinimum() ) );
  int fieldWidthInDeviceCoordinate = int( fieldInterestZoneInDeviceCoordinates.width() );
  int fieldHeightInDeviceCoordinate = int ( fieldInterestZoneInDeviceCoordinates.height() );

  int fieldWidth = int( fieldWidthInDeviceCoordinate / mFieldResolution );
  int fieldHeight = int( fieldHeightInDeviceCoordinate / mFieldResolution );

  //increase the field size if this size is not adjusted to extent of zone of interest in device coordinates
  if ( fieldWidthInDeviceCoordinate % mFieldResolution > 0 )
    fieldWidth++;
  if ( fieldHeightInDeviceCoordinate % mFieldResolution > 0 )
    fieldHeight++;

  if ( fieldWidth == 0 || fieldHeight == 0 )
  {
    mFieldSize = QSize();
  }
  else
  {
    mFieldSize.setWidth( fieldWidth );
    mFieldSize.setHeight( fieldHeight );
  }

  double mapUnitPerFieldPixel;
  if ( interestZoneExtent.width() > 0 )
    mapUnitPerFieldPixel = deviceMapToPixel.mapUnitsPerPixel() * mFieldResolution * mFieldSize.width() / ( fieldWidthInDeviceCoordinate / mFieldResolution ) ;
  else
    mapUnitPerFieldPixel = 1e-8;

  int fieldRightDevice = mFieldTopLeftInDeviceCoordinates.x() + mFieldSize.width() * mFieldResolution;
  int fieldBottomDevice = mFieldTopLeftInDeviceCoordinates.y() + mFieldSize.height() * mFieldResolution;
  QgsPointXY fieldRightBottomMap = deviceMapToPixel.toMapCoordinates( fieldRightDevice, fieldBottomDevice );

  int fieldTopDevice = mFieldTopLeftInDeviceCoordinates.x();
  int fieldLeftDevice = mFieldTopLeftInDeviceCoordinates.y();
  QgsPointXY fieldTopLeftMap = deviceMapToPixel.toMapCoordinates( fieldTopDevice, fieldLeftDevice );

  double xc = ( fieldRightBottomMap.x() + fieldTopLeftMap.x() ) / 2;
  double yc = ( fieldTopLeftMap.y() + fieldRightBottomMap.y() ) / 2;

  mMapToFieldPixel = QgsMapToPixel( mapUnitPerFieldPixel,
                                    xc,
                                    yc,
                                    fieldWidth,
                                    fieldHeight,
                                    deviceMapToPixel.mapRotation()
                                  );

  initField();
  mValid = true;
}

void QgsMeshStreamField::updateSize( const QgsRenderContext &renderContext, int resolution )
{
  if ( renderContext.mapExtent() == mMapExtent && resolution == mFieldResolution )
    return;
  mFieldResolution = resolution;

  updateSize( renderContext );
}

bool QgsMeshStreamField::isValid() const
{
  return mValid;
}

void QgsMeshStreamField::addTrace( QgsPointXY startPoint )
{
  addTrace( mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint() );
}


void QgsMeshStreamField::addRandomTraces()
{
  if ( mMaximumMagnitude > 0 )
    while ( mPixelFillingCount < mMaxPixelFillingCount && !mRenderContext.renderingStopped() )
      addRandomTrace();
}

void QgsMeshStreamField::addRandomTrace()
{
  if ( !mValid )
    return;

  int xRandom =  1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.width() ) )  ;
  int yRandom = 1 + std::rand() / int ( ( RAND_MAX + 1u ) / uint( mFieldSize.height() ) ) ;
  addTrace( QPoint( xRandom, yRandom ) );
}

void QgsMeshStreamField::addGriddedTraces( int dx, int dy )
{
  int i = 0 ;
  while ( i < mFieldSize.width() && !mRenderContext.renderingStopped() )
  {
    int j = 0 ;
    while ( j < mFieldSize.height() && !mRenderContext.renderingStopped() )
    {
      addTrace( QPoint( i, j ) );
      j += dy;
    }
    i += dx;
  }
}

void QgsMeshStreamField::addTracesOnMesh( const QgsTriangularMesh &mesh, const QgsRectangle &extent )
{
  QList<int> facesInExtent = mesh.faceIndexesForRectangle( extent );
  QSet<int> vertices;
  for ( auto f : std::as_const( facesInExtent ) )
  {
    auto face = mesh.triangles().at( f );
    for ( auto i : std::as_const( face ) )
      vertices.insert( i );
  }

  for ( auto i : std::as_const( vertices ) )
  {
    addTrace( mesh.vertices().at( i ) );
  }
}

void QgsMeshStreamField::addTrace( QPoint startPixel )
{
  //This is where each traces are constructed
  if ( !mPainter )
    return;

  if ( isTraceExists( startPixel ) || isTraceOutside( startPixel ) )
    return;

  if ( !mVectorValueInterpolator )
    return;

  if ( !( mMaximumMagnitude > 0 ) )
    return;

  mPainter->setPen( mPen );

  //position in the pixelField
  double x1 = 0;
  double y1 = 0;

  std::list<QPair<QPoint, FieldData>> chunkTrace;

  QPoint currentPixel = startPixel;
  QgsVector vector;
  FieldData data;
  data.time = 1;

  while ( !mRenderContext.renderingStopped() )
  {
    QgsPointXY mapPosition = positionToMapCoordinates( currentPixel, QgsPointXY( x1, y1 ) );
    vector = mVectorValueInterpolator->vectorValue( mapPosition ) ;

    if ( std::isnan( vector.x() ) || std::isnan( vector.y() ) )
    {
      mPixelFillingCount++;
      setChunkTrace( chunkTrace );
      drawChunkTrace( chunkTrace );
      break;
    }

    /* nondimensional value :  Vu=2 when the particle need dt=1 to go through a pixel with the mMagMax magnitude
     * The nondimensional size of the side of a pixel is 2
     */
    vector = vector.rotateBy( -mMapToFieldPixel.mapRotation() * M_DEG2RAD );
    QgsVector vu = vector / mMaximumMagnitude * 2;
    data.magnitude = vector.length();

    double Vx = vu.x();
    double Vy = vu.y();
    double Vu = data.magnitude / mMaximumMagnitude * 2; //nondimensional vector magnitude

    if ( qgsDoubleNear( Vu, 0 ) )
    {
      // no trace anymore
      addPixelToChunkTrace( currentPixel, data, chunkTrace );
      simplifyChunkTrace( chunkTrace );
      setChunkTrace( chunkTrace );
      drawChunkTrace( chunkTrace );
      break;
    }

    //calculates where the particle will be after dt=1,
    QgsPointXY  nextPosition = QgsPointXY( x1, y1 ) + vu;
    int incX = 0;
    int incY = 0;
    if ( nextPosition.x() > 1 )
      incX = +1;
    if ( nextPosition.x() < -1 )
      incX = -1;
    if ( nextPosition.y() > 1 )
      incY = +1;
    if ( nextPosition.y() < -1 )
      incY = -1;

    double x2, y2;

    if ( incX != 0 || incY != 0 )
    {
      data.directionX = incX;
      data.directionY = -incY;
      //the particule leave the current pixel --> store pixels, calculates where the particle is and change the current pixel
      if ( chunkTrace.empty() )
      {
        storeInField( QPair<QPoint, FieldData>( currentPixel, data ) );
      }
      if ( addPixelToChunkTrace( currentPixel, data, chunkTrace ) )
      {
        setChunkTrace( chunkTrace );
        drawChunkTrace( chunkTrace );
        clearChunkTrace( chunkTrace );
      }

      data.time = 1;
      currentPixel += QPoint( incX, -incY );
      x1 = nextPosition.x() - 2 * incX;
      y1 = nextPosition.y() - 2 * incY;
    }
    else
    {
      /*the particule still in the pixel --> "push" the position with the vector value to join a border
       * and calculate the time spent to go to this border
       */
      if ( qgsDoubleNear( Vy, 0 ) )
      {
        y2 = y1;
        if ( Vx > 0 )
          incX = +1;
        else
          incX = -1;

        x2 = incX ;
      }
      else if ( qgsDoubleNear( Vx, 0 ) )
      {
        x2 = x1;
        if ( Vy > 0 )
          incY = +1;
        else
          incY = -1;

        y2 = incY ;
      }
      else
      {
        if ( Vy > 0 )
          x2 = x1 + ( 1 -  y1 ) * Vx / fabs( Vy ) ;
        else
          x2 = x1 + ( 1 + y1 ) * Vx / fabs( Vy ) ;
        if ( Vx > 0 )
          y2 = y1 + ( 1 - x1 ) * Vy / fabs( Vx ) ;
        else
          y2 = y1 + ( 1 + x1 ) * Vy / fabs( Vx ) ;

        if ( x2 >= 1 )
        {
          x2 = 1;
          incX = +1;
        }
        if ( x2 <= -1 )
        {
          x2 = -1;
          incX = -1;
        }
        if ( y2 >= 1 )
        {
          y2 = 1;
          incY = +1;
        }
        if ( y2 <= -1 )
        {
          y2 = -1;
          incY = -1;
        }
      }

      //calculate distance
      double dx = x2 - x1;
      double dy = y2 - y1;
      double dl = sqrt( dx * dx + dy * dy );

      data.time += dl / Vu ; //adimensional time step : this the time needed to go to the border of the pixel
      if ( data.time > 10000 ) //Guard to prevent that the particle never leave the pixel
      {
        addPixelToChunkTrace( currentPixel, data, chunkTrace );
        setChunkTrace( chunkTrace );
        drawChunkTrace( chunkTrace );
        break;
      }
      x1 = x2;
      y1 = y2;
    }

    //test if the new current pixel is already defined, if yes no need to continue
    if ( isTraceExists( currentPixel ) )
    {
      //Set the pixel in the chunk before adding the current pixel because this pixel is already defined
      setChunkTrace( chunkTrace );
      addPixelToChunkTrace( currentPixel, data, chunkTrace );
      drawChunkTrace( chunkTrace );
      break;
    }

    if ( isTraceOutside( currentPixel ) )
    {
      setChunkTrace( chunkTrace );
      drawChunkTrace( chunkTrace );
      break;
    }
  }
}

void QgsMeshStreamField::setResolution( int width )
{
  mFieldResolution = width;
}

QSize QgsMeshStreamField::imageSize() const
{
  return mFieldSize * mFieldResolution;
}

QPointF QgsMeshStreamField::fieldToDevice( const QPoint &pixel ) const
{
  QPointF p( pixel );
  p = mFieldResolution * p + QPointF( mFieldResolution - 1, mFieldResolution - 1 ) / 2;
  return p;
}

bool QgsMeshStreamField::addPixelToChunkTrace( QPoint &pixel,
    QgsMeshStreamField::FieldData &data,
    std::list<QPair<QPoint, QgsMeshStreamField::FieldData> > &chunkTrace )
{
  chunkTrace.emplace_back( pixel, data );
  if ( chunkTrace.size() == 3 )
  {
    simplifyChunkTrace( chunkTrace );
    return true;
  }
  return false;
}

void QgsMeshStreamlinesField::initField()
{
  mField = QVector<bool>( mFieldSize.width() * mFieldSize.height(), false );
  initImage();
}

QgsMeshStreamlinesField::QgsMeshStreamlinesField( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetVectorValues,
    const QgsMeshDataBlock &scalarActiveFaceFlagValues,
    const QgsRectangle &layerExtent,
    double magMax,
    bool dataIsOnVertices,
    QgsRenderContext &rendererContext,
    const QgsInterpolatedLineColor vectorColoring ):
  QgsMeshStreamField( triangularMesh,
                      datasetVectorValues,
                      scalarActiveFaceFlagValues,
                      layerExtent,
                      magMax,
                      dataIsOnVertices,
                      rendererContext,
                      vectorColoring )
{}

QgsMeshStreamlinesField::QgsMeshStreamlinesField( const QgsMeshStreamlinesField &other ):
  QgsMeshStreamField( other ),
  mField( other.mField )
{}

QgsMeshStreamlinesField &QgsMeshStreamlinesField::operator=( const QgsMeshStreamlinesField &other )
{
  QgsMeshStreamField::operator=( other );
  mField = other.mField;
  return *this;
}

void QgsMeshStreamlinesField::storeInField( const QPair<QPoint, FieldData> pixelData )
{
  int i = pixelData.first.x();
  int j = pixelData.first.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    mField[j * mFieldSize.width() + i] = true;
  }
}

void QgsMeshStreamField::setChunkTrace( std::list<QPair<QPoint, FieldData> > &chunkTrace )
{
  auto p = chunkTrace.begin();
  while ( p != chunkTrace.end() )
  {
    storeInField( ( *p ) );
    mPixelFillingCount++;
    ++p;
  }
}

void QgsMeshStreamlinesField::drawChunkTrace( const std::list<QPair<QPoint, QgsMeshStreamField::FieldData> > &chunkTrace )
{
  auto p1 = chunkTrace.begin();
  auto p2 = p1;
  p2++;
  while ( p2 != chunkTrace.end() )
  {
    double mag1 = ( *p1 ).second.magnitude;
    double mag2 = ( *p2 ).second.magnitude;
    if ( filterMag( mag1 ) && filterMag( mag2 ) )
    {
      QPen pen = mPainter->pen();
      pen.setColor( mVectorColoring.color( ( mag1 + mag2 ) / 2 ) );
      mPainter->setPen( pen );
      mPainter->drawLine( fieldToDevice( ( *p1 ).first ), fieldToDevice( ( *p2 ).first ) );
    }

    p1++;
    p2++;
  }
}

void QgsMeshStreamField::clearChunkTrace( std::list<QPair<QPoint, QgsMeshStreamField::FieldData> > &chunkTrace )
{
  auto one_before_end = std::prev( chunkTrace.end() );
  chunkTrace.erase( chunkTrace.begin(), one_before_end );
}

void QgsMeshStreamField::simplifyChunkTrace( std::list<QPair<QPoint, FieldData> > &shunkTrace )
{
  if ( shunkTrace.size() != 3 )
    return;

  auto ip3 = shunkTrace.begin();
  auto ip1 = ip3++;
  auto ip2 = ip3++;

  while ( ip3 != shunkTrace.end() && ip2 != shunkTrace.end() )
  {
    QPoint v1 = ( *ip1 ).first - ( *ip2 ).first;
    QPoint v2 = ( *ip2 ).first - ( *ip3 ).first;
    if ( v1.x()*v2.x() + v1.y()*v2.y() == 0 )
    {
      ( *ip1 ).second.time += ( ( *ip2 ).second.time ) / 2;
      ( *ip3 ).second.time += ( ( *ip2 ).second.time ) / 2;
      ( *ip1 ).second.directionX += ( *ip2 ).second.directionX;
      ( *ip1 ).second.directionY += ( *ip2 ).second.directionY;
      shunkTrace.erase( ip2 );
    }
    ip1 = ip3++;
    ip2 = ip3++;
  }
}

bool QgsMeshStreamlinesField::isTraceExists( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mField[j * mFieldSize.width() + i];
  }

  return false;
}

bool QgsMeshStreamField::isTraceOutside( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return false;
  }
  return true;
}

void QgsMeshStreamField::setMinimizeFieldSize( bool minimizeFieldSize )
{
  mMinimizeFieldSize = minimizeFieldSize;
}

QgsMeshStreamField &QgsMeshStreamField::operator=( const QgsMeshStreamField &other )
{
  mFieldSize = other.mFieldSize ;
  mFieldResolution = other.mFieldResolution;
  mPen = other.mPen;
  mTraceImage = other.mTraceImage ;
  mMapToFieldPixel = other.mMapToFieldPixel ;
  mVectorColoring = other.mVectorColoring;
  mPixelFillingCount = other.mPixelFillingCount ;
  mMaxPixelFillingCount = other.mMaxPixelFillingCount ;
  mLayerExtent = other.mLayerExtent ;
  mMapExtent = other.mMapExtent;
  mFieldTopLeftInDeviceCoordinates = other.mFieldTopLeftInDeviceCoordinates ;
  mValid = other.mValid ;
  mMaximumMagnitude = other.mMaximumMagnitude ;
  mPixelFillingDensity = other.mPixelFillingDensity ;
  mMinMagFilter = other.mMinMagFilter ;
  mMaxMagFilter = other.mMaxMagFilter ;
  mMinimizeFieldSize = other.mMinimizeFieldSize ;
  mVectorValueInterpolator =
    std::unique_ptr<QgsMeshVectorValueInterpolator>( other.mVectorValueInterpolator->clone() );

  mPainter.reset( new QPainter( &mTraceImage ) );

  return ( *this );
}

void QgsMeshStreamField::initImage()
{

  mTraceImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mTraceImage.fill( 0X00000000 );

  mPainter.reset( new QPainter( &mTraceImage ) );
  mPainter->setRenderHint( QPainter::Antialiasing, true );
  mPainter->setPen( mPen );
}

bool QgsMeshStreamField::filterMag( double value ) const
{
  return ( mMinMagFilter < 0 || value > mMinMagFilter ) && ( mMaxMagFilter < 0 || value < mMaxMagFilter );
}

QImage QgsMeshStreamField::image()
{
  return mTraceImage.scaled( mFieldSize * mFieldResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

void QgsMeshStreamField::setPixelFillingDensity( double maxFilling )
{
  mPixelFillingDensity = maxFilling;
  mMaxPixelFillingCount = int( mPixelFillingDensity * mFieldSize.width() * mFieldSize.height() );
}

void QgsMeshStreamField::setColor( QColor color )
{
  mPen.setColor( color );
}

void QgsMeshStreamField::setLineWidth( double width )
{
  mPen.setWidthF( width );
}

void QgsMeshStreamField::setFilter( double min, double max )
{
  mMinMagFilter = min;
  mMaxMagFilter = max;
}

QgsMeshVectorValueInterpolatorFromFace::QgsMeshVectorValueInterpolatorFromFace( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &datasetVectorValues ):
  QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues )
{}

QgsMeshVectorValueInterpolatorFromFace::QgsMeshVectorValueInterpolatorFromFace( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &datasetVectorValues, const QgsMeshDataBlock &scalarActiveFaceFlagValues ):
  QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues, scalarActiveFaceFlagValues )
{}

QgsMeshVectorValueInterpolatorFromFace::QgsMeshVectorValueInterpolatorFromFace( const QgsMeshVectorValueInterpolatorFromFace &other ):
  QgsMeshVectorValueInterpolator( other )
{}

QgsMeshVectorValueInterpolatorFromFace *QgsMeshVectorValueInterpolatorFromFace::clone()
{
  return new QgsMeshVectorValueInterpolatorFromFace( *this );
}

QgsMeshVectorValueInterpolatorFromFace &QgsMeshVectorValueInterpolatorFromFace::operator=( const QgsMeshVectorValueInterpolatorFromFace &other )
{
  QgsMeshVectorValueInterpolator::operator=( other );
  return ( *this );
}

QgsVector QgsMeshVectorValueInterpolatorFromFace::interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const
{
  QgsMeshFace face = mTriangularMesh.triangles().at( faceIndex );

  QgsPoint p1 = mTriangularMesh.vertices().at( face.at( 0 ) );
  QgsPoint p2 = mTriangularMesh.vertices().at( face.at( 1 ) );
  QgsPoint p3 = mTriangularMesh.vertices().at( face.at( 2 ) );

  QgsVector vect = QgsVector( mDatasetValues.value( mTriangularMesh.trianglesToNativeFaces().at( faceIndex ) ).x(),
                              mDatasetValues.value( mTriangularMesh.trianglesToNativeFaces().at( faceIndex ) ).y() );

  return QgsMeshLayerUtils::interpolateVectorFromFacesData(
           p1,
           p2,
           p3,
           vect,
           point );
}

QgsMeshVectorStreamlineRenderer::QgsMeshVectorStreamlineRenderer(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &dataSetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  bool dataIsOnVertices,
  const QgsMeshRendererVectorSettings &settings,
  QgsRenderContext &rendererContext,
  const QgsRectangle &layerExtent, double magMax ):
  mRendererContext( rendererContext )
{
  mStreamlineField.reset( new QgsMeshStreamlinesField( triangularMesh,
                          dataSetVectorValues,
                          scalarActiveFaceFlagValues,
                          layerExtent,
                          magMax,
                          dataIsOnVertices,
                          rendererContext,
                          settings.vectorStrokeColoring() ) );

  mStreamlineField->updateSize( rendererContext );
  mStreamlineField->setPixelFillingDensity( settings.streamLinesSettings().seedingDensity() );
  mStreamlineField->setLineWidth( rendererContext.convertToPainterUnits( settings.lineWidth(),
                                  QgsUnitTypes::RenderUnit::RenderMillimeters ) ) ;
  mStreamlineField->setColor( settings.color() );
  mStreamlineField->setFilter( settings.filterMin(), settings.filterMax() );

  switch ( settings.streamLinesSettings().seedingMethod() )
  {
    case QgsMeshRendererVectorStreamlineSettings::MeshGridded:
      if ( settings.isOnUserDefinedGrid() )
        mStreamlineField->addGriddedTraces( settings.userGridCellWidth(), settings.userGridCellHeight() );
      else
        mStreamlineField->addTracesOnMesh( triangularMesh, rendererContext.mapExtent() );
      break;
    case QgsMeshRendererVectorStreamlineSettings::Random:
      mStreamlineField->addRandomTraces();
      break;
  }
}

void QgsMeshVectorStreamlineRenderer::draw()
{
  if ( mRendererContext.renderingStopped() )
    return;
  mRendererContext.painter()->drawImage( mStreamlineField->topLeft(), mStreamlineField->image() );
}

QgsMeshParticleTracesField::QgsMeshParticleTracesField( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetVectorValues,
    const QgsMeshDataBlock &scalarActiveFaceFlagValues,
    const QgsRectangle &layerExtent,
    double magMax,
    bool dataIsOnVertices,
    const QgsRenderContext &rendererContext,
    const QgsInterpolatedLineColor vectorColoring ):
  QgsMeshStreamField( triangularMesh,
                      datasetVectorValues,
                      scalarActiveFaceFlagValues,
                      layerExtent,
                      magMax,
                      dataIsOnVertices,
                      rendererContext,
                      vectorColoring )
{
  std::srand( uint( ::time( nullptr ) ) );
  mPen.setCapStyle( Qt::RoundCap );
}

QgsMeshParticleTracesField::QgsMeshParticleTracesField( const QgsMeshParticleTracesField &other ):
  QgsMeshStreamField( other ),
  mTimeField( other.mTimeField ),
  mMagnitudeField( other.mMagnitudeField ),
  mDirectionField( other.mDirectionField ),
  mParticles( other.mParticles ),
  mStumpImage( other.mStumpImage ),
  mTimeStep( other.mTimeStep ),
  mParticlesLifeTime( other.mParticlesLifeTime ),
  mParticlesCount( other.mParticlesCount ),
  mTailFactor( other.mTailFactor ),
  mParticleColor( other.mParticleColor ),
  mParticleSize( other.mParticleSize ),
  mStumpFactor( other.mStumpFactor )
{}

void QgsMeshParticleTracesField::addParticle( const QPoint &startPoint, double lifeTime )
{
  addTrace( startPoint );
  if ( time( startPoint ) > 0 )
  {
    QgsMeshTraceParticle p;
    p.lifeTime = lifeTime;
    p.position = startPoint;
    mParticles.append( p );
  }

}

void QgsMeshParticleTracesField::addParticleXY( const QgsPointXY &startPoint, double lifeTime )
{
  addParticle( mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint(), lifeTime );
}

void QgsMeshParticleTracesField::moveParticles()
{
  stump();
  for ( auto &p : mParticles )
  {
    double spentTime = p.remainingTime; //adjust with the past remaining time
    size_t countAdded = 0;
    while ( spentTime < mTimeStep && p.lifeTime > 0 )
    {
      double timeToSpend = double( time( p.position ) );
      if ( timeToSpend > 0 )
      {
        p.lifeTime -= timeToSpend;
        spentTime += timeToSpend;
        QPoint dir = direction( p.position );
        if ( p.lifeTime > 0 )
        {
          p.position += dir;
          p.tail.emplace_back( p.position );
          countAdded++;
        }
        else
        {
          break;
        }
      }
      else
      {
        p.lifeTime = -1;
        break;
      }
    }

    if ( p.lifeTime <= 0 )
    {
      // the particle is not alive anymore
      p.lifeTime = 0;
      p.tail.clear();
    }
    else
    {
      p.remainingTime = spentTime - mTimeStep;
      while ( int( p.tail.size() ) > mMinTailLength && p.tail.size() > countAdded * mTailFactor )
        p.tail.erase( p.tail.begin() );
      drawParticleTrace( p );
    }
  }

  //remove empty (dead particles)
  int i = 0;
  while ( i < mParticles.count() )
  {
    if ( mParticles.at( i ).tail.size() == 0 )
      mParticles.removeAt( i );
    else
      ++i;
  }

  //add new particles if needed
  if ( mParticles.count() < mParticlesCount )
    addRandomParticles();
}

void QgsMeshParticleTracesField::addRandomParticles()
{
  if ( !isValid() )
    return;

  if ( mParticlesCount < 0 ) //for tests, add one particle on the center of the map
  {
    addParticleXY( QgsPointXY( mMapToFieldPixel.xCenter(), mMapToFieldPixel.yCenter() ), mParticlesLifeTime );
    return;
  }

  int count = mParticlesCount - mParticles.count();

  for ( int i = 0; i < count; ++i )
  {
    int xRandom =  1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.width() ) )  ;
    int yRandom = 1 + std::rand() / int ( ( RAND_MAX + 1u ) / uint( mFieldSize.height() ) ) ;
    double lifeTime = ( std::rand() / ( ( RAND_MAX + 1u ) / mParticlesLifeTime ) );
    addParticle( QPoint( xRandom, yRandom ), lifeTime );
  }
}

void QgsMeshParticleTracesField::storeInField( const QPair<QPoint, QgsMeshStreamField::FieldData> pixelData )
{
  int i = pixelData.first.x();
  int j = pixelData.first.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    mTimeField[j * mFieldSize.width() + i] = pixelData.second.time;
    int d = pixelData.second.directionX + 2 + ( pixelData.second.directionY + 1 ) * 3;
    mDirectionField[j * mFieldSize.width() + i] = static_cast<char>( d );
    mMagnitudeField[j * mFieldSize.width() + i] = pixelData.second.magnitude;
  }
}

void QgsMeshParticleTracesField::initField()
{
  mTimeField = QVector<float>( mFieldSize.width() * mFieldSize.height(), -1 );
  mDirectionField = QVector<char>( mFieldSize.width() * mFieldSize.height(), static_cast<char>( int( 0 ) ) );
  mMagnitudeField = QVector<float>( mFieldSize.width() * mFieldSize.height(), 0 );
  initImage();
  mStumpImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mStumpImage.fill( QColor( 0, 0, 0, mStumpFactor ) ); //alpha=0 -> no persitence, alpha=255 -> total persistence
}

bool QgsMeshParticleTracesField::isTraceExists( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mTimeField[j * mFieldSize.width() + i] >= 0;
  }

  return false;
}

void QgsMeshParticleTracesField::setStumpParticleWithLifeTime( bool stumpParticleWithLifeTime )
{
  mStumpParticleWithLifeTime = stumpParticleWithLifeTime;
}

void QgsMeshParticleTracesField::setParticlesColor( const QColor &c )
{
  mVectorColoring.setColor( c );
}

void QgsMeshParticleTracesField::setMinTailLength( int minTailLength )
{
  mMinTailLength = minTailLength;
}

QgsMeshParticleTracesField &QgsMeshParticleTracesField::operator=( const QgsMeshParticleTracesField &other )
{
  QgsMeshStreamField::operator=( other );
  mTimeField = other.mTimeField;
  mDirectionField = other.mDirectionField;
  mParticles = other.mParticles;
  mStumpImage = other.mStumpImage;
  mTimeStep = other.mTimeStep;
  mParticlesLifeTime = other.mParticlesLifeTime;
  mParticlesCount = other.mParticlesCount;
  mTailFactor = other.mTailFactor;
  mParticleColor = other.mParticleColor;
  mParticleSize = other.mParticleSize;
  mStumpFactor = other.mStumpFactor;

  return ( *this );
}

void QgsMeshParticleTracesField::setTailFactor( double tailFactor )
{
  mTailFactor = tailFactor;
}

void QgsMeshParticleTracesField::setParticleSize( double particleSize )
{
  mParticleSize = particleSize;
}

void QgsMeshParticleTracesField::setTimeStep( double timeStep )
{
  mTimeStep = timeStep;
}

void QgsMeshParticleTracesField::setParticlesLifeTime( double particlesLifeTime )
{
  mParticlesLifeTime = particlesLifeTime;
}

QImage QgsMeshParticleTracesField::imageRendered() const
{
  return mTraceImage;
}

void QgsMeshParticleTracesField::stump()
{
  QgsScopedQPainterState painterState( mPainter.get() );
  mPainter->setCompositionMode( QPainter::CompositionMode_DestinationIn );
  mPainter->drawImage( QPoint( 0, 0 ), mStumpImage );
}

void QgsMeshParticleTracesField::setStumpFactor( int sf )
{
  mStumpFactor = sf;
  mStumpImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mStumpImage.fill( QColor( 0, 0, 0, mStumpFactor ) );
}

QPoint QgsMeshParticleTracesField::direction( QPoint position ) const
{
  int i = position.x();
  int j = position.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    int dir = static_cast<int>( mDirectionField[j * mFieldSize.width() + i] );
    if ( dir != 0 && dir < 10 )
      return QPoint( ( dir - 1 ) % 3 - 1, ( dir - 1 ) / 3 - 1 );
  }
  return QPoint( 0, 0 );
}

float QgsMeshParticleTracesField::time( QPoint position ) const
{
  int i = position.x();
  int j = position.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mTimeField[j * mFieldSize.width() + i];
  }
  return -1;
}

float QgsMeshParticleTracesField::magnitude( QPoint position ) const
{
  int i = position.x();
  int j = position.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mMagnitudeField[j * mFieldSize.width() + i];
  }
  return -1;
}

void QgsMeshParticleTracesField::drawParticleTrace( const QgsMeshTraceParticle &particle )
{
  const std::list<QPoint> &tail = particle.tail;
  if ( tail.size() == 0 )
    return;
  double iniWidth = mParticleSize;
  double finWidth = 0;

  size_t pixelCount = tail.size();

  double transparency = 1;
  if ( mStumpParticleWithLifeTime )
    transparency = sin( M_PI * particle.lifeTime / mParticlesLifeTime );

  double dw;
  if ( pixelCount > 1 )
    dw = ( iniWidth - finWidth ) / ( pixelCount );
  else
    dw = 0;

  auto ip1 = std::prev( tail.end() );
  auto ip2 = std::prev( ip1 );
  int i = 0;
  while ( ip1 != tail.begin() )
  {
    QPointF p1 = fieldToDevice( ( *ip1 ) );
    QPointF p2 = fieldToDevice( ( *ip2 ) );
    QColor traceColor = mVectorColoring.color( magnitude( *ip1 ) );
    traceColor.setAlphaF( traceColor.alphaF()*transparency );
    mPen.setColor( traceColor );
    mPen.setWidthF( iniWidth - i * dw );
    mPainter->setPen( mPen );
    mPainter->drawLine( p1, p2 );
    ip1--;
    ip2--;
    ++i;
  }
}

void QgsMeshParticleTracesField::setParticlesCount( int particlesCount )
{
  mParticlesCount = particlesCount;
}

QgsMeshVectorTraceAnimationGenerator::QgsMeshVectorTraceAnimationGenerator( const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &dataSetVectorValues,
    const QgsMeshDataBlock &scalarActiveFaceFlagValues,
    bool dataIsOnVertices,
    const QgsRenderContext &rendererContext,
    const QgsRectangle &layerExtent,
    double magMax,
    const QgsMeshRendererVectorSettings &vectorSettings ):
  mRendererContext( rendererContext )
{
  mParticleField = std::unique_ptr<QgsMeshParticleTracesField>( new QgsMeshParticleTracesField( triangularMesh,
                   dataSetVectorValues,
                   scalarActiveFaceFlagValues,
                   layerExtent,
                   magMax,
                   dataIsOnVertices,
                   rendererContext,
                   vectorSettings.vectorStrokeColoring() ) ) ;
  mParticleField->updateSize( rendererContext ) ;
}

QgsMeshVectorTraceAnimationGenerator::QgsMeshVectorTraceAnimationGenerator( QgsMeshLayer *layer, const QgsRenderContext &rendererContext ):
  mRendererContext( rendererContext )
{
  if ( !layer->triangularMesh() )
    layer->reload();

  QgsMeshDataBlock vectorDatasetValues;
  QgsMeshDataBlock scalarActiveFaceFlagValues;
  bool vectorDataOnVertices;
  double magMax;

  QgsMeshDatasetIndex datasetIndex = layer->activeVectorDatasetAtTime( rendererContext.temporalRange() );

  // Find out if we can use cache up to date. If yes, use it and return
  int datasetGroupCount = layer->dataProvider()->datasetGroupCount();
  const QgsMeshRendererVectorSettings vectorSettings = layer->rendererSettings().vectorSettings( datasetIndex.group() );
  QgsMeshLayerRendererCache *cache = layer->rendererCache();

  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveVectorDatasetIndex == datasetIndex ) )
  {
    vectorDatasetValues = cache->mVectorDatasetValues;
    scalarActiveFaceFlagValues = cache->mScalarActiveFaceFlagValues;
    magMax = cache->mVectorDatasetMagMaximum;
    vectorDataOnVertices = cache->mVectorDataType == QgsMeshDatasetGroupMetadata::DataOnVertices;
  }
  else
  {
    const QgsMeshDatasetGroupMetadata metadata =
      layer->dataProvider()->datasetGroupMetadata( datasetIndex.group() );
    magMax = metadata.maximum();
    vectorDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;

    int count;
    if ( vectorDataOnVertices )
      count = layer->nativeMesh()->vertices.count();
    else
      count = layer->nativeMesh()->faces.count();

    vectorDatasetValues = QgsMeshLayerUtils::datasetValues( layer, datasetIndex, 0, count );

    scalarActiveFaceFlagValues = layer->dataProvider()->areFacesActive(
                                   datasetIndex,
                                   0,
                                   layer->nativeMesh()->faces.count() );
  }

  mParticleField = std::unique_ptr<QgsMeshParticleTracesField>( new QgsMeshParticleTracesField( ( *layer->triangularMesh() ),
                   vectorDatasetValues,
                   scalarActiveFaceFlagValues,
                   layer->extent(),
                   magMax,
                   vectorDataOnVertices,
                   rendererContext,
                   vectorSettings.vectorStrokeColoring() ) )  ;

  mParticleField->setMinimizeFieldSize( false );
  mParticleField->updateSize( mRendererContext );
}

QgsMeshVectorTraceAnimationGenerator::QgsMeshVectorTraceAnimationGenerator( const QgsMeshVectorTraceAnimationGenerator &other ):
  mRendererContext( other.mRendererContext ),
  mFPS( other.mFPS ),
  mVpixMax( other.mVpixMax ),
  mParticleLifeTime( other.mParticleLifeTime )
{
  mParticleField = std::unique_ptr<QgsMeshParticleTracesField>(
                     new QgsMeshParticleTracesField( *other.mParticleField ) );
}


void QgsMeshVectorTraceAnimationGenerator::seedRandomParticles( int count )
{
  mParticleField->setParticlesCount( count );
  mParticleField->addRandomParticles();
}

QImage QgsMeshVectorTraceAnimationGenerator::imageRendered()
{
  mParticleField->moveParticles();
  return mParticleField->image();
}

void QgsMeshVectorTraceAnimationGenerator::setFPS( int FPS )
{
  if ( FPS > 0 )
    mFPS = FPS;
  else
    mFPS = 1;

  updateFieldParameter();
}

void QgsMeshVectorTraceAnimationGenerator::setMaxSpeedPixel( int max )
{
  mVpixMax = max;
  updateFieldParameter();
}

void QgsMeshVectorTraceAnimationGenerator::setParticlesLifeTime( double particleLifeTime )
{
  mParticleLifeTime = particleLifeTime;
  updateFieldParameter();
}

void QgsMeshVectorTraceAnimationGenerator::setParticlesColor( const QColor &c )
{
  mParticleField->setParticlesColor( c );
}

void QgsMeshVectorTraceAnimationGenerator::setParticlesSize( double width )
{
  mParticleField->setParticleSize( width );
}

void QgsMeshVectorTraceAnimationGenerator::setTailFactor( double fct )
{
  mParticleField->setTailFactor( fct );
}

void QgsMeshVectorTraceAnimationGenerator::setMinimumTailLength( int l )
{
  mParticleField->setMinTailLength( l );
}

void QgsMeshVectorTraceAnimationGenerator::setTailPersitence( double p )
{
  if ( p < 0 )
    p = 0;
  if ( p > 1 )
    p = 1;
  mParticleField->setStumpFactor( int( 255 * p ) );
}

QgsMeshVectorTraceAnimationGenerator &QgsMeshVectorTraceAnimationGenerator::operator=( const QgsMeshVectorTraceAnimationGenerator &other )
{
  mParticleField.reset( new QgsMeshParticleTracesField( *mParticleField ) );
  const_cast<QgsRenderContext &>( mRendererContext ) = other.mRendererContext;
  mFPS = other.mFPS;
  mVpixMax = other.mVpixMax;
  mParticleLifeTime = other.mParticleLifeTime;

  return ( *this );
}

void QgsMeshVectorTraceAnimationGenerator::updateFieldParameter()
{
  double fieldTimeStep = mVpixMax / mFPS;
  double fieldLifeTime = mParticleLifeTime * mFPS * fieldTimeStep;
  mParticleField->setTimeStep( fieldTimeStep );
  mParticleField->setParticlesLifeTime( fieldLifeTime );
}

QgsMeshVectorTraceRenderer::QgsMeshVectorTraceRenderer(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &dataSetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  bool dataIsOnVertices,
  const QgsMeshRendererVectorSettings &settings,
  QgsRenderContext &rendererContext,
  const QgsRectangle &layerExtent,
  double magMax ):
  mRendererContext( rendererContext )
{
  mParticleField = std::unique_ptr<QgsMeshParticleTracesField>( new QgsMeshParticleTracesField( triangularMesh,
                   dataSetVectorValues,
                   scalarActiveFaceFlagValues,
                   layerExtent,
                   magMax,
                   dataIsOnVertices,
                   rendererContext,
                   settings.vectorStrokeColoring() ) ) ;
  mParticleField->updateSize( rendererContext ) ;

  mParticleField->setParticleSize( rendererContext.convertToPainterUnits(
                                     settings.lineWidth(), QgsUnitTypes::RenderUnit::RenderMillimeters ) );
  mParticleField->setParticlesCount( settings.tracesSettings().particlesCount() );
  mParticleField->setTailFactor( 1 );
  mParticleField->setStumpParticleWithLifeTime( false );
  mParticleField->setTimeStep( rendererContext.convertToPainterUnits( settings.tracesSettings().maximumTailLength(),
                               settings.tracesSettings().maximumTailLengthUnit() ) ); //as the particles go through 1 pix for dt=1 and Vmax
  mParticleField->addRandomParticles();
  mParticleField->moveParticles();
}

void QgsMeshVectorTraceRenderer::draw()
{
  if ( mRendererContext.renderingStopped() )
    return;
  mRendererContext.painter()->drawImage( mParticleField->topLeft(), mParticleField->image() );
}

///@endcond
