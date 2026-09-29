/***************************************************************************
    qgsvectorfieldstreamfield.cpp
    -----------------------------
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

#include "qgsvectorfieldstreamfield.h"

#include <cmath>

#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsrasterblock.h"
#include "qgsrasterinterface.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"

///@cond PRIVATE

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

//! Returns the device coordinates bounding box of a map \a bbox, as transformed by \a mtp
static QgsRectangle boundingBoxToScreenRectangle( const QgsMapToPixel &mtp, const QgsRectangle &bbox )
{
  const QgsPointXY topLeft = mtp.transform( bbox.xMinimum(), bbox.yMaximum() );
  const QgsPointXY topRight = mtp.transform( bbox.xMaximum(), bbox.yMaximum() );
  const QgsPointXY bottomLeft = mtp.transform( bbox.xMinimum(), bbox.yMinimum() );
  const QgsPointXY bottomRight = mtp.transform( bbox.xMaximum(), bbox.yMinimum() );

  const double xMin = std::min( { topLeft.x(), topRight.x(), bottomLeft.x(), bottomRight.x() } );
  const double xMax = std::max( { topLeft.x(), topRight.x(), bottomLeft.x(), bottomRight.x() } );
  const double yMin = std::min( { topLeft.y(), topRight.y(), bottomLeft.y(), bottomRight.y() } );
  const double yMax = std::max( { topLeft.y(), topRight.y(), bottomLeft.y(), bottomRight.y() } );

  return QgsRectangle( xMin, yMin, xMax, yMax );
}

QSize QgsVectorFieldStreamField::size() const
{
  return mFieldSize;
}

QPoint QgsVectorFieldStreamField::topLeft() const
{
  return mFieldTopLeftInDeviceCoordinates;
}

int QgsVectorFieldStreamField::resolution() const
{
  return mFieldResolution;
}

QgsPointXY QgsVectorFieldStreamField::positionToMapCoordinates( const QPoint &pixelPosition, const QgsPointXY &positionInPixel )
{
  QgsPointXY mapPoint = mMapToFieldPixel.toMapCoordinates( pixelPosition );
  mapPoint = mapPoint + QgsVector( positionInPixel.x() * mMapToFieldPixel.mapUnitsPerPixel(), positionInPixel.y() * mMapToFieldPixel.mapUnitsPerPixel() );
  return mapPoint;
}

QgsVectorFieldStreamField::QgsVectorFieldStreamField( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring, int resolution )
  : mFieldResolution( resolution )
  , mVectorColoring( vectorColoring )
  , mRenderContext( rendererContext )
  , mSource( std::move( source ) )
{}

QgsVectorFieldStreamField::QgsVectorFieldStreamField( const QgsVectorFieldStreamField &other )
  : mFieldSize( other.mFieldSize )
  , mFieldResolution( other.mFieldResolution )
  , mPen( other.mPen )
  , mTraceImage( other.mTraceImage )
  , mMapToFieldPixel( other.mMapToFieldPixel )
  , mOutputExtent( other.mOutputExtent )
  , mVectorColoring( other.mVectorColoring )
  , mDirectionField( other.mDirectionField )
  , mRenderContext( other.mRenderContext )
  , mPixelFillingCount( other.mPixelFillingCount )
  , mMaxPixelFillingCount( other.mMaxPixelFillingCount )
  , mMapExtent( other.mMapExtent )
  , mFieldTopLeftInDeviceCoordinates( other.mFieldTopLeftInDeviceCoordinates )
  , mValid( other.mValid )
  , mPixelFillingDensity( other.mPixelFillingDensity )
  , mMinMagFilter( other.mMinMagFilter )
  , mMaxMagFilter( other.mMaxMagFilter )
  , mMinimizeFieldSize( other.mMinimizeFieldSize )
{
  mPainter = std::make_unique<QPainter>( &mTraceImage );
  mSource = other.mSource ? std::unique_ptr<QgsVectorFieldValueSource>( other.mSource->clone() ) : nullptr;
}

QgsVectorFieldStreamField::~QgsVectorFieldStreamField()
{
  if ( mPainter )
    mPainter->end();
}

void QgsVectorFieldStreamField::updateSize( const QgsRenderContext &renderContext )
{
  mMapExtent = renderContext.mapExtent();
  const QgsMapToPixel &deviceMapToPixel = renderContext.mapToPixel();
  QgsRectangle layerExtent;
  try
  {
    QgsCoordinateTransform extentTransform = renderContext.coordinateTransform();
    extentTransform.setBallparkTransformsAreAppropriate( true );
    layerExtent = extentTransform.transformBoundingBox( mSource->extent() );
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

  QgsRectangle fieldInterestZoneInDeviceCoordinates = ::boundingBoxToScreenRectangle( deviceMapToPixel, interestZoneExtent );
  mFieldTopLeftInDeviceCoordinates
    = QPoint( static_cast<int>( std::round( fieldInterestZoneInDeviceCoordinates.xMinimum() ) ), static_cast<int>( std::round( fieldInterestZoneInDeviceCoordinates.yMinimum() ) ) );
  int fieldWidthInDeviceCoordinate = int( fieldInterestZoneInDeviceCoordinates.width() );
  int fieldHeightInDeviceCoordinate = int( fieldInterestZoneInDeviceCoordinates.height() );

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
    mOutputExtent = QgsRectangle();
  }
  else
  {
    mFieldSize.setWidth( fieldWidth );
    mFieldSize.setHeight( fieldHeight );
    QgsPointXY pt1 = deviceMapToPixel.toMapCoordinates( mFieldTopLeftInDeviceCoordinates );
    QgsPointXY pt2 = deviceMapToPixel.toMapCoordinates( mFieldTopLeftInDeviceCoordinates + QPoint( fieldWidth, fieldHeight ) );
    QgsPointXY pt3 = deviceMapToPixel.toMapCoordinates( mFieldTopLeftInDeviceCoordinates + QPoint( 0, fieldHeight ) );
    QgsPointXY pt4 = deviceMapToPixel.toMapCoordinates( mFieldTopLeftInDeviceCoordinates + QPoint( fieldWidth, 0 ) );

    mOutputExtent = QgsRectangle(
      std::min( { pt1.x(), pt2.x(), pt3.x(), pt4.x() } ),
      std::min( { pt1.y(), pt2.y(), pt3.y(), pt4.y() } ),
      std::max( { pt1.x(), pt2.x(), pt3.x(), pt4.x() } ),
      std::max( { pt1.y(), pt2.y(), pt3.y(), pt4.y() } ),
      true
    );
  }

  double mapUnitPerFieldPixel;
  if ( interestZoneExtent.width() > 0 )
    mapUnitPerFieldPixel = deviceMapToPixel.mapUnitsPerPixel() * mFieldResolution * mFieldSize.width() / ( fieldWidthInDeviceCoordinate / static_cast<double>( mFieldResolution ) );
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

  mMapToFieldPixel = QgsMapToPixel( mapUnitPerFieldPixel, xc, yc, fieldWidth, fieldHeight, deviceMapToPixel.mapRotation() );

  initField();
  mValid = true;
}

void QgsVectorFieldStreamField::updateSize( const QgsRenderContext &renderContext, int resolution )
{
  if ( renderContext.mapExtent() == mMapExtent && resolution == mFieldResolution )
    return;
  mFieldResolution = resolution;

  updateSize( renderContext );
}

bool QgsVectorFieldStreamField::isValid() const
{
  return mValid;
}

void QgsVectorFieldStreamField::addTrace( QgsPointXY startPoint )
{
  addTrace( mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint() );
}


void QgsVectorFieldStreamField::addRandomTraces()
{
  if ( mSource && mSource->maximumMagnitude() > 0 )
    while ( ( mPixelFillingCount < mMaxPixelFillingCount ) && ( !mRenderContext.feedback() || !mRenderContext.feedback()->isCanceled() || !mRenderContext.renderingStopped() ) )
      addRandomTrace();
}

void QgsVectorFieldStreamField::addRandomTrace()
{
  if ( !mValid )
    return;

  int xRandom = 1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.width() ) );
  int yRandom = 1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.height() ) );
  addTrace( QPoint( xRandom, yRandom ) );
}

void QgsVectorFieldStreamField::addGriddedTraces( int dx, int dy )
{
  int i = 0;
  while ( i < mFieldSize.width() && mRenderContext.feedback() && !mRenderContext.feedback()->isCanceled() )
  {
    int j = 0;
    while ( j < mFieldSize.height() && mRenderContext.feedback() && !mRenderContext.feedback()->isCanceled() )
    {
      addTrace( QPoint( i, j ) );
      j += dy;
    }
    i += dx;
  }
}

void QgsVectorFieldStreamField::addTracesOnDataPoints( const QgsRectangle &extent )
{
  if ( !mSource )
    return;

  const QVector<QgsPointXY> points = mSource->seedPoints( extent );
  for ( const QgsPointXY &point : points )
    addTrace( point );
}

void QgsVectorFieldStreamField::addTrace( QPoint startPixel )
{
  //This is where each traces are constructed
  if ( !mPainter )
    return;

  if ( isTraceExists( startPixel ) || isTraceOutside( startPixel ) )
    return;

  if ( !mSource )
    return;

  const double maximumMagnitude = mSource->maximumMagnitude();
  if ( !( maximumMagnitude > 0 ) )
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

  while ( true )
  {
    QgsPointXY mapPosition = positionToMapCoordinates( currentPixel, QgsPointXY( x1, y1 ) );
    vector = mSource->vectorValue( mapPosition );

    if ( std::isnan( vector.x() ) || std::isnan( vector.y() ) )
    {
      mPixelFillingCount++;
      setChunkTrace( chunkTrace );
      break;
    }

    /* nondimensional value :  Vu=2 when the particle need dt=1 to go through a pixel with the mMagMax magnitude
     * The nondimensional size of the side of a pixel is 2
     */
    vector = vector.rotateBy( -mMapToFieldPixel.mapRotation() * M_DEG2RAD );
    QgsVector vu = vector / maximumMagnitude * 2;
    data.magnitude = vector.length();

    double Vx = vu.x();
    double Vy = vu.y();
    double Vu = data.magnitude / maximumMagnitude * 2; //nondimensional vector magnitude

    if ( qgsDoubleNear( Vu, 0 ) )
    {
      // no trace anymore
      addPixelToChunkTrace( currentPixel, data, chunkTrace );
      simplifyChunkTrace( chunkTrace );
      setChunkTrace( chunkTrace );
      break;
    }

    //calculates where the particle will be after dt=1,
    QgsPointXY nextPosition = QgsPointXY( x1, y1 ) + vu;
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
        clearChunkTrace( chunkTrace );
      }

      data.time = 1;
      currentPixel += QPoint( incX, -incY );
      x1 = nextPosition.x() - 2 * incX;
      y1 = nextPosition.y() - 2 * incY;
    }
    else
    {
      double x2, y2;
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

        x2 = incX;
      }
      else if ( qgsDoubleNear( Vx, 0 ) )
      {
        x2 = x1;
        if ( Vy > 0 )
          incY = +1;
        else
          incY = -1;

        y2 = incY;
      }
      else
      {
        if ( Vy > 0 )
          x2 = x1 + ( 1 - y1 ) * Vx / fabs( Vy );
        else
          x2 = x1 + ( 1 + y1 ) * Vx / fabs( Vy );
        if ( Vx > 0 )
          y2 = y1 + ( 1 - x1 ) * Vy / fabs( Vx );
        else
          y2 = y1 + ( 1 + x1 ) * Vy / fabs( Vx );

        if ( x2 >= 1 )
          x2 = 1;

        if ( x2 <= -1 )
          x2 = -1;

        if ( y2 >= 1 )
          y2 = 1;

        if ( y2 <= -1 )
          y2 = -1;
      }

      //calculate distance
      double dx = x2 - x1;
      double dy = y2 - y1;
      double dl = sqrt( dx * dx + dy * dy );

      data.time += static_cast<float>( dl / Vu ); //adimensional time step : this the time needed to go to the border of the pixel
      if ( data.time > 10000 )                    //Guard to prevent that the particle never leave the pixel
      {
        addPixelToChunkTrace( currentPixel, data, chunkTrace );
        setChunkTrace( chunkTrace );
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
      break;
    }

    if ( isTraceOutside( currentPixel ) )
    {
      setChunkTrace( chunkTrace );
      break;
    }

    if ( mRenderContext.feedback() && mRenderContext.feedback()->isCanceled() )
      break;

    if ( mRenderContext.renderingStopped() )
      break;
  }

  drawTrace( startPixel );
}

