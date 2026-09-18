/***************************************************************************
                         qgsrastervectorfieldrenderer.cpp
                         --------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastervectorfieldrenderer.h"

#include <algorithm>
#include <cmath>

#include "qgscolorrampshader.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsrasterblock.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastervectorfieldvaluesource.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgsvectorfieldengine.h"

#include <QImage>
#include <QPainter>
#include <QString>

using namespace Qt::StringLiterals;

//! Upper bound on the number of vector values read from the two bands for a single block
constexpr qint64 VECTOR_FIELD_MAXIMUM_SAMPLES = 4'000'000;

QgsRasterVectorFieldRenderer::QgsRasterVectorFieldRenderer( QgsRasterInterface *input )
  : QgsRasterRenderer( input, u"vectorfield"_s )
{}

QgsRasterVectorFieldRenderer::~QgsRasterVectorFieldRenderer() = default;

QgsRasterVectorFieldRenderer *QgsRasterVectorFieldRenderer::clone() const
{
  QgsRasterVectorFieldRenderer *renderer = new QgsRasterVectorFieldRenderer( nullptr );
  renderer->copyCommonProperties( this );
  renderer->mSourceMode = mSourceMode;
  renderer->mXBand = mXBand;
  renderer->mYBand = mYBand;
  renderer->mDirectionBand = mDirectionBand;
  renderer->mDirectionEncoding = mDirectionEncoding;
  renderer->mSettings = mSettings;
  renderer->mMinimumMagnitude = mMinimumMagnitude;
  renderer->mMaximumMagnitude = mMaximumMagnitude;
  return renderer;
}

Qgis::RasterRendererFlags QgsRasterVectorFieldRenderer::flags() const
{
  return Qgis::RasterRendererFlag::UseNoDataForOutOfRangePixels;
}

std::unique_ptr<QgsRasterRenderer> QgsRasterVectorFieldRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
    return nullptr;

  auto renderer = std::make_unique<QgsRasterVectorFieldRenderer>( input );
  renderer->readXml( elem );

  renderer->setSourceMode( qgsEnumKeyToValue( elem.attribute( u"source-mode"_s ), Qgis::RasterVectorFieldSourceMode::CartesianComponents ) );
  renderer->setXBand( elem.attribute( u"x-band"_s, u"-1"_s ).toInt() );
  renderer->setYBand( elem.attribute( u"y-band"_s, u"-1"_s ).toInt() );
  renderer->setDirectionBand( elem.attribute( u"direction-band"_s, u"-1"_s ).toInt() );
  renderer->setDirectionEncoding( qgsEnumKeyToValue( elem.attribute( u"direction-encoding"_s ), Qgis::RasterDirectionEncoding::Esri ) );

  if ( elem.hasAttribute( u"minimum-magnitude"_s ) )
    renderer->setMinimumMagnitude( elem.attribute( u"minimum-magnitude"_s ).toDouble() );
  if ( elem.hasAttribute( u"maximum-magnitude"_s ) )
    renderer->setMaximumMagnitude( elem.attribute( u"maximum-magnitude"_s ).toDouble() );

  const QDomElement settingsElem = elem.firstChildElement( u"vector-settings"_s );
  if ( !settingsElem.isNull() )
  {
    QgsVectorFieldSettings settings;
    settings.readXml( settingsElem, QgsReadWriteContext() );
    renderer->setSettings( settings );
  }

  return renderer;
}

void QgsRasterVectorFieldRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
    return;

  QDomElement rasterRendererElem = doc.createElement( u"rasterrenderer"_s );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( u"source-mode"_s, qgsEnumValueToKey( mSourceMode ) );
  rasterRendererElem.setAttribute( u"x-band"_s, mXBand );
  rasterRendererElem.setAttribute( u"y-band"_s, mYBand );
  rasterRendererElem.setAttribute( u"direction-band"_s, mDirectionBand );
  rasterRendererElem.setAttribute( u"direction-encoding"_s, qgsEnumValueToKey( mDirectionEncoding ) );
  if ( !std::isnan( mMinimumMagnitude ) )
    rasterRendererElem.setAttribute( u"minimum-magnitude"_s, qgsDoubleToString( mMinimumMagnitude ) );
  if ( !std::isnan( mMaximumMagnitude ) )
    rasterRendererElem.setAttribute( u"maximum-magnitude"_s, qgsDoubleToString( mMaximumMagnitude ) );
  rasterRendererElem.appendChild( mSettings.writeXml( doc, QgsReadWriteContext() ) );

  parentElem.appendChild( rasterRendererElem );
}

bool QgsRasterVectorFieldRenderer::shouldAcceptBand( int band ) const
{
  // without an input the band cannot be validated yet, which happens while cloning or reading a project
  return band > 0 && ( !mInput || band <= mInput->bandCount() );
}

bool QgsRasterVectorFieldRenderer::setXBand( int band )
{
  if ( !shouldAcceptBand( band ) )
    return false;

  mXBand = band;
  return true;
}

bool QgsRasterVectorFieldRenderer::setYBand( int band )
{
  if ( !shouldAcceptBand( band ) )
    return false;

  mYBand = band;
  return true;
}

bool QgsRasterVectorFieldRenderer::setDirectionBand( int band )
{
  if ( !shouldAcceptBand( band ) )
    return false;

  mDirectionBand = band;
  return true;
}

QList<int> QgsRasterVectorFieldRenderer::usesBands() const
{
  QList<int> bands;
  switch ( mSourceMode )
  {
    case Qgis::RasterVectorFieldSourceMode::CartesianComponents:
      if ( mXBand > 0 )
        bands << mXBand;
      if ( mYBand > 0 )
        bands << mYBand;
      break;

    case Qgis::RasterVectorFieldSourceMode::EncodedDirection:
      if ( mDirectionBand > 0 )
        bands << mDirectionBand;
      break;
  }
  return bands;
}

QList<QPair<QString, QColor>> QgsRasterVectorFieldRenderer::legendSymbologyItems() const
{
  QList<QPair<QString, QColor>> items;

  switch ( mSettings.coloringMethod() )
  {
    case QgsInterpolatedLineColor::SingleColor:
      items << qMakePair( QString(), mSettings.color() );
      break;

    case QgsInterpolatedLineColor::ColorRamp:
    {
      const QList<QgsColorRampShader::ColorRampItem> rampItems = mSettings.colorRampShader().colorRampItemList();
      items.reserve( rampItems.size() );
      for ( const QgsColorRampShader::ColorRampItem &rampItem : rampItems )
        items << qMakePair( rampItem.label, rampItem.color );
      break;
    }
  }

  return items;
}


//! The extent and size at which the two component bands are read for one block
struct QgsRasterVectorFieldSampleGrid
{
    QgsRectangle extent;
    int columns = 0;
    int rows = 0;
};

/**
 * Returns the extent and size at which \a input should be read to draw the block of \a width by
 * \a height pixels covering \a extent.
 */
