/***************************************************************************
                         qgsrastercontourlabeling.cpp
                         ---------------
    begin                : February 2026
    copyright            : (C) 2026 by the QGIS project
    email                : info at qgis dot org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastercontourlabeling.h"
#include "qgsrastercontourrenderer.h"

#include <gdal_alg.h>

#include "qgsapplication.h"
#include "qgsbasicnumericformat.h"
#include "qgscoordinatetransform.h"
#include "qgsgeos.h"
#include "qgslinestring.h"
#include "qgsmessagelog.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerrenderer.h"
#include "qgsrasterpipe.h"
#include "qgsrasterviewport.h"
#include "qgsscaleutils.h"
#include "qgsstyle.h"
#include "qgsstyleentityvisitor.h"
#include "qgstextlabelfeature.h"

#include <QString>

using namespace Qt::StringLiterals;

//
// QgsRasterContourLabelProvider
//

struct ContourLabelData
{
  QgsRasterContourLabelProvider *provider;
  QgsRenderContext *context;
  QgsNumericFormat *numericFormat;
  QgsNumericFormatContext *numericContext;
  QgsRectangle extent;
  int inputWidth;
  int inputHeight;
  QgsCoordinateTransform transform;
  double contourIndexInterval;
  bool labelIndexOnly;
};

static CPLErr _contourLabelWriter( double dfLevel, int nPoints, double *padfX, double *padfY, void *ptr )
{
  ContourLabelData *data = static_cast<ContourLabelData *>( ptr );

  if ( data->labelIndexOnly && data->contourIndexInterval > 0 )
  {
    if ( !qgsDoubleNear( fmod( dfLevel, data->contourIndexInterval ), 0 ) )
      return CE_None;
  }

  if ( nPoints < 2 )
    return CE_None;

  QVector<double> xCoords( nPoints );
  QVector<double> yCoords( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    // pixel coords → layer CRS
    xCoords[i] = data->extent.xMinimum() + ( padfX[i] / data->inputWidth ) * data->extent.width();
    yCoords[i] = data->extent.yMaximum() - ( padfY[i] / data->inputHeight ) * data->extent.height();
  }

  QgsLineString lineString( xCoords, yCoords );

  if ( !data->transform.isShortCircuited() )
  {
    try
    {
      lineString.transform( data->transform );
    }
    catch ( QgsCsException & )
    {
      return CE_None;
    }
  }

  const QString labelText = data->numericFormat->formatDouble( dfLevel, *data->numericContext );
  data->provider->addContourLabel( lineString, labelText, *data->context );

  return CE_None;
}


QgsRasterContourLabelProvider::QgsRasterContourLabelProvider( QgsRasterLayer *layer )
  : QgsRasterLayerLabelProvider( layer )
{
  mPlacement = Qgis::LabelPlacement::Line;
  mFlags |= MergeConnectedLines;
}

QgsRasterContourLabelProvider::~QgsRasterContourLabelProvider() = default;

void QgsRasterContourLabelProvider::addContourLabel( const QgsLineString &line, const QString &text, QgsRenderContext &context )
{
  const QgsMapToPixel &m2p = context.mapToPixel();
  const double uPP = m2p.mapUnitsPerPixel();

  QgsLineString geom( line );

  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPointXY center = context.mapExtent().center();
    QTransform t = QTransform::fromTranslate( center.x(), center.y() );
    t.rotate( -m2p.mapRotation() );
    t.translate( -center.x(), -center.y() );
    geom.transform( t );
  }

  const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( { text }, mFormat );
  QgsTextDocumentMetrics documentMetrics = QgsTextDocumentMetrics::calculateMetrics( doc, mFormat, context );
  const QSizeF size = documentMetrics.documentSize( Qgis::TextLayoutMode::Labeling, Qgis::TextOrientation::Horizontal );

  auto feature = std::make_unique<QgsTextLabelFeature>( mLabels.size(),
                 QgsGeos::asGeos( &geom ),
                 QSizeF( size.width() * uPP,
                         size.height() * uPP ) );

  feature->setDocument( doc, documentMetrics );
  feature->setZIndex( mZIndex );
  feature->setOverlapHandling( mPlacementSettings.overlapHandling() );

  mLabels.append( feature.release() );
}

void QgsRasterContourLabelProvider::generateLabels( QgsRenderContext &context, QgsRasterPipe *pipe, QgsRasterViewPort *rasterViewPort, QgsRasterLayerRendererFeedback *feedback )
{
  if ( !pipe )
    return;

  QgsRasterDataProvider *provider = pipe->provider();
  if ( !provider )
    return;

  if ( provider->xSize() == 0 || provider->ySize() == 0 )
    return;

  if ( !rasterViewPort )
    return;

  QgsCoordinateTransform layerToMapTransform = context.coordinateTransform();
  layerToMapTransform.setBallparkTransformsAreAppropriate( true );
  QgsRectangle layerVisibleExtent;
  try
  {
    layerVisibleExtent = layerToMapTransform.transformBoundingBox( rasterViewPort->mDrawnExtent, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &cs )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not reproject view extent: %1" ).arg( cs.what() ), QObject::tr( "Raster" ) );
    return;
  }

  int subRegionWidth = 0;
  int subRegionHeight = 0;
  int subRegionLeft = 0;
  int subRegionTop = 0;
  QgsRectangle rasterSubRegion = QgsRasterIterator::subRegion(
                                   provider->extent(),
                                   provider->xSize(),
                                   provider->ySize(),
                                   layerVisibleExtent,
                                   subRegionWidth,
                                   subRegionHeight,
                                   subRegionLeft,
                                   subRegionTop );

  const int inputWidth = std::max( 1, static_cast<int>( std::round( subRegionWidth / mDownscale ) ) );
  const int inputHeight = std::max( 1, static_cast<int>( std::round( subRegionHeight / mDownscale ) ) );

  std::unique_ptr<QgsRasterBlock> inputBlock( provider->block( mInputBand, rasterSubRegion, inputWidth, inputHeight, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
    return;

  if ( !inputBlock->convert( Qgis::DataType::Float64 ) )
    return;

  double *scanline = reinterpret_cast<double *>( inputBlock->bits() );

  QgsNumericFormatContext numericContext;
  numericContext.setExpressionContext( context.expressionContext() );

  ContourLabelData clData;
  clData.provider = this;
  clData.context = &context;
  clData.numericFormat = numericFormat();
  clData.numericContext = &numericContext;
  clData.extent = rasterSubRegion;
  clData.inputWidth = inputWidth;
  clData.inputHeight = inputHeight;
  clData.transform = layerToMapTransform;
  clData.contourIndexInterval = mContourIndexInterval;
  clData.labelIndexOnly = mLabelIndexOnly;

  const double contourBase = 0.;
  GDALContourGeneratorH cg = GDAL_CG_Create( inputWidth, inputHeight,
                             inputBlock->hasNoDataValue(), inputBlock->noDataValue(),
                             mContourInterval, contourBase,
                             _contourLabelWriter, static_cast<void *>( &clData ) );

  for ( int i = 0; i < inputHeight; ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    GDAL_CG_FeedLine( cg, scanline );
    scanline += inputWidth;
  }

  GDAL_CG_Destroy( cg );
}


//
// QgsRasterLayerContourLabeling
//

QgsRasterLayerContourLabeling::QgsRasterLayerContourLabeling()
  : mNumericFormat( std::make_unique<QgsBasicNumericFormat>() )
{
  mThinningSettings.setMaximumNumberLabels( 2000 );
  mThinningSettings.setLimitNumberLabelsEnabled( true );
}

QgsRasterLayerContourLabeling::~QgsRasterLayerContourLabeling() = default;

QString QgsRasterLayerContourLabeling::type() const
{
  return u"contour"_s;
}

QgsRasterLayerContourLabeling *QgsRasterLayerContourLabeling::clone() const
{
  auto res = std::make_unique<QgsRasterLayerContourLabeling>();
  res->setTextFormat( mTextFormat );

  if ( mNumericFormat )
    res->mNumericFormat.reset( mNumericFormat->clone() );

  res->setLabelIndexOnly( mLabelIndexOnly );
  res->setPriority( mPriority );
  res->setPlacementSettings( mPlacementSettings );
  res->setThinningSettings( mThinningSettings );
  res->setZIndex( mZIndex );
  res->setScaleBasedVisibility( mScaleVisibility );
  res->setMaximumScale( mMaximumScale );
  res->setMinimumScale( mMinimumScale );

  return res.release();
}

std::unique_ptr<QgsRasterLayerLabelProvider> QgsRasterLayerContourLabeling::provider( QgsRasterLayer *layer ) const
{
  auto res = std::make_unique<QgsRasterContourLabelProvider>( layer );
  res->setTextFormat( mTextFormat );
  res->setLabelIndexOnly( mLabelIndexOnly );
  res->setPriority( mPriority );
  res->setPlacementSettings( mPlacementSettings );
  res->setZIndex( mZIndex );
  res->setThinningSettings( mThinningSettings );
  if ( mNumericFormat )
  {
    res->setNumericFormat( std::unique_ptr<QgsNumericFormat>( mNumericFormat->clone() ) );
  }

  // Read contour parameters from the renderer so they stay in sync with the Symbology tab
  if ( const QgsRasterContourRenderer *renderer = dynamic_cast<const QgsRasterContourRenderer *>( layer->renderer() ) )
  {
    res->setInputBand( renderer->inputBand() );
    res->setContourInterval( renderer->contourInterval() );
    res->setContourIndexInterval( renderer->contourIndexInterval() );
    res->setDownscale( renderer->downscale() );
  }

  return res;
}

QDomElement QgsRasterLayerContourLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"labeling"_s );
  elem.setAttribute( u"type"_s, u"contour"_s );
  elem.setAttribute( u"labelIndexOnly"_s, mLabelIndexOnly ? 1 : 0 );
  elem.setAttribute( u"priority"_s, mPriority );
  elem.setAttribute( u"zIndex"_s, mZIndex );

  {
    QDomElement textFormatElem = doc.createElement( u"textFormat"_s );
    textFormatElem.appendChild( mTextFormat.writeXml( doc, context ) );
    elem.appendChild( textFormatElem );
  }

  {
    QDomElement numericFormatElem = doc.createElement( u"numericFormat"_s );
    mNumericFormat->writeXml( numericFormatElem, doc, context );
    elem.appendChild( numericFormatElem );
  }

  {
    QDomElement placementElem = doc.createElement( u"placement"_s );
    placementElem.setAttribute( u"overlapHandling"_s, qgsEnumValueToKey( mPlacementSettings.overlapHandling() ) );
    elem.appendChild( placementElem );
  }

  {
    QDomElement thinningElem = doc.createElement( u"thinning"_s );
    thinningElem.setAttribute( u"maxNumLabels"_s, mThinningSettings.maximumNumberLabels() );
    thinningElem.setAttribute( u"limitNumLabels"_s, mThinningSettings.limitNumberOfLabelsEnabled() );
    elem.appendChild( thinningElem );
  }

  {
    QDomElement renderingElem = doc.createElement( u"rendering"_s );
    renderingElem.setAttribute( u"scaleVisibility"_s, mScaleVisibility );
    // element names flipped vs member — matches vector labeling convention
    renderingElem.setAttribute( u"scaleMin"_s, mMaximumScale );
    renderingElem.setAttribute( u"scaleMax"_s, mMinimumScale );
    elem.appendChild( renderingElem );
  }

  return elem;
}

bool QgsRasterLayerContourLabeling::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  QgsStyleTextFormatEntity entity( mTextFormat );
  if ( !visitor->visit( &entity ) )
    return false;

  return true;
}

bool QgsRasterLayerContourLabeling::requiresAdvancedEffects() const
{
  return mTextFormat.containsAdvancedEffects();
}

bool QgsRasterLayerContourLabeling::hasNonDefaultCompositionMode() const
{
  return mTextFormat.hasNonDefaultCompositionMode();
}

void QgsRasterLayerContourLabeling::multiplyOpacity( double opacityFactor )
{
  mTextFormat.multiplyOpacity( opacityFactor );
}

bool QgsRasterLayerContourLabeling::isInScaleRange( double scale ) const
{
  return !mScaleVisibility
         || ( ( mMinimumScale == 0 || !QgsScaleUtils::lessThanMaximumScale( scale, mMinimumScale ) )
              && ( mMaximumScale == 0 || !QgsScaleUtils::equalToOrGreaterThanMinimumScale( scale, mMaximumScale ) ) );
}

QgsRasterLayerContourLabeling *QgsRasterLayerContourLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  auto res = std::make_unique<QgsRasterLayerContourLabeling>();
  res->setLabelIndexOnly( element.attribute( u"labelIndexOnly"_s, u"0"_s ).toInt() );
  res->setPriority( element.attribute( u"priority"_s, u"0.5"_s ).toDouble() );
  res->setZIndex( element.attribute( u"zIndex"_s, u"0"_s ).toDouble() );

  const QDomElement textFormatElem = element.firstChildElement( u"textFormat"_s );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( u"text-style"_s );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    QgsTextFormat format;
    format.readXml( textFormatElem, context );
    res->setTextFormat( format );
  }

  const QDomNodeList numericFormatNodeList = element.elementsByTagName( u"numericFormat"_s );
  if ( !numericFormatNodeList.isEmpty() )
  {
    const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
    res->mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
  }

  QDomElement placementElem = element.firstChildElement( u"placement"_s );
  res->mPlacementSettings.setOverlapHandling( qgsEnumKeyToValue( placementElem.attribute( u"overlapHandling"_s ), Qgis::LabelOverlapHandling::PreventOverlap ) );

  QDomElement thinningElem = element.firstChildElement( u"thinning"_s );
  res->mThinningSettings.setMaximumNumberLabels( thinningElem.attribute( u"maxNumLabels"_s, u"2000"_s ).toInt() );
  res->mThinningSettings.setLimitNumberLabelsEnabled( thinningElem.attribute( u"limitNumLabels"_s, u"1"_s ).toInt() );

  QDomElement renderingElem = element.firstChildElement( u"rendering"_s );
  res->mMaximumScale = renderingElem.attribute( u"scaleMin"_s, u"0"_s ).toDouble();
  res->mMinimumScale = renderingElem.attribute( u"scaleMax"_s, u"0"_s ).toDouble();
  res->mScaleVisibility = renderingElem.attribute( u"scaleVisibility"_s ).toInt();

  return res.release();
}

QgsTextFormat QgsRasterLayerContourLabeling::textFormat() const
{
  return mTextFormat;
}

void QgsRasterLayerContourLabeling::setTextFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

const QgsNumericFormat *QgsRasterLayerContourLabeling::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsRasterLayerContourLabeling::setNumericFormat( QgsNumericFormat *format )
{
  if ( format != mNumericFormat.get() )
    mNumericFormat.reset( format );
}

double QgsRasterLayerContourLabeling::zIndex() const
{
  return mZIndex;
}

void QgsRasterLayerContourLabeling::setZIndex( double index )
{
  mZIndex = index;
}

double QgsRasterLayerContourLabeling::maximumScale() const
{
  return mMaximumScale;
}

void QgsRasterLayerContourLabeling::setMaximumScale( double scale )
{
  mMaximumScale = scale;
}

double QgsRasterLayerContourLabeling::minimumScale() const
{
  return mMinimumScale;
}

void QgsRasterLayerContourLabeling::setMinimumScale( double scale )
{
  mMinimumScale = scale;
}

bool QgsRasterLayerContourLabeling::hasScaleBasedVisibility() const
{
  return mScaleVisibility;
}

void QgsRasterLayerContourLabeling::setScaleBasedVisibility( bool enabled )
{
  mScaleVisibility = enabled;
}