void QgsVectorFieldStreamField::setResolution( int width )
{
  mFieldResolution = width;
}

QSize QgsVectorFieldStreamField::imageSize() const
{
  return mFieldSize * mFieldResolution;
}

QPointF QgsVectorFieldStreamField::fieldToDevice( const QPoint &pixel ) const
{
  QPointF p( pixel );
  p = mFieldResolution * p + QPointF( mFieldResolution - 1, mFieldResolution - 1 ) / 2;
  return p;
}

bool QgsVectorFieldStreamField::addPixelToChunkTrace( QPoint &pixel, QgsVectorFieldStreamField::FieldData &data, std::list<QPair<QPoint, QgsVectorFieldStreamField::FieldData> > &chunkTrace )
{
  chunkTrace.emplace_back( pixel, data );
  if ( chunkTrace.size() == 3 )
  {
    simplifyChunkTrace( chunkTrace );
    return true;
  }
  return false;
}

void QgsVectorFieldStreamlinesField::initField()
{
  mField = QVector<bool>( mFieldSize.width() * mFieldSize.height(), false );
  mDirectionField = QVector<unsigned char>( mFieldSize.width() * mFieldSize.height(), static_cast<unsigned char>( int( 0 ) ) );
  initImage();
}

void QgsVectorFieldStreamlinesField::initImage()
{
  mTraceImage = QImage();
  switch ( mVectorColoring.coloringMethod() )
  {
    case QgsInterpolatedLineColor::ColorRamp:
    {
      QSize imgSize = mFieldSize * mFieldResolution;
      QgsRenderContext fieldContext = mRenderContext;

      fieldContext.setMapToPixel( mMapToFieldPixel );
      // the returned interface keeps a reference on fieldContext, so it must not outlive this scope
      std::unique_ptr<QgsRasterInterface> magnitudeSource = mSource ? mSource->magnitudeSource( fieldContext, imgSize ) : nullptr;

      if ( magnitudeSource && imgSize.isValid() )
      {
        QgsRasterShader *sh = new QgsRasterShader();
        sh->setRasterShaderFunction( new QgsColorRampShader( mVectorColoring.colorRampShader() ) ); // takes ownership of fcn
        QgsSingleBandPseudoColorRenderer renderer( magnitudeSource.get(), 0, sh );                  // takes ownership of sh
        std::unique_ptr<QgsRasterBlock> bl( renderer.block( 0, mOutputExtent, imgSize.width(), imgSize.height(), mFeedBack ) );
        mTraceImage = bl->image();
      }
      else
      {
        // the source cannot provide a magnitude raster, degrade to a flat single color
        mTraceImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32_Premultiplied );
        if ( !mTraceImage.isNull() )
          mTraceImage.fill( mVectorColoring.singleColor() );
      }
    }
    break;
    case QgsInterpolatedLineColor::SingleColor:
    {
      mTraceImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32_Premultiplied );
      QColor col = mVectorColoring.singleColor();
      mTraceImage.fill( col );
    }
    break;
  }

  if ( !mTraceImage.isNull() )
  {
    mPainter = std::make_unique<QPainter>( &mTraceImage );
    mPainter->setRenderHint( QPainter::Antialiasing, true );

    mDrawingTraceImage = QImage( mTraceImage.size(), QImage::Format_ARGB32_Premultiplied );
    mDrawingTraceImage.fill( Qt::transparent );
    mDrawingTracePainter = std::make_unique<QPainter>( &mDrawingTraceImage );
    mDrawingTracePainter->setRenderHint( QPainter::Antialiasing, true );
  }
}