static QgsRasterVectorFieldSampleGrid calculateSampleGrid( QgsRasterInterface *input, const QgsRectangle &extent, int width, int height )
{
  QgsRasterVectorFieldSampleGrid grid;
  grid.extent = extent;
  grid.columns = width;
  grid.rows = height;

  const QgsRectangle sourceExtent = input->extent();
  const int sourceWidth = input->xSize();
  const int sourceHeight = input->ySize();
  if ( sourceWidth <= 0 || sourceHeight <= 0 || sourceExtent.isEmpty() )
    return grid; // the source resolution is unknown, sample at the resolution of the block

  const double xResolution = sourceExtent.width() / sourceWidth;
  const double yResolution = sourceExtent.height() / sourceHeight;

  // the range of source pixels covering the block
  const double firstColumn = std::floor( ( extent.xMinimum() - sourceExtent.xMinimum() ) / xResolution );
  const double lastColumn = std::ceil( ( extent.xMaximum() - sourceExtent.xMinimum() ) / xResolution );
  const double firstRow = std::floor( ( sourceExtent.yMaximum() - extent.yMaximum() ) / yResolution );
  const double lastRow = std::ceil( ( sourceExtent.yMaximum() - extent.yMinimum() ) / yResolution );

  const qint64 columns = std::max<qint64>( 1, static_cast<qint64>( lastColumn - firstColumn ) );
  const qint64 rows = std::max<qint64>( 1, static_cast<qint64>( lastRow - firstRow ) );

  // sampling much finer than the block is drawn is wasted, and a whole fine raster does not fit in memory
  qint64 step = 1;
  step = std::max( step, ( columns + 2 * width - 1 ) / ( 2 * width ) );
  step = std::max( step, ( rows + 2 * height - 1 ) / ( 2 * height ) );
  while ( ( ( columns + step - 1 ) / step ) * ( ( rows + step - 1 ) / step ) > VECTOR_FIELD_MAXIMUM_SAMPLES )
    ++step;

  grid.columns = static_cast<int>( ( columns + step - 1 ) / step );
  grid.rows = static_cast<int>( ( rows + step - 1 ) / step );
  grid.extent = QgsRectangle(
    sourceExtent.xMinimum() + firstColumn * xResolution,
    sourceExtent.yMaximum() - ( firstRow + static_cast<double>( grid.rows ) * step ) * yResolution,
    sourceExtent.xMinimum() + ( firstColumn + static_cast<double>( grid.columns ) * step ) * xResolution,
    sourceExtent.yMaximum() - firstRow * yResolution
  );

  return grid;
}

