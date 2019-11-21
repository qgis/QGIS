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

///@cond PRIVATE

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

QgsMeshVectorValueInterpolatorFromVertex::QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &datasetVectorValues ):
  QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues )
{

}

QgsMeshVectorValueInterpolatorFromVertex::QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &datasetVectorValues, const QgsMeshDataBlock &scalarActiveFaceFlagValues ):
  QgsMeshVectorValueInterpolator( triangularMesh, datasetVectorValues, scalarActiveFaceFlagValues )
{

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

QgsMeshStreamField::QgsMeshStreamField( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &dataSetVectorValues, const QgsMeshDataBlock &scalarActiveFaceFlagValues, const QgsRectangle &layerExtent, double magMax, bool dataIsOnVertices, QgsRenderContext &rendererContext, int resolution ):
  mFieldResolution( resolution ),
  mLayerExtent( layerExtent ),
  mMagMax( magMax ),
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

void QgsMeshStreamField::updateSize( const QgsRenderContext &renderContext )
{
  mMapExtent = renderContext.mapExtent();
  const QgsMapToPixel &deviceMapToPixel = renderContext.mapToPixel();
  QgsRectangle layerExtent;
  try
  {
    layerExtent = renderContext.coordinateTransform().transform( mLayerExtent );

  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    layerExtent = mLayerExtent;
  }

  QgsRectangle interestZoneExtent = layerExtent.intersect( mMapExtent );
  if ( interestZoneExtent == QgsRectangle() )
  {
    mValid = false;
    mFieldSize = QSize();
    mFieldTopLeftInDeviceCoordinates = QPoint();
    initField();
    return;
  }

  QgsPointXY interestZoneTopLeft = deviceMapToPixel.transform( QgsPointXY( interestZoneExtent.xMinimum(), interestZoneExtent.yMaximum() ) );
  QgsPointXY interestZoneBottomRight = deviceMapToPixel.transform( QgsPointXY( interestZoneExtent.xMaximum(), interestZoneExtent.yMinimum() ) );

  mFieldTopLeftInDeviceCoordinates = interestZoneTopLeft.toQPointF().toPoint();
  QPoint mFieldBottomRightInDeviceCoordinates = interestZoneBottomRight.toQPointF().toPoint();
  int fieldWidthInDeviceCoordinate = mFieldBottomRightInDeviceCoordinates.x() - mFieldTopLeftInDeviceCoordinates.x();
  int fieldHeightInDeviceCoordinate = mFieldBottomRightInDeviceCoordinates.y() - mFieldTopLeftInDeviceCoordinates.y();

  int fieldWidth = int( fieldWidthInDeviceCoordinate / mFieldResolution );
  int fieldHeight = int( fieldHeightInDeviceCoordinate / mFieldResolution );

  //increase the field size if this size is not adjusted to extent of zone of interest in device coordinates
  if ( fieldWidthInDeviceCoordinate % mFieldResolution > 0 )
    fieldWidth++;
  if ( fieldHeightInDeviceCoordinate % mFieldResolution > 0 )
    fieldHeight++;

  mFieldSize.setWidth( fieldWidth );
  mFieldSize.setHeight( fieldHeight );

  double mapUnitPerFieldPixel;
  if ( interestZoneExtent.width() > 0 )
    mapUnitPerFieldPixel = interestZoneExtent.width() / fieldWidthInDeviceCoordinate * mFieldResolution;
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
                                    fieldHeight, 0 );

  mLayerPixelExtent.setTopLeft(
    mMapToFieldPixel.transform( QgsPointXY( layerExtent.xMinimum(), layerExtent.yMaximum() ) ).toQPointF().toPoint() );
  mLayerPixelExtent.setBottomRight(
    mMapToFieldPixel.transform( QgsPointXY( layerExtent.xMaximum(), layerExtent.yMinimum() ) ).toQPointF().toPoint() );

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
  QPoint sp;
  sp = mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint();
  addTrace( mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint() );
}