void QgsVectorFieldStreamField::clearChunkTrace( std::list<QPair<QPoint, QgsVectorFieldStreamField::FieldData> > &chunkTrace )
{
  auto one_before_end = std::prev( chunkTrace.end() );
  chunkTrace.erase( chunkTrace.begin(), one_before_end );
}

void QgsVectorFieldStreamField::simplifyChunkTrace( std::list<QPair<QPoint, FieldData> > &chunkTrace )
{
  if ( chunkTrace.size() != 3 )
    return;

  auto ip3 = chunkTrace.begin();
  auto ip1 = ip3++;
  auto ip2 = ip3++;

  while ( ip3 != chunkTrace.end() && ip2 != chunkTrace.end() )
  {
    QPoint v1 = ( *ip1 ).first - ( *ip2 ).first;
    QPoint v2 = ( *ip2 ).first - ( *ip3 ).first;
    if ( v1.x() * v2.x() + v1.y() * v2.y() == 0 )
    {
      ( *ip1 ).second.time += ( ( *ip2 ).second.time ) / 2;
      ( *ip3 ).second.time += ( ( *ip2 ).second.time ) / 2;
      ( *ip1 ).second.directionX += ( *ip2 ).second.directionX;
      ( *ip1 ).second.directionY += ( *ip2 ).second.directionY;
      chunkTrace.erase( ip2 );
    }
    ip1 = ip3++;
    ip2 = ip3++;
  }
}