/**
 * Returns the ratio between the resolution the block is drawn at and the resolution of the map
 * painter, or 1 when \a context does not describe a map render.
 *
 * The block is requested at a higher resolution on high DPI displays, and both the resample filter
 * and the projector may request it at a resolution of their own, so it is measured rather than
 * derived from the DPI settings.
 */
static double calculateBlockScale( const QgsRenderContext &context, const QgsRectangle &extent, int width )
{
  if ( !context.mapToPixel().isValid() )
    return 1.0;

  QgsRectangle extentInMapCrs = extent;
  try
  {
    QgsCoordinateTransform transform = context.coordinateTransform();
    if ( transform.isValid() )
    {
      transform.setBallparkTransformsAreAppropriate( true );
      extentInMapCrs = transform.transformBoundingBox( extent );
    }
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( u"Could not transform the block extent to the map CRS"_s );
  }

  const double mapUnitsPerPixel = context.mapToPixel().mapUnitsPerPixel();
  if ( mapUnitsPerPixel <= 0 || extentInMapCrs.width() <= 0 )
    return 1.0;

  return std::clamp( width / ( extentInMapCrs.width() / mapUnitsPerPixel ), 0.05, 20.0 );
}

/**
 * Calculates the \a minimum and \a maximum magnitude of the vectors held by \a xValues and
 * \a yValues, and returns FALSE when neither holds any data.
 */
static bool calculateMagnitudeRange( const QgsRasterBlock *xValues, const QgsRasterBlock *yValues, double &minimum, double &maximum )
{
  minimum = std::numeric_limits<double>::max();
  maximum = std::numeric_limits<double>::lowest();
  bool found = false;

  const int rows = std::min( xValues->height(), yValues->height() );
  const int columns = std::min( xValues->width(), yValues->width() );
  for ( int row = 0; row < rows; ++row )
  {
    for ( int column = 0; column < columns; ++column )
    {
      bool isNoData = false;
      const double x = xValues->valueAndNoData( row, column, isNoData );
      if ( isNoData || std::isnan( x ) )
        continue;

      const double y = yValues->valueAndNoData( row, column, isNoData );
      if ( isNoData || std::isnan( y ) )
        continue;

      const double magnitude = std::hypot( x, y );
      minimum = std::min( minimum, magnitude );
      maximum = std::max( maximum, magnitude );
      found = true;
    }
  }

  return found;
}

