/***************************************************************************
                         qgsrasterlabeling.cpp
                         ---------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlabeling.h"
#include "qgsrasterlayer.h"
#include "labelposition.h"
#include "feature.h"
#include "qgstextrenderer.h"
#include "qgstextlabelfeature.h"
#include "qgsstyleentityvisitor.h"
#include "qgsstyle.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgsbasicnumericformat.h"
#include "qgsapplication.h"
#include "qgsscaleutils.h"
#include "qgsmessagelog.h"
#include "qgsrasterpipe.h"
#include "qgsrasterlayerrenderer.h"

QgsRasterLayerLabelProvider::QgsRasterLayerLabelProvider( QgsRasterLayer *layer )
  : QgsAbstractLabelProvider( layer )
  , mNumericFormat( std::make_unique< QgsBasicNumericFormat >() )
{
  mPlacement = Qgis::LabelPlacement::OverPoint;
  mFlags |= DrawLabels;
}

QgsRasterLayerLabelProvider::~QgsRasterLayerLabelProvider()
{
  qDeleteAll( mLabels );
}

void QgsRasterLayerLabelProvider::addLabel( const QgsPoint &mapPoint, const QString &text, QgsRenderContext &context )
{
  QgsPoint geom = mapPoint;
  const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( { text }, mFormat );
  QgsTextDocumentMetrics documentMetrics = QgsTextDocumentMetrics::calculateMetrics( doc, mFormat, context );
  const QSizeF size = documentMetrics.documentSize( Qgis::TextLayoutMode::Point, Qgis::TextOrientation::Horizontal );


  // Rotate the geometry if needed, before clipping
  const QgsMapToPixel &m2p = context.mapToPixel();
  if ( !qgsDoubleNear( m2p.mapRotation(), 0 ) )
  {
    QgsPointXY center = context.mapExtent().center();

    QTransform t = QTransform::fromTranslate( center.x(), center.y() );
    t.rotate( - m2p.mapRotation() );
    t.translate( -center.x(), -center.y() );
    geom.transform( t );
  }

  const double uPP = m2p.mapUnitsPerPixel();
  std::unique_ptr< QgsTextLabelFeature > feature = std::make_unique< QgsTextLabelFeature >( mLabels.size(),
      QgsGeos::asGeos( &geom ),
      QSizeF( size.width() * uPP,
              size.height() * uPP ) );

  feature->setDocument( doc, documentMetrics );
  feature->setFixedAngle( 0 );
  feature->setHasFixedAngle( true );
  feature->setQuadOffset( QPointF( 0, 0 ) );
  feature->setZIndex( mZIndex );

  feature->setOverlapHandling( mPlacementSettings.overlapHandling() );

  mLabels.append( feature.release() );
}

void QgsRasterLayerLabelProvider::setTextFormat( const QgsTextFormat &format )
{
  mFormat = format;
}

void QgsRasterLayerLabelProvider::setNumericFormat( std::unique_ptr<QgsNumericFormat> format )
{
  mNumericFormat = std::move( format );
}

QgsNumericFormat *QgsRasterLayerLabelProvider::numericFormat()
{
  return mNumericFormat.get();
}

void QgsRasterLayerLabelProvider::setResampleMethod( Qgis::RasterResamplingMethod method )
{
  mResampleMethod = method;
}

void QgsRasterLayerLabelProvider::setResampleOver( int pixels )
{
  mResampleOver = pixels;
}

QList<QgsLabelFeature *> QgsRasterLayerLabelProvider::labelFeatures( QgsRenderContext & )
{
  return mLabels;
}

void QgsRasterLayerLabelProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  // as per vector label rendering...
  QgsMapToPixel xform = context.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
  const QPointF outPt = xform.transform( label->getX(), label->getY() ).toQPointF();

  QgsTextLabelFeature *lf = qgis::down_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );
  QgsTextRenderer::drawDocument( outPt,
                                 mFormat, lf->document(), lf->documentMetrics(), context, Qgis::TextHorizontalAlignment::Left,
                                 label->getAlpha(), Qgis::TextLayoutMode::Labeling );
}

void QgsRasterLayerLabelProvider::startRender( QgsRenderContext &context )
{
  if ( mFormat.dataDefinedProperties().hasActiveProperties() )
    mFormat.updateDataDefinedProperties( context );
  QgsAbstractLabelProvider::startRender( context );
}

///@cond PRIVATE
// RAII properties restorer for QgsRasterDataProvider
struct RasterProviderSettingsRestorer
{
  QgsRasterDataProvider *mProvider;
  const bool mProviderResampling;
  const Qgis::RasterResamplingMethod mZoomedOutMethod;
  const double mMaxOversampling;

  RasterProviderSettingsRestorer( QgsRasterDataProvider *provider )
    : mProvider( provider )
    , mProviderResampling( provider->isProviderResamplingEnabled() )
    , mZoomedOutMethod( provider->zoomedOutResamplingMethod() )
    , mMaxOversampling( provider->maxOversampling() ) {}

  ~RasterProviderSettingsRestorer()
  {
    mProvider->enableProviderResampling( mProviderResampling );
    mProvider->setZoomedOutResamplingMethod( mZoomedOutMethod );
    mProvider->setMaxOversampling( mMaxOversampling );
  }
};
///@endcond

void QgsRasterLayerLabelProvider::generateLabels( QgsRenderContext &context, QgsRasterPipe *pipe, QgsRasterViewPort *rasterViewPort, QgsRasterLayerRendererFeedback *feedback )
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

  // iterate through blocks, directly over the provider.
  QgsRasterIterator iterator( provider );

  const QSize maxTileSize {provider->maximumTileSize()};
  iterator.setMaximumTileWidth( maxTileSize.width() );
  iterator.setMaximumTileHeight( maxTileSize.height() );
  iterator.setSnapToPixelFactor( mResampleOver );

  // we need to calculate the visible portion of the layer, in the original (layer) CRS:
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

  const int maxNumLabels = mThinningSettings.limitNumberOfLabelsEnabled() ? mThinningSettings.maximumNumberLabels() : 0;

  // calculate the portion of the raster which is actually visible in the map
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

  const double rasterUnitsPerPixelX = provider->extent().width() / provider->xSize() * mResampleOver;
  const double rasterUnitsPerPixelY = provider->extent().height() / provider->ySize() * mResampleOver;

  const double minPixelSizePainterUnits = context.convertToPainterUnits( mThinningSettings.minimumFeatureSize(), Qgis::RenderUnit::Millimeters );
  if ( minPixelSizePainterUnits > 0 )
  {
    // calculate size in painter units of one raster pixel
    QgsPointXY p1( rasterSubRegion.xMinimum(), rasterSubRegion.yMinimum() );
    QgsPointXY p2( rasterSubRegion.xMinimum() + rasterSubRegion.width() / subRegionWidth,
                   rasterSubRegion.yMinimum() + rasterSubRegion.height() / subRegionHeight );
    try
    {
      p1 = context.coordinateTransform().transform( p1 );
      p2 = context.coordinateTransform().transform( p2 );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform raster pixel to map crs" ) );
      return;
    }
    const QgsPointXY p1PainterUnits = context.mapToPixel().transform( p1 );
    const QgsPointXY p2PainterUnits = context.mapToPixel().transform( p2 );
    const double painterUnitsPerRasterPixel = std::max( std::fabs( p1PainterUnits.x() - p2PainterUnits.x() ),
        std::fabs( p1PainterUnits.y() - p2PainterUnits.y() ) ) * mResampleOver;
    if ( painterUnitsPerRasterPixel < minPixelSizePainterUnits )
      return;
  }

  iterator.startRasterRead( mBandNumber, subRegionWidth, subRegionHeight, rasterSubRegion, feedback );

  const QgsNumericFormatContext numericContext;
  QgsNumericFormat *numericFormat = mNumericFormat.get();

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRectangle blockExtent;
  std::unique_ptr< QgsRasterBlock > block;
  bool isNoData = false;
  int numberLabels = 0;

  RasterProviderSettingsRestorer restorer( provider );
  if ( mResampleOver > 1 )
  {
    provider->enableProviderResampling( true );
    provider->setZoomedOutResamplingMethod( mResampleMethod );
    provider->setMaxOversampling( mResampleOver );
  }

  while ( iterator.next( mBandNumber, iterCols, iterRows, iterLeft, iterTop, blockExtent ) )
  {
    if ( feedback && feedback->isCanceled() )
      return;

    const int resampledColumns = iterCols / mResampleOver;
    const int resampledRows = iterRows / mResampleOver;
    block.reset( provider->block( mBandNumber, blockExtent, resampledColumns, resampledRows, feedback ) );

    double currentY = blockExtent.yMaximum() - 0.5 * rasterUnitsPerPixelY;

    for ( int row = 0; row < resampledRows; row++ )
    {
      if ( feedback && feedback->isCanceled() )
        return;

      double currentX = blockExtent.xMinimum() + 0.5 * rasterUnitsPerPixelX;

      for ( int column = 0; column < resampledColumns; column++ )
      {
        const double value = block->valueAndNoData( row, column, isNoData );
        if ( !isNoData )
        {
          try
          {
            QgsPoint pixelCenter( currentX, currentY );
            pixelCenter.transform( context.coordinateTransform() );

            addLabel( pixelCenter,
                      numericFormat->formatDouble( value, numericContext ),
                      context );
            numberLabels++;
            if ( maxNumLabels > 0 && numberLabels >= maxNumLabels )
              return;
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Could not transform raster pixel center to map crs" ) );
          }
        }
        currentX += rasterUnitsPerPixelX;
      }
      currentY -= rasterUnitsPerPixelY;
    }
  }
}

//
// QgsAbstractRasterLayerLabeling
//

void QgsAbstractRasterLayerLabeling::multiplyOpacity( double )
{

}

bool QgsAbstractRasterLayerLabeling::isInScaleRange( double ) const
{
  return true;
}

QgsAbstractRasterLayerLabeling *QgsAbstractRasterLayerLabeling::createFromElement( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString type = element.attribute( QStringLiteral( "type" ) );
  if ( type == QLatin1String( "simple" ) )
  {
    return QgsRasterLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

void QgsAbstractRasterLayerLabeling::toSld( QDomNode &parent, const QVariantMap &props ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( props )
  QDomDocument doc = parent.ownerDocument();
  parent.appendChild( doc.createComment( QStringLiteral( "SE Export for %1 not implemented yet" ).arg( type() ) ) );
}

bool QgsAbstractRasterLayerLabeling::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

//
// QgsRasterLayerSimpleLabeling
//


QgsRasterLayerSimpleLabeling::QgsRasterLayerSimpleLabeling()
  : mNumericFormat( std::make_unique< QgsBasicNumericFormat >() )
{
  mThinningSettings.setMaximumNumberLabels( 4000 );
  mThinningSettings.setLimitNumberLabelsEnabled( true );
  mThinningSettings.setMinimumFeatureSize( 8 );
}

QgsRasterLayerSimpleLabeling::~QgsRasterLayerSimpleLabeling() = default;

QString QgsRasterLayerSimpleLabeling::type() const
{
  return QStringLiteral( "simple" );
}

QgsRasterLayerSimpleLabeling *QgsRasterLayerSimpleLabeling::clone() const
{
  std::unique_ptr< QgsRasterLayerSimpleLabeling > res = std::make_unique< QgsRasterLayerSimpleLabeling >();
  res->setTextFormat( mTextFormat );

  if ( mNumericFormat )
    res->mNumericFormat.reset( mNumericFormat->clone() );

  res->setBand( mBandNumber );
  res->setPriority( mPriority );
  res->setPlacementSettings( mPlacementSettings );
  res->setThinningSettings( mThinningSettings );
  res->setZIndex( mZIndex );
  res->setScaleBasedVisibility( mScaleVisibility );
  res->setMaximumScale( mMaximumScale );
  res->setMinimumScale( mMinimumScale );
  res->setResampleMethod( mResampleMethod );
  res->setResampleOver( mResampleOver );

  return res.release();
}

std::unique_ptr< QgsRasterLayerLabelProvider > QgsRasterLayerSimpleLabeling::provider( QgsRasterLayer *layer ) const
{
  std::unique_ptr< QgsRasterLayerLabelProvider > res = std::make_unique< QgsRasterLayerLabelProvider >( layer );
  res->setTextFormat( mTextFormat );
  res->setBand( mBandNumber );
  res->setPriority( mPriority );
  res->setPlacementSettings( mPlacementSettings );
  res->setZIndex( mZIndex );
  res->setThinningSettings( mThinningSettings );
  res->setResampleMethod( mResampleMethod );
  res->setResampleOver( mResampleOver );
  if ( mNumericFormat )
  {
    res->setNumericFormat( std::unique_ptr< QgsNumericFormat >( mNumericFormat->clone() ) );
  }
  return res;
}

QDomElement QgsRasterLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "simple" ) );
  elem.setAttribute( QStringLiteral( "band" ), mBandNumber );
  elem.setAttribute( QStringLiteral( "priority" ), mPriority );
  elem.setAttribute( QStringLiteral( "zIndex" ), mZIndex );

  if ( mResampleOver > 1 )
  {
    elem.setAttribute( QStringLiteral( "resampleOver" ), mResampleOver );
  }
  elem.setAttribute( QStringLiteral( "resampleMethod" ), qgsEnumValueToKey( mResampleMethod ) );

  {
    QDomElement textFormatElem = doc.createElement( QStringLiteral( "textFormat" ) );
    textFormatElem.appendChild( mTextFormat.writeXml( doc, context ) );
    elem.appendChild( textFormatElem );
  }

  {
    QDomElement numericFormatElem = doc.createElement( QStringLiteral( "numericFormat" ) );
    mNumericFormat->writeXml( numericFormatElem, doc, context );
    elem.appendChild( numericFormatElem );
  }

  {
    QDomElement placementElem = doc.createElement( QStringLiteral( "placement" ) );
    placementElem.setAttribute( QStringLiteral( "overlapHandling" ), qgsEnumValueToKey( mPlacementSettings.overlapHandling() ) );
    elem.appendChild( placementElem );
  }

  {
    QDomElement thinningElem = doc.createElement( QStringLiteral( "thinning" ) );
    thinningElem.setAttribute( QStringLiteral( "maxNumLabels" ), mThinningSettings.maximumNumberLabels() );
    thinningElem.setAttribute( QStringLiteral( "limitNumLabels" ), mThinningSettings.limitNumberOfLabelsEnabled() );
    thinningElem.setAttribute( QStringLiteral( "minFeatureSize" ), mThinningSettings.minimumFeatureSize() );
    elem.appendChild( thinningElem );
  }

  {
    QDomElement renderingElem = doc.createElement( QStringLiteral( "rendering" ) );
    renderingElem.setAttribute( QStringLiteral( "scaleVisibility" ), mScaleVisibility );
    // note the element names are "flipped" vs the member -- this is intentional, and done to match vector labeling
    renderingElem.setAttribute( QStringLiteral( "scaleMin" ), mMaximumScale );
    renderingElem.setAttribute( QStringLiteral( "scaleMax" ), mMinimumScale );
    elem.appendChild( renderingElem );
  }

  return elem;
}

bool QgsRasterLayerSimpleLabeling::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  QgsStyleTextFormatEntity entity( mTextFormat );
  if ( !visitor->visit( &entity ) )
    return false;

  return true;
}

bool QgsRasterLayerSimpleLabeling::requiresAdvancedEffects() const
{
  return mTextFormat.containsAdvancedEffects();
}

QgsRasterLayerSimpleLabeling *QgsRasterLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsRasterLayerSimpleLabeling > res = std::make_unique< QgsRasterLayerSimpleLabeling >();
  res->setBand( element.attribute( QStringLiteral( "band" ), QStringLiteral( "1" ) ).toInt() );
  res->setPriority( element.attribute( QStringLiteral( "priority" ), QStringLiteral( "0.5" ) ).toDouble() );
  res->setZIndex( element.attribute( QStringLiteral( "zIndex" ), QStringLiteral( "0" ) ).toDouble() );
  res->setResampleOver( element.attribute( QStringLiteral( "resampleOver" ), QStringLiteral( "1" ) ).toInt() );
  res->setResampleMethod( qgsEnumKeyToValue( element.attribute( QStringLiteral( "resampleMethod" ) ), Qgis::RasterResamplingMethod::Average ) );

  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "textFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    QgsTextFormat format;
    format.readXml( textFormatElem, context );
    res->setTextFormat( format );
  }

  const QDomNodeList numericFormatNodeList = element.elementsByTagName( QStringLiteral( "numericFormat" ) );
  if ( !numericFormatNodeList.isEmpty() )
  {
    const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
    res->mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
  }

  QDomElement placementElem = element.firstChildElement( QStringLiteral( "placement" ) );
  res->mPlacementSettings.setOverlapHandling( qgsEnumKeyToValue( placementElem.attribute( QStringLiteral( "overlapHandling" ) ), Qgis::LabelOverlapHandling::PreventOverlap ) );

  QDomElement thinningElem = element.firstChildElement( QStringLiteral( "thinning" ) );
  res->mThinningSettings.setMaximumNumberLabels( thinningElem.attribute( QStringLiteral( "maxNumLabels" ), QStringLiteral( "4000" ) ).toInt() );
  res->mThinningSettings.setLimitNumberLabelsEnabled( thinningElem.attribute( QStringLiteral( "limitNumLabels" ), QStringLiteral( "1" ) ).toInt() );
  res->mThinningSettings.setMinimumFeatureSize( thinningElem.attribute( QStringLiteral( "minFeatureSize" ), QStringLiteral( "8" ) ).toDouble() );

  QDomElement renderingElem = element.firstChildElement( QStringLiteral( "rendering" ) );
  // note the element names are "flipped" vs the member -- this is intentional, and done to match vector labeling
  res->mMaximumScale = renderingElem.attribute( QStringLiteral( "scaleMin" ), QStringLiteral( "0" ) ).toDouble();
  res->mMinimumScale = renderingElem.attribute( QStringLiteral( "scaleMax" ), QStringLiteral( "0" ) ).toDouble();
  res->mScaleVisibility = renderingElem.attribute( QStringLiteral( "scaleVisibility" ) ).toInt();

  return res.release();
}

QgsTextFormat QgsRasterLayerSimpleLabeling::textFormat() const
{
  return mTextFormat;
}

void QgsRasterLayerSimpleLabeling::setTextFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

const QgsNumericFormat *QgsRasterLayerSimpleLabeling::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsRasterLayerSimpleLabeling::setNumericFormat( QgsNumericFormat *format )
{
  if ( format != mNumericFormat.get() )
    mNumericFormat.reset( format );
}

double QgsRasterLayerSimpleLabeling::zIndex() const
{
  return mZIndex;
}

void QgsRasterLayerSimpleLabeling::setZIndex( double index )
{
  mZIndex = index;
}

double QgsRasterLayerSimpleLabeling::maximumScale() const
{
  return mMaximumScale;
}

void QgsRasterLayerSimpleLabeling::setMaximumScale( double scale )
{
  mMaximumScale = scale;
}

double QgsRasterLayerSimpleLabeling::minimumScale() const
{
  return mMinimumScale;
}

void QgsRasterLayerSimpleLabeling::setMinimumScale( double scale )
{
  mMinimumScale = scale;
}

bool QgsRasterLayerSimpleLabeling::hasScaleBasedVisibility() const
{
  return mScaleVisibility;
}

bool QgsRasterLayerSimpleLabeling::isInScaleRange( double scale ) const
{
  // mMinScale (denominator!) is inclusive ( >= --> In range )
  // mMaxScale (denominator!) is exclusive ( < --> In range )
  return !mScaleVisibility
         || ( ( mMinimumScale == 0 || !QgsScaleUtils::lessThanMaximumScale( scale, mMinimumScale ) )
              && ( mMaximumScale == 0 || !QgsScaleUtils::equalToOrGreaterThanMinimumScale( scale, mMaximumScale ) ) );
}

Qgis::RasterResamplingMethod QgsRasterLayerSimpleLabeling::resampleMethod() const
{
  return mResampleMethod;
}

void QgsRasterLayerSimpleLabeling::setResampleMethod( Qgis::RasterResamplingMethod method )
{
  mResampleMethod = method;
}

int QgsRasterLayerSimpleLabeling::resampleOver() const
{
  return mResampleOver;
}

void QgsRasterLayerSimpleLabeling::setResampleOver( int pixels )
{
  mResampleOver = pixels;
}

void QgsRasterLayerSimpleLabeling::setScaleBasedVisibility( bool enabled )
{
  mScaleVisibility = enabled;
}

void QgsRasterLayerSimpleLabeling::multiplyOpacity( double opacityFactor )
{
  mTextFormat.multiplyOpacity( opacityFactor );
}

QgsAbstractRasterLayerLabeling *QgsAbstractRasterLayerLabeling::defaultLabelingForLayer( QgsRasterLayer *layer )
{
  std::unique_ptr< QgsRasterLayerSimpleLabeling > res = std::make_unique< QgsRasterLayerSimpleLabeling >();
  res->setTextFormat( QgsStyle::defaultTextFormatForProject( layer->project() ) );
  res->setBand( 1 );
  return res.release();
}