QgsVectorFieldStreamlinesField::QgsVectorFieldStreamlinesField(
  std::unique_ptr<QgsVectorFieldValueSource> source, QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring, QgsRasterBlockFeedback *feedBack
)
  : QgsVectorFieldStreamField( std::move( source ), rendererContext, vectorColoring )
  , mFeedBack( feedBack )
{}

void QgsVectorFieldStreamlinesField::compose()
{
  if ( !mPainter )
    return;
  mPainter->setCompositionMode( QPainter::CompositionMode_DestinationIn );
  mPainter->drawImage( 0, 0, mDrawingTraceImage );
}

void QgsVectorFieldStreamlinesField::storeInField( const QPair<QPoint, FieldData> pixelData )
{
  int i = pixelData.first.x();
  int j = pixelData.first.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    mField[j * mFieldSize.width() + i] = true;
    int d = pixelData.second.directionX + 2 + ( pixelData.second.directionY + 1 ) * 3;
    mDirectionField[j * mFieldSize.width() + i] = static_cast<unsigned char>( d );
  }
}

void QgsVectorFieldStreamField::setChunkTrace( std::list<QPair<QPoint, FieldData> > &chunkTrace )
{
  auto p = chunkTrace.begin();
  while ( p != chunkTrace.end() )
  {
    storeInField( ( *p ) );
    mPixelFillingCount++;
    ++p;
  }
}