QgsRasterBlock *QgsRasterVectorFieldRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )

  auto outputBlock = std::make_unique<QgsRasterBlock>();
  if ( !mInput || width <= 0 || height <= 0 || extent.isEmpty() )
    return outputBlock.release();

  // each source mode requires different bands
  switch ( mSourceMode )
  {
    case Qgis::RasterVectorFieldSourceMode::CartesianComponents:
      if ( mXBand <= 0 || mYBand <= 0 )
        return outputBlock.release();
      break;

    case Qgis::RasterVectorFieldSourceMode::EncodedDirection:
      if ( mDirectionBand <= 0 )
        return outputBlock.release();
      break;
  }

  QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider *>( mInput->sourceInput() );
  const QgsCoordinateReferenceSystem layerCrs = provider ? provider->crs() : QgsCoordinateReferenceSystem();
  const QgsRectangle layerExtent = provider ? provider->extent() : mInput->extent();

  const QgsRasterVectorFieldSampleGrid grid = calculateSampleGrid( mInput, extent, width, height );

  std::shared_ptr<const QgsRasterBlock> xValues;
  std::shared_ptr<const QgsRasterBlock> yValues;
  std::shared_ptr<const QgsRasterBlock> directionValues;

  double minimumMagnitude = mMinimumMagnitude;
  double maximumMagnitude = mMaximumMagnitude;

  switch ( mSourceMode )
  {
    case Qgis::RasterVectorFieldSourceMode::CartesianComponents:
    {
      xValues.reset( mInput->block( mXBand, grid.extent, grid.columns, grid.rows, feedback ) );
      yValues.reset( mInput->block( mYBand, grid.extent, grid.columns, grid.rows, feedback ) );
      if ( !xValues || !yValues || xValues->isEmpty() || yValues->isEmpty() )
      {
        QgsDebugError( u"No raster data!"_s );
        return outputBlock.release();
      }

      if ( std::isnan( minimumMagnitude ) || std::isnan( maximumMagnitude ) )
      {
        double dataMinimum = 0;
        double dataMaximum = 0;
        if ( !calculateMagnitudeRange( xValues.get(), yValues.get(), dataMinimum, dataMaximum ) )
          return outputBlock.release(); // the whole block holds no data

        if ( std::isnan( minimumMagnitude ) )
          minimumMagnitude = dataMinimum;
        if ( std::isnan( maximumMagnitude ) )
          maximumMagnitude = dataMaximum;
      }
      break;
    }

    case Qgis::RasterVectorFieldSourceMode::EncodedDirection:
    {
      // We don't want resampling, the pixels hold direction coded values
      const bool resamplingWasEnabled = provider && provider->isProviderResamplingEnabled();
      if ( resamplingWasEnabled )
        provider->enableProviderResampling( false );

      directionValues.reset( mInput->block( mDirectionBand, grid.extent, grid.columns, grid.rows, feedback ) );

      if ( resamplingWasEnabled )
        provider->enableProviderResampling( true );

      if ( !directionValues || directionValues->isEmpty() )
      {
        QgsDebugError( u"No raster data!"_s );
        return outputBlock.release();
      }

      // the encodings carry a direction only, every vector has a magnitude of one
      minimumMagnitude = 0;
      maximumMagnitude = 1;
      break;
    }
  }

  QImage image( width, height, QImage::Format_ARGB32_Premultiplied );
  if ( image.isNull() )
    return outputBlock.release();
  image.fill( Qt::transparent );

  // scoped so that the painter has ended before the block takes the image, and the engine, which
  // restores the painter state when it goes, is destroyed before the painter
  {
    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing, true );

    // the copy taken from the feedback has no feedback of its own, and the streamline field only
    // iterates while it has one to check for cancellation
    QgsRasterBlockFeedback localFeedback;
    QgsRasterBlockFeedback *effectiveFeedback = feedback ? feedback : &localFeedback;

    // the feedback carries a copy of the map render context, which is where the scale factor, the
    // reference scale and the render flags come from. It is missing when the renderer is used
    // outside of a map render, by the file writer or by the layer preview.
    const bool hasMapContext = feedback && feedback->renderContext().mapToPixel().isValid();
    QgsRenderContext context = hasMapContext ? feedback->renderContext() : QgsRenderContext::fromQPainter( &painter );

    const double blockScale = hasMapContext ? calculateBlockScale( context, extent, width ) : 1.0;

    // the copied context still points at the map painter and describes the map coordinate space,
    // everything which positions or sizes the symbology has to be replaced by its block equivalent
    context.setPainter( &painter );
    context.setPreviewRenderPainter( nullptr );
    context.setMaskPainter( nullptr );
    context.setElevationMap( nullptr );
    context.setScaleFactor( context.scaleFactor() * blockScale );
    context.setDevicePixelRatio( 1 );
    context.setDpiTarget( -1.0 );
    // the block image is axis aligned, the drawer rotates it once it is complete
    context.setMapToPixel( QgsMapToPixel( extent.width() / width, extent.center().x(), extent.center().y(), width, height, 0 ) );
    context.setMapExtent( extent );
    context.setExtent( extent );
    context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

    // the symbology is drawn in the coordinates of the raster itself, and reprojected afterwards
    // along with the image, so the context must not transform anything. A valid transform is still
    // needed for the wind barbs to tell which hemisphere they are in.
    if ( layerCrs.isValid() )
    {
      context.setCoordinateTransform( QgsCoordinateTransform( layerCrs, layerCrs, context.transformContext() ) );
      QgsDistanceArea distanceArea;
      distanceArea.setSourceCrs( layerCrs, context.transformContext() );
      distanceArea.setEllipsoid( layerCrs.ellipsoidAcronym() );
      context.setDistanceArea( distanceArea );
    }

    context.setFeedback( effectiveFeedback );

    // the grid cell size is expressed in canvas pixels, which the block pixels only match at a
    // device pixel ratio of one
    QgsVectorFieldSettings settings = mSettings;
    settings.setUserGridCellWidth( std::max( 1, static_cast<int>( std::round( settings.userGridCellWidth() * blockScale ) ) ) );
    settings.setUserGridCellHeight( std::max( 1, static_cast<int>( std::round( settings.userGridCellHeight() * blockScale ) ) ) );

    const QgsRectangle dataExtent = extent.intersect( layerExtent );

    std::unique_ptr<QgsVectorFieldValueSource> source;
    switch ( mSourceMode )
    {
      case Qgis::RasterVectorFieldSourceMode::CartesianComponents:
        source = std::make_unique<QgsRasterVectorFieldValueSource>( xValues, yValues, grid.extent, dataExtent, maximumMagnitude );
        break;

      case Qgis::RasterVectorFieldSourceMode::EncodedDirection:
        source = std::make_unique<QgsRasterVectorFieldEncodedDirectionValueSource>( directionValues, mDirectionEncoding, grid.extent, dataExtent );
        break;
    }

    QgsVectorFieldEngine engine( maximumMagnitude, minimumMagnitude, settings, context, QSize( width, height ) );

    switch ( settings.symbology() )
    {
      case Qgis::VectorFieldSymbology::Arrows:
      case Qgis::VectorFieldSymbology::WindBarbs:
        // the glyphs are laid out from the corner of the layer, a point which does not depend on
        // how the render is split into blocks, so that the pattern continues across block boundaries
        engine.drawGlyphs( std::move( source ), QgsPointXY( layerExtent.xMinimum(), layerExtent.yMaximum() ), effectiveFeedback );
        break;

      case Qgis::VectorFieldSymbology::Streamlines:
        engine.drawStreamlines( std::move( source ), effectiveFeedback );
        break;

      case Qgis::VectorFieldSymbology::Traces:
        engine.drawTraces( std::move( source ) );
        break;
    }
  }

  outputBlock->setImage( &image );
  return outputBlock.release();
}