void QgsMeshStreamField::addRandomTraces()
{
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
  for ( auto f : qgis::as_const( facesInExtent ) )
  {
    auto face = mesh.triangles().at( f );
    for ( auto i : qgis::as_const( face ) )
      vertices.insert( i );
  }

  for ( auto i : qgis::as_const( vertices ) )
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

  mPainter->setPen( mPen );

  bool end = false;
  //position in the pixelField
  double x1 = 0;
  double y1 = 0;

  std::list<QPair<QPoint, double>> shunkTrace;

  QPoint currentPixel = startPixel;
  QgsVector vector;

  float dt = 0;

  while ( !end )
  {
    QgsPointXY mapPosition = positionToMapCoordinates( currentPixel, QgsPointXY( x1, y1 ) );
    vector = mVectorValueInterpolator->vectorValue( mapPosition ) ;

    if ( std::isnan( vector.x() ) || std::isnan( vector.y() ) )
    {
      mPixelFillingCount++;
      break;
    }

    /* adimensional value :  Vu=2 when the particule need dt=1 to go through a pixel
     * The size of the side of a pixel is 2
     */
    QgsVector vu = vector / mMagMax * 2;
    double mag = vector.length();
    double Vx = vu.x();
    double Vy = vu.y();
    double Vu = mag / mMagMax * 2; //nondimensional vector magnitude

    if ( qgsDoubleNear( Vu, 0 ) )
    {
      // no trace anymore
      mPixelFillingCount++;
      break;
    }

    //calculates where the particule will be after dt=1,
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
      //the particule leave the current pixel --> store pixels, calculate where the particule is and change the current pixel
      shunkTrace.emplace_back( currentPixel, mag );
      if ( shunkTrace.size() == 3 )
      {
        simplifyShunkTrace( shunkTrace );
        drawShunkTrace( shunkTrace );
      }

      dt = 1;
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

      dt += dl / Vu ; //adimensional time step : this the time needed to go to the border of the pixel

      if ( dt > 10000 ) //Guard to prevent that the particle never leave the pixel
      {
        mPixelFillingCount++;
        break;
      }
      x1 = x2;
      y1 = y2;
    }

    //test if the new current pixel is already defined, if yes no need to continue
    if ( isTraceExists( currentPixel ) )
    {
      shunkTrace.emplace_back( currentPixel, mag );
      if ( shunkTrace.size() == 3 )
        simplifyShunkTrace( shunkTrace );
      drawShunkTrace( shunkTrace );
      break;
    }

    if ( isTraceOutside( currentPixel ) )
    {
      if ( shunkTrace.size() == 3 )
        simplifyShunkTrace( shunkTrace );
      drawShunkTrace( shunkTrace );
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

void QgsMeshStreamField::initField()
{
  mField = QVector<bool>( mFieldSize.width() * mFieldSize.height(), false );

  if ( mPainter )
    delete mPainter;

  mTraceImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mTraceImage.fill( 0X00000000 );

  mPainter = new QPainter( &mTraceImage );
  mPainter->setRenderHint( QPainter::Antialiasing, true );
  mPainter->setPen( mPen );
}

void QgsMeshStreamField::storeInField( const QPoint &pixel )
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    mField[j * mFieldSize.width() + i] = true;
  }
}

void QgsMeshStreamField::drawShunkTrace( std::list<QPair<QPoint, double> > &shunkTrace )
{
  auto p1 = shunkTrace.begin();
  auto p2 = p1;
  p2++;
  while ( p2 != shunkTrace.end() )
  {
    storeInField( ( *p2 ).first );
    if ( filterMag( ( *p1 ).second ) && filterMag( ( *p2 ).second ) )
      mPainter->drawLine( fieldToDevice( ( *p1 ).first ), fieldToDevice( ( *p2 ).first ) );
    mPixelFillingCount++;
    auto p = p1;
    p1++;
    p2++;
    shunkTrace.erase( p );

  }
}

void QgsMeshStreamField::simplifyShunkTrace( std::list<QPair<QPoint, double> > &shunkTrace )
{
  Q_ASSERT( shunkTrace.size() == 3 );

  QPoint p1 = shunkTrace.front().first;
  auto ip2 = shunkTrace.begin();
  ip2++;
  QPoint p2 = ( *( ip2 ) ).first;
  QPoint p3 = shunkTrace.back().first;

  QPoint v1 = p1 - p2;
  QPoint v2 = p2 - p3;

  if ( v1.x()*v2.x() + v1.y()*v2.y() == 0 )
    shunkTrace.erase( ip2 );
}

bool QgsMeshStreamField::isTraceExists( const QPoint &pixel ) const
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

QgsMeshVectorStreamLineRenderer::QgsMeshVectorStreamLineRenderer( const QgsTriangularMesh &triangularMesh, const QgsMeshDataBlock &dataSetVectorValues, const QgsMeshDataBlock &scalarActiveFaceFlagValues, bool dataIsOnVertices, const QgsMeshRendererVectorSettings &settings, QgsRenderContext &rendererContext, const QgsRectangle &layerExtent, double magMax ):
  mRendererContext( rendererContext )
{
  mStreamLineField.reset( new QgsMeshStreamField( triangularMesh,
                          dataSetVectorValues,
                          scalarActiveFaceFlagValues,
                          layerExtent,
                          magMax, dataIsOnVertices, rendererContext ) );

  mStreamLineField->updateSize( rendererContext );
  mStreamLineField->setPixelFillingDensity( settings.streamLinesSettings().seedingDensity() );
  mStreamLineField->setLineWidth(
    rendererContext.convertToPainterUnits( settings.lineWidth(), QgsUnitTypes::RenderUnit::RenderMillimeters ) );

  mStreamLineField->setColor( settings.color() );
  mStreamLineField->setFilter( settings.filterMin(), settings.filterMax() );


  switch ( settings.streamLinesSettings().seedingMethod() )
  {
    case QgsMeshRendererVectorStreamlineSettings::MeshGridded:
      if ( settings.isOnUserDefinedGrid() )
        mStreamLineField->addGriddedTraces( settings.userGridCellWidth(), settings.userGridCellHeight() );
      else
        mStreamLineField->addTracesOnMesh( triangularMesh, rendererContext.mapExtent() );
      break;
    case QgsMeshRendererVectorStreamlineSettings::Random:
      mStreamLineField->addRandomTraces();
      break;
  }
}

void QgsMeshVectorStreamLineRenderer::draw()
{
  if ( mRendererContext.renderingStopped() )
    return;
  mRendererContext.painter()->drawImage( mStreamLineField->topLeft(), mStreamLineField->image() );
}

///@endcond