void QgsVectorFieldStreamlinesField::drawTrace( const QPoint &start ) const
{
  if ( !isTraceExists( start ) || isTraceOutside( start ) )
    return;

  if ( !mDrawingTracePainter )
    return;

  QPoint pt1 = start;
  QPoint curPt = pt1;
  int fieldWidth = mFieldSize.width();
  QSet<QgsPointXY> path;
  unsigned char dir = 0;
  unsigned char prevDir = mDirectionField.at( pt1.y() * fieldWidth + pt1.x() );

  QVector<double> xPoly;
  QVector<double> yPoly;
  QPointF devicePt = fieldToDevice( pt1 );
  xPoly.append( devicePt.x() );
  yPoly.append( devicePt.y() );

  while ( isTraceExists( curPt ) && !isTraceOutside( curPt ) && !path.contains( curPt ) )
  {
    dir = mDirectionField.at( curPt.y() * fieldWidth + curPt.x() );
    if ( dir == 5 ) //no direction, static pixel
      break;

    const QPoint curPtDir( ( dir - 1 ) % 3 - 1, ( dir - 1 ) / 3 - 1 );
    const QPoint pt2 = curPt + curPtDir;

    if ( dir != prevDir )
    {
      path.insert( curPt );
      devicePt = fieldToDevice( curPt );
      xPoly.append( devicePt.x() );
      yPoly.append( devicePt.y() );
      prevDir = dir;
    }
    curPt = pt2;
  }

  if ( !isTraceExists( curPt ) || isTraceOutside( curPt ) )
  {
    // just add the last point
    devicePt = fieldToDevice( curPt - QPoint( ( dir - 1 ) % 3 - 1, ( dir - 1 ) / 3 - 1 ) );
    xPoly.append( devicePt.x() );
    yPoly.append( devicePt.y() );
  }

  QgsGeometry geom( new QgsLineString( xPoly, yPoly ) );
  geom = geom.simplify( 1.5 * mFieldResolution ).smooth( 1, 0.25, -1.0, 45 );
  QPen pen = mPen;
  pen.setColor( QColor( 0, 0, 0, 255 ) );
  mDrawingTracePainter->setPen( pen );
  mDrawingTracePainter->drawPolyline( geom.asQPolygonF() );
}

