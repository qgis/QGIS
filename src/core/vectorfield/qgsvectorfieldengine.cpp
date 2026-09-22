/***************************************************************************
    qgsvectorfieldengine.cpp
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldengine.h"

#include <algorithm>
#include <cmath>

#include "qgsfeedback.h"
#include "qgsrendercontext.h"
#include "qgsvectorfieldstreamfield.h"
#include "qgsvectorfieldvaluesource.h"

#include <QString>

using namespace Qt::StringLiterals;

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

//! Upper bound on the number of glyphs drawn in a single render
constexpr qint64 VECTOR_FIELD_MAXIMUM_GLYPHS = 100'000;

QgsVectorFieldEngine::QgsVectorFieldEngine( double datasetMagMaximumValue, double datasetMagMinimumValue, const QgsVectorFieldSettings &settings, QgsRenderContext &context, QSize size )
  : mMinMag( datasetMagMinimumValue )
  , mMaxMag( datasetMagMaximumValue )
  , mContext( context )
  , mCfg( settings )
  , mVectorColoring( settings.vectorStrokeColoring() )
  , mOutputSize( size )
{
  switch ( settings.symbology() )
  {
    case Qgis::VectorFieldSymbology::WindBarbs:
    {
      const QgsCoordinateReferenceSystem mapCrs = mContext.distanceArea().sourceCrs();
      mGeographicTransform = std::make_unique<QgsCoordinateTransform>( mapCrs, mapCrs.toGeographicCrs(), mContext.transformContext() );
      break;
    }
    case Qgis::VectorFieldSymbology::Arrows:
    case Qgis::VectorFieldSymbology::Streamlines:
    case Qgis::VectorFieldSymbology::Traces:
      break;
  }

  // Set up the render configuration options
  QPainter *painter = mContext.painter();

  mScopedPainterState = std::make_unique<QgsScopedQPainterState>( painter );
  mContext.setPainterFlagsUsingContext( painter );

  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  const double penWidth = mContext.convertToPainterUnits( mCfg.lineWidth(), Qgis::RenderUnit::Millimeters );
  pen.setWidthF( penWidth );
  painter->setPen( pen );
}

QgsVectorFieldEngine::~QgsVectorFieldEngine() = default;

double QgsVectorFieldEngine::glyphExtentBuffer() const
{
  double buffer = 0;
  switch ( mCfg.symbology() )
  {
    case Qgis::VectorFieldSymbology::WindBarbs:
      buffer = mContext.convertToPainterUnits( mCfg.windBarbSettings().shaftLength(), mCfg.windBarbSettings().shaftLengthUnits() );
      break;

    case Qgis::VectorFieldSymbology::Arrows:
      switch ( mCfg.arrowSettings().shaftLengthMethod() )
      {
        case Qgis::VectorFieldArrowScalingMethod::MinMax:
          buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().maxShaftLength(), Qgis::RenderUnit::Millimeters );
          break;

        case Qgis::VectorFieldArrowScalingMethod::Scaled:
        {
          // the shaft length follows the magnitude, so the longest shaft which is actually drawn is
          // the one of the largest magnitude which passes the filter
          const double magnitude = mCfg.filterMax() >= 0 ? std::min( mMaxMag, mCfg.filterMax() ) : mMaxMag;
          buffer = mCfg.arrowSettings().scaleFactor() * magnitude;
          break;
        }

        case Qgis::VectorFieldArrowScalingMethod::Fixed:
          buffer = mContext.convertToPainterUnits( mCfg.arrowSettings().fixedShaftLength(), Qgis::RenderUnit::Millimeters );
          break;
      }
      break;

    case Qgis::VectorFieldSymbology::Streamlines:
    case Qgis::VectorFieldSymbology::Traces:
      break;
  }

  return std::max( 0.0, buffer );
}

void QgsVectorFieldEngine::drawGlyph( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude )
{
  switch ( mCfg.symbology() )
  {
    case Qgis::VectorFieldSymbology::Arrows:
      drawArrow( lineStart, xVal, yVal, magnitude );
      break;
    case Qgis::VectorFieldSymbology::WindBarbs:
      drawWindBarb( lineStart, xVal, yVal, magnitude );
      break;
    case Qgis::VectorFieldSymbology::Streamlines:
    case Qgis::VectorFieldSymbology::Traces:
      // not drawn one glyph at a time, see drawStreamlines() and drawTraces()
      break;
  }
}

QgsVectorFieldEngine::GlyphLayout QgsVectorFieldEngine::glyphLayout( const QgsPointXY &anchor, const QgsVectorFieldValueSource &source ) const
{
  const QgsRectangle extent = mContext.mapExtent();
  if ( extent.isEmpty() || mOutputSize.width() <= 0 || mOutputSize.height() <= 0 )
    return GlyphLayout();

  const double mapUnitsPerPixelX = extent.width() / mOutputSize.width();
  const double mapUnitsPerPixelY = extent.height() / mOutputSize.height();

  GlyphLayout layout;
  if ( mCfg.isOnUserDefinedGrid() )
  {
    layout.origin = anchor;
    layout.spacingX = mCfg.userGridCellWidth() * mapUnitsPerPixelX;
    layout.spacingY = mCfg.userGridCellHeight() * mapUnitsPerPixelY;
  }
  else if ( !source.nativeLayout( layout.origin, layout.spacingX, layout.spacingY ) )
  {
    // one glyph per data position was asked for, but the data has no regular layout to place them on
    return GlyphLayout();
  }

  if ( !layout.isValid() )
    return GlyphLayout();

  // safety belt to avoid rendering huge amount of overlapping glyphs
  const double minimumSpacing = std::max( 1.0, std::sqrt( static_cast<double>( mOutputSize.width() ) * mOutputSize.height() / VECTOR_FIELD_MAXIMUM_GLYPHS ) );
  layout.spacingX = std::max( layout.spacingX, minimumSpacing * mapUnitsPerPixelX );
  layout.spacingY = std::max( layout.spacingY, minimumSpacing * mapUnitsPerPixelY );

  return layout;
}

void QgsVectorFieldEngine::drawGlyphs( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsPointXY &anchor, QgsFeedback *feedback )
{
  if ( !source )
    return;

  switch ( mCfg.symbology() )
  {
    case Qgis::VectorFieldSymbology::Streamlines:
    case Qgis::VectorFieldSymbology::Traces:
      // not drawn one glyph at a time, see drawStreamlines() and drawTraces()
      return;

    case Qgis::VectorFieldSymbology::Arrows:
    case Qgis::VectorFieldSymbology::WindBarbs:
      break;
  }

  const GlyphLayout layout = glyphLayout( anchor, *source );
  if ( !layout.isValid() )
    return;

  source->setSamplingWindow( layout.spacingX, layout.spacingY );

  const QgsRectangle extent = mContext.mapExtent();
  const double mapUnitsPerPixelX = extent.width() / mOutputSize.width();
  const double mapUnitsPerPixelY = extent.height() / mOutputSize.height();

  // grown so that glyphs centered just outside of the extent are still drawn
  const double buffer = glyphExtentBuffer();
  const QgsRectangle
    bufferedExtent( extent.xMinimum() - buffer * mapUnitsPerPixelX, extent.yMinimum() - buffer * mapUnitsPerPixelY, extent.xMaximum() + buffer * mapUnitsPerPixelX, extent.yMaximum() + buffer * mapUnitsPerPixelY );

  const QgsPointXY origin = layout.origin;
  const double spacingX = layout.spacingX;
  const double spacingY = layout.spacingY;

  const long long firstColumn = static_cast<long long>( std::ceil( ( bufferedExtent.xMinimum() - origin.x() ) / spacingX ) );
  const long long lastColumn = static_cast<long long>( std::floor( ( bufferedExtent.xMaximum() - origin.x() ) / spacingX ) );
  const long long firstRow = static_cast<long long>( std::ceil( ( origin.y() - bufferedExtent.yMaximum() ) / spacingY ) );
  const long long lastRow = static_cast<long long>( std::floor( ( origin.y() - bufferedExtent.yMinimum() ) / spacingY ) );

  for ( long long row = firstRow; row <= lastRow; ++row )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const double y = origin.y() - row * spacingY;
    for ( long long column = firstColumn; column <= lastColumn; ++column )
    {
      const QgsPointXY point( origin.x() + column * spacingX, y );
      const QgsVector value = source->vectorValue( point );
      if ( std::isnan( value.x() ) || std::isnan( value.y() ) )
        continue;

      drawGlyph( mContext.mapToPixel().transform( point ), value.x(), value.y(), value.length() );
    }
  }
}

void QgsVectorFieldEngine::drawStreamlines( std::unique_ptr<QgsVectorFieldValueSource> source, QgsRasterBlockFeedback *feedback )
{
  if ( !source )
    return;

  auto field = std::make_unique<QgsVectorFieldStreamlinesField>( std::move( source ), mContext, mVectorColoring, feedback );

  field->updateSize( mContext );
  field->setPixelFillingDensity( mCfg.streamLinesSettings().seedingDensity() );
  field->setLineWidth( mContext.convertToPainterUnits( mCfg.lineWidth(), Qgis::RenderUnit::Millimeters ) );
  field->setColor( mCfg.color() );
  field->setFilter( mCfg.filterMin(), mCfg.filterMax() );

  switch ( mCfg.streamLinesSettings().seedingMethod() )
  {
    case Qgis::VectorFieldSeedingMethod::Gridded:
      if ( mCfg.isOnUserDefinedGrid() )
        field->addGriddedTraces( mCfg.userGridCellWidth(), mCfg.userGridCellHeight() );
      else
        field->addTracesOnDataPoints( mContext.mapExtent() );
      break;
    case Qgis::VectorFieldSeedingMethod::Random:
      field->addRandomTraces();
      break;
  }

  if ( mContext.renderingStopped() )
    return;

  field->compose();
  mContext.painter()->drawImage( field->topLeft(), field->image() );
}

void QgsVectorFieldEngine::drawTraces( std::unique_ptr<QgsVectorFieldValueSource> source )
{
  if ( !source )
    return;

  auto field = std::make_unique<QgsVectorFieldParticleTracesField>( std::move( source ), mContext, mVectorColoring );

  field->updateSize( mContext );
  field->setParticleSize( mContext.convertToPainterUnits( mCfg.lineWidth(), Qgis::RenderUnit::Millimeters ) );
  field->setParticlesCount( mCfg.tracesSettings().particlesCount() );
  field->setTailFactor( 1 );
  field->setStumpParticleWithLifeTime( false );

  // as the particles go through 1 pixel for dt=1 and Vmax, the maximum tail length is the time step
  field->setTimeStep( mContext.convertToPainterUnits( mCfg.tracesSettings().maximumTailLength(), mCfg.tracesSettings().maximumTailLengthUnit() ) );

  field->addRandomParticles();
  field->moveParticles();

  if ( mContext.renderingStopped() )
    return;

  mContext.painter()->drawImage( field->topLeft(), field->image() );
}

bool QgsVectorFieldEngine::calcVectorLineEnd(
  QgsPointXY &lineEnd,
  double &vectorLength,
  double &cosAlpha,
  double &sinAlpha, //out
  const QgsPointXY &lineStart,
  double xVal,
  double yVal,
  double magnitude //in
)
{
  // return true on error

  if ( xVal == 0.0 && yVal == 0.0 )
    return true;

  // do not render if magnitude is outside of the filtered range (if filtering is enabled)
  if ( mCfg.filterMin() >= 0 && magnitude < mCfg.filterMin() )
    return true;
  if ( mCfg.filterMax() >= 0 && magnitude > mCfg.filterMax() )
    return true;

  // Determine the angle of the vector, counter-clockwise, from east
  // (and associated trigs)
  const double vectorAngle = std::atan2( yVal, xVal ) - mContext.mapToPixel().mapRotation() * M_DEG2RAD;

  cosAlpha = cos( vectorAngle );
  sinAlpha = sin( vectorAngle );

  // Now determine the X and Y distances of the end of the line from the start
  double xDist = 0.0;
  double yDist = 0.0;
  switch ( mCfg.arrowSettings().shaftLengthMethod() )
  {
    case Qgis::VectorFieldArrowScalingMethod::MinMax:
    {
      const double minShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().minShaftLength(), Qgis::RenderUnit::Millimeters );
      const double maxShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().maxShaftLength(), Qgis::RenderUnit::Millimeters );
      const double minVal = mMinMag;
      const double maxVal = mMaxMag;
      const double k = ( magnitude - minVal ) / ( maxVal - minVal );
      const double L = minShaftLength + k * ( maxShaftLength - minShaftLength );
      xDist = cosAlpha * L;
      yDist = sinAlpha * L;
      break;
    }
    case Qgis::VectorFieldArrowScalingMethod::Scaled:
    {
      const double scaleFactor = mCfg.arrowSettings().scaleFactor();
      xDist = scaleFactor * xVal;
      yDist = scaleFactor * yVal;
      break;
    }
    case Qgis::VectorFieldArrowScalingMethod::Fixed:
    {
      // We must be using a fixed length
      const double fixedShaftLength = mContext.convertToPainterUnits( mCfg.arrowSettings().fixedShaftLength(), Qgis::RenderUnit::Millimeters );
      xDist = cosAlpha * fixedShaftLength;
      yDist = sinAlpha * fixedShaftLength;
      break;
    }
  }

  // Flip the Y axis (pixel vs real-world axis)
  yDist *= -1.0;

  if ( std::abs( xDist ) < 1 && std::abs( yDist ) < 1 )
    return true;

  // Determine the line coords
  lineEnd = QgsPointXY( lineStart.x() + xDist, lineStart.y() + yDist );

  vectorLength = sqrt( xDist * xDist + yDist * yDist );

  // skip rendering if line bbox does not intersect the QImage area
  if ( !QgsRectangle( lineStart, lineEnd ).intersects( QgsRectangle( 0, 0, mOutputSize.width(), mOutputSize.height() ) ) )
    return true;

  return false; //success
}

void QgsVectorFieldEngine::drawArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude )
{
  QgsPointXY lineEnd;
  double vectorLength;
  double cosAlpha, sinAlpha;
  if ( calcVectorLineEnd( lineEnd, vectorLength, cosAlpha, sinAlpha, lineStart, xVal, yVal, magnitude ) )
    return;

  // Make a set of vector head coordinates that we will place at the end of each vector,
  // scale, translate and rotate.
  QgsPointXY vectorHeadPoints[3];
  QVector<QPointF> finalVectorHeadPoints( 3 );

  const double vectorHeadWidthRatio = mCfg.arrowSettings().arrowHeadWidthRatio();
  const double vectorHeadLengthRatio = mCfg.arrowSettings().arrowHeadLengthRatio();

  // First head point:  top of ->
  vectorHeadPoints[0].setX( -1.0 * vectorHeadLengthRatio );
  vectorHeadPoints[0].setY( vectorHeadWidthRatio * 0.5 );

  // Second head point:  right of ->
  vectorHeadPoints[1].setX( 0.0 );
  vectorHeadPoints[1].setY( 0.0 );

  // Third head point:  bottom of ->
  vectorHeadPoints[2].setX( -1.0 * vectorHeadLengthRatio );
  vectorHeadPoints[2].setY( -1.0 * vectorHeadWidthRatio * 0.5 );

  // Determine the arrow head coords
  for ( int j = 0; j < 3; j++ )
  {
    finalVectorHeadPoints[j].setX( lineEnd.x() + ( vectorHeadPoints[j].x() * cosAlpha * vectorLength ) - ( vectorHeadPoints[j].y() * sinAlpha * vectorLength ) );

    finalVectorHeadPoints[j].setY( lineEnd.y() - ( vectorHeadPoints[j].x() * sinAlpha * vectorLength ) - ( vectorHeadPoints[j].y() * cosAlpha * vectorLength ) );
  }

  // Now actually draw the vector
  QPen pen( mContext.painter()->pen() );
  pen.setColor( mVectorColoring.color( magnitude ) );
  mContext.painter()->setPen( pen );
  mContext.painter()->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
  mContext.painter()->drawPolygon( finalVectorHeadPoints );
}

void QgsVectorFieldEngine::drawWindBarb( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude )
{
  // do not render if magnitude is outside of the filtered range (if filtering is enabled)
  if ( mCfg.filterMin() >= 0 && magnitude < mCfg.filterMin() )
    return;
  if ( mCfg.filterMax() >= 0 && magnitude > mCfg.filterMax() )
    return;

  QPen pen( mContext.painter()->pen() );
  pen.setColor( mVectorColoring.color( magnitude ) );
  mContext.painter()->setPen( pen );

  // we need a brush to fill center circle and pennants
  QBrush brush( pen.color() );
  mContext.painter()->setBrush( brush );

  const double shaftLength = mContext.convertToPainterUnits( mCfg.windBarbSettings().shaftLength(), mCfg.windBarbSettings().shaftLengthUnits() );
  if ( shaftLength < 1 )
    return;

  // Check if barb is above or below the equinox
  const QgsPointXY mapPoint = mContext.mapToPixel().toMapCoordinates( lineStart.x(), lineStart.y() );
  bool isNorthHemisphere = true;
  try
  {
    const QgsPointXY geoPoint = mGeographicTransform->transform( mapPoint );
    isNorthHemisphere = geoPoint.y() >= 0;
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( u"Could not transform wind barb coordinates to geographic ones"_s );
  }

  const double d = shaftLength / 25; // this is a magic number ratio between shaft length and other barb dimensions
  const double centerRadius = d;
  const double zeroCircleRadius = 2 * d;
  const double barbLength = 8 * d + pen.widthF();
  const double barbAngle = 135;
  const double barbOffset = 2 * d + pen.widthF();
  const int sign = isNorthHemisphere ? 1 : -1;

  // Determine the angle of the vector, counter-clockwise, from east
  // (and associated trigs)
  const double vectorAngle = std::atan2( yVal, xVal ) - mContext.mapToPixel().mapRotation() * M_DEG2RAD;

  // Now determine the X and Y distances of the end of the line from the start
  // Flip the Y axis (pixel vs real-world axis)
  const double xDist = cos( vectorAngle ) * shaftLength;
  const double yDist = -sin( vectorAngle ) * shaftLength;

  // Determine the line coords
  const QgsPointXY lineEnd = QgsPointXY( lineStart.x() - xDist, lineStart.y() - yDist );

  // skip rendering if line bbox does not intersect the QImage area
  if ( !QgsRectangle( lineStart, lineEnd ).intersects( QgsRectangle( 0, 0, mOutputSize.width(), mOutputSize.height() ) ) )
    return;

  // scale the magnitude to convert it to knots
  double knots = magnitude * mCfg.windBarbSettings().magnitudeMultiplier();
  QgsPointXY nextLineOrigin = lineEnd;

  // special case for no wind, just an empty circle
  if ( knots < 2.5 )
  {
    mContext.painter()->setBrush( Qt::NoBrush );
    mContext.painter()->drawEllipse( lineStart.toQPointF(), zeroCircleRadius, zeroCircleRadius );
    mContext.painter()->setBrush( brush );
    return;
  }

  const double azimuth = lineEnd.azimuth( lineStart );

  // conditionally draw the shaft
  if ( knots < 47.5 && knots > 7.5 )
  {
    // When first barb is a '10', we want to draw the shaft and barb as a single polyline for a proper join
    const QVector< QPointF > pts { lineStart.toQPointF(), lineEnd.toQPointF(), nextLineOrigin.project( barbLength, azimuth + barbAngle * sign ).toQPointF() };
    mContext.painter()->drawPolyline( pts );
    nextLineOrigin = nextLineOrigin.project( barbOffset, azimuth );
    knots -= 10;
  }
  else
  {
    // draw just the shaft
    mContext.painter()->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
  }

  // draw the center circle
  mContext.painter()->drawEllipse( lineStart.toQPointF(), centerRadius, centerRadius );

  // draw pennants (50)
  while ( knots > 47.5 )
  {
    const QVector< QPointF >
      pts { nextLineOrigin.toQPointF(), nextLineOrigin.project( barbLength / 1.414, azimuth + 90 * sign ).toQPointF(), nextLineOrigin.project( barbLength / 1.414, azimuth ).toQPointF() };
    mContext.painter()->drawPolygon( pts );
    knots -= 50;

    // don't use an offset for the next pennant
    if ( knots > 47.5 )
      nextLineOrigin = nextLineOrigin.project( barbLength / 1.414, azimuth );
    else
      nextLineOrigin = nextLineOrigin.project( barbLength / 1.414 + barbOffset, azimuth );
  }

  // draw large barbs (10)
  while ( knots > 7.5 )
  {
    mContext.painter()->drawLine( nextLineOrigin.toQPointF(), nextLineOrigin.project( barbLength, azimuth + barbAngle * sign ).toQPointF() );
    nextLineOrigin = nextLineOrigin.project( barbOffset, azimuth );
    knots -= 10;
  }

  // draw small barb (5)
  if ( knots > 2.5 )
  {
    // a single '5' barb should not start at the line end
    if ( nextLineOrigin == lineEnd )
      nextLineOrigin = nextLineOrigin.project( barbLength / 2, azimuth );

    mContext.painter()->drawLine( nextLineOrigin.toQPointF(), nextLineOrigin.project( barbLength / 2, azimuth + barbAngle * sign ).toQPointF() );
  }
}