bool QgsVectorFieldStreamlinesField::isTraceExists( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mField[j * mFieldSize.width() + i];
  }

  return false;
}

bool QgsVectorFieldStreamField::isTraceOutside( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();

  return !( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() );
}

void QgsVectorFieldStreamField::setMinimizeFieldSize( bool minimizeFieldSize )
{
  mMinimizeFieldSize = minimizeFieldSize;
}

QgsVectorFieldStreamField &QgsVectorFieldStreamField::operator=( const QgsVectorFieldStreamField &other )
{
  if ( &other == this )
    return *this;

  mFieldSize = other.mFieldSize;
  mFieldResolution = other.mFieldResolution;
  mPen = other.mPen;
  mTraceImage = other.mTraceImage;
  mMapToFieldPixel = other.mMapToFieldPixel;
  mOutputExtent = other.mOutputExtent;
  mVectorColoring = other.mVectorColoring;
  mDirectionField = other.mDirectionField;
  mRenderContext = other.mRenderContext;
  mPixelFillingCount = other.mPixelFillingCount;
  mMaxPixelFillingCount = other.mMaxPixelFillingCount;
  mMapExtent = other.mMapExtent;
  mFieldTopLeftInDeviceCoordinates = other.mFieldTopLeftInDeviceCoordinates;
  mValid = other.mValid;
  mPixelFillingDensity = other.mPixelFillingDensity;
  mMinMagFilter = other.mMinMagFilter;
  mMaxMagFilter = other.mMaxMagFilter;
  mMinimizeFieldSize = other.mMinimizeFieldSize;
  mSource = other.mSource ? std::unique_ptr<QgsVectorFieldValueSource>( other.mSource->clone() ) : nullptr;

  mPainter = std::make_unique<QPainter>( &mTraceImage );

  return ( *this );
}

void QgsVectorFieldStreamField::initImage()
{
  mTraceImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  if ( !mTraceImage.isNull() )
  {
    mTraceImage.fill( 0X00000000 );
    mPainter = std::make_unique<QPainter>( &mTraceImage );
    mPainter->setRenderHint( QPainter::Antialiasing, true );
    mPainter->setPen( mPen );
  }
}

bool QgsVectorFieldStreamField::filterMag( double value ) const
{
  return ( mMinMagFilter < 0 || value > mMinMagFilter ) && ( mMaxMagFilter < 0 || value < mMaxMagFilter );
}

QImage QgsVectorFieldStreamField::image() const
{
  if ( mTraceImage.isNull() )
    return QImage();
  return mTraceImage.scaled( mFieldSize * mFieldResolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

void QgsVectorFieldStreamField::setPixelFillingDensity( double maxFilling )
{
  mPixelFillingDensity = maxFilling;
  mMaxPixelFillingCount = int( mPixelFillingDensity * mFieldSize.width() * mFieldSize.height() );
}

void QgsVectorFieldStreamField::setColor( QColor color )
{
  mPen.setColor( color );
}

void QgsVectorFieldStreamField::setLineWidth( double width )
{
  mPen.setWidthF( width );
}

void QgsVectorFieldStreamField::setFilter( double min, double max )
{
  mMinMagFilter = min;
  mMaxMagFilter = max;
}

QgsVectorFieldParticleTracesField::QgsVectorFieldParticleTracesField( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsInterpolatedLineColor &vectorColoring )
  : QgsVectorFieldStreamField( std::move( source ), rendererContext, vectorColoring )
{
  std::srand( uint( ::time( nullptr ) ) );
  mPen.setCapStyle( Qt::RoundCap );
}

QgsVectorFieldParticleTracesField::QgsVectorFieldParticleTracesField( const QgsVectorFieldParticleTracesField &other )
  : QgsVectorFieldStreamField( other )
  , mTimeField( other.mTimeField )
  , mMagnitudeField( other.mMagnitudeField )
  , mParticles( other.mParticles )
  , mStumpImage( other.mStumpImage )
  , mTimeStep( other.mTimeStep )
  , mParticlesLifeTime( other.mParticlesLifeTime )
  , mParticlesCount( other.mParticlesCount )
  , mTailFactor( other.mTailFactor )
  , mMinTailLength( other.mMinTailLength )
  , mParticleColor( other.mParticleColor )
  , mParticleSize( other.mParticleSize )
  , mStumpFactor( other.mStumpFactor )
  , mStumpParticleWithLifeTime( other.mStumpParticleWithLifeTime )
{}

void QgsVectorFieldParticleTracesField::addParticle( const QPoint &startPoint, double lifeTime )
{
  addTrace( startPoint );
  if ( time( startPoint ) > 0 )
  {
    QgsVectorFieldTraceParticle p;
    p.lifeTime = lifeTime;
    p.position = startPoint;
    mParticles.append( p );
  }
}

void QgsVectorFieldParticleTracesField::addParticleXY( const QgsPointXY &startPoint, double lifeTime )
{
  addParticle( mMapToFieldPixel.transform( startPoint ).toQPointF().toPoint(), lifeTime );
}

void QgsVectorFieldParticleTracesField::moveParticles()
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
      while ( static_cast<int>( p.tail.size() ) > mMinTailLength && static_cast<double>( p.tail.size() ) > ( static_cast<double>( countAdded ) * mTailFactor ) )
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

void QgsVectorFieldParticleTracesField::addRandomParticles()
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
    int xRandom = 1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.width() ) );
    int yRandom = 1 + std::rand() / int( ( RAND_MAX + 1u ) / uint( mFieldSize.height() ) );
    double lifeTime = ( std::rand() / ( ( RAND_MAX + 1u ) / mParticlesLifeTime ) );
    addParticle( QPoint( xRandom, yRandom ), lifeTime );
  }
}

void QgsVectorFieldParticleTracesField::storeInField( const QPair<QPoint, QgsVectorFieldStreamField::FieldData> pixelData )
{
  int i = pixelData.first.x();
  int j = pixelData.first.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    mTimeField[j * mFieldSize.width() + i] = pixelData.second.time;
    int d = pixelData.second.directionX + 2 + ( pixelData.second.directionY + 1 ) * 3;
    mDirectionField[j * mFieldSize.width() + i] = static_cast<unsigned char>( d );
    mMagnitudeField[j * mFieldSize.width() + i] = static_cast<float>( pixelData.second.magnitude );
  }
}

void QgsVectorFieldParticleTracesField::initField()
{
  mTimeField = QVector<float>( mFieldSize.width() * mFieldSize.height(), -1 );
  mDirectionField = QVector<unsigned char>( mFieldSize.width() * mFieldSize.height(), static_cast<unsigned char>( int( 0 ) ) );
  mMagnitudeField = QVector<float>( mFieldSize.width() * mFieldSize.height(), 0 );
  initImage();
  mStumpImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mStumpImage.fill( QColor( 0, 0, 0, mStumpFactor ) ); //alpha=0 -> no persitence, alpha=255 -> total persistence
}

bool QgsVectorFieldParticleTracesField::isTraceExists( const QPoint &pixel ) const
{
  int i = pixel.x();
  int j = pixel.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mTimeField[j * mFieldSize.width() + i] >= 0;
  }

  return false;
}

void QgsVectorFieldParticleTracesField::setStumpParticleWithLifeTime( bool stumpParticleWithLifeTime )
{
  mStumpParticleWithLifeTime = stumpParticleWithLifeTime;
}

void QgsVectorFieldParticleTracesField::setParticlesColor( const QColor &c )
{
  mVectorColoring.setColor( c );
}

QgsVectorFieldParticleTracesField &QgsVectorFieldParticleTracesField::operator=( const QgsVectorFieldParticleTracesField &other )
{
  if ( &other == this )
    return *this;

  QgsVectorFieldStreamField::operator=( other );
  mTimeField = other.mTimeField;
  mMagnitudeField = other.mMagnitudeField;
  mDirectionField = other.mDirectionField;
  mParticles = other.mParticles;
  mStumpImage = other.mStumpImage;
  mTimeStep = other.mTimeStep;
  mParticlesLifeTime = other.mParticlesLifeTime;
  mParticlesCount = other.mParticlesCount;
  mMinTailLength = other.mMinTailLength;
  mTailFactor = other.mTailFactor;
  mParticleColor = other.mParticleColor;
  mParticleSize = other.mParticleSize;
  mStumpFactor = other.mStumpFactor;
  mStumpParticleWithLifeTime = other.mStumpParticleWithLifeTime;

  return ( *this );
}

void QgsVectorFieldParticleTracesField::setMinTailLength( int minTailLength )
{
  mMinTailLength = minTailLength;
}

void QgsVectorFieldParticleTracesField::setTailFactor( double tailFactor )
{
  mTailFactor = tailFactor;
}

void QgsVectorFieldParticleTracesField::setParticleSize( double particleSize )
{
  mParticleSize = particleSize;
}

void QgsVectorFieldParticleTracesField::setTimeStep( double timeStep )
{
  mTimeStep = timeStep;
}

void QgsVectorFieldParticleTracesField::setParticlesLifeTime( double particlesLifeTime )
{
  mParticlesLifeTime = particlesLifeTime;
}

QImage QgsVectorFieldParticleTracesField::imageRendered() const
{
  return mTraceImage;
}

void QgsVectorFieldParticleTracesField::stump()
{
  if ( !mPainter )
    return;
  QgsScopedQPainterState painterState( mPainter.get() );
  mPainter->setCompositionMode( QPainter::CompositionMode_DestinationIn );
  mPainter->drawImage( QPoint( 0, 0 ), mStumpImage );
}

void QgsVectorFieldParticleTracesField::setStumpFactor( int sf )
{
  mStumpFactor = sf;
  mStumpImage = QImage( mFieldSize * mFieldResolution, QImage::Format_ARGB32 );
  mStumpImage.fill( QColor( 0, 0, 0, mStumpFactor ) );
}

QPoint QgsVectorFieldParticleTracesField::direction( QPoint position ) const
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

float QgsVectorFieldParticleTracesField::time( QPoint position ) const
{
  int i = position.x();
  int j = position.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mTimeField[j * mFieldSize.width() + i];
  }
  return -1;
}

float QgsVectorFieldParticleTracesField::magnitude( QPoint position ) const
{
  int i = position.x();
  int j = position.y();
  if ( i >= 0 && i < mFieldSize.width() && j >= 0 && j < mFieldSize.height() )
  {
    return mMagnitudeField[j * mFieldSize.width() + i];
  }
  return -1;
}

void QgsVectorFieldParticleTracesField::drawParticleTrace( const QgsVectorFieldTraceParticle &particle )
{
  if ( !mPainter )
    return;
  const std::list<QPoint> &tail = particle.tail;
  if ( tail.size() == 0 )
    return;
  double iniWidth = mParticleSize;

  size_t pixelCount = tail.size();

  double transparency = 1;
  if ( mStumpParticleWithLifeTime )
    transparency = sin( M_PI * particle.lifeTime / mParticlesLifeTime );

  double dw;
  if ( pixelCount > 1 )
    dw = iniWidth / static_cast<double>( pixelCount );
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
    traceColor.setAlphaF( traceColor.alphaF() * transparency );
    mPen.setColor( traceColor );
    mPen.setWidthF( iniWidth - i * dw );
    mPainter->setPen( mPen );
    mPainter->drawLine( p1, p2 );
    ip1--;
    ip2--;
    ++i;
  }
}

void QgsVectorFieldParticleTracesField::setParticlesCount( int particlesCount )
{
  mParticlesCount = particlesCount;
}

///@endcond
