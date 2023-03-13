/***************************************************************************
                             qgslayoutitemelevationprofile.cpp
                             ---------------------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemregistry.h"
#include "qgsplot.h"
#include "qgslayout.h"
#include "qgsmessagelog.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgscurve.h"
#include "qgsprofilerequest.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgsprofilerenderer.h"
#include "qgslayoututils.h"
#include "qgsvectorlayer.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"

#include <QTimer>

#define CACHE_SIZE_LIMIT 5000

///@cond PRIVATE
class QgsLayoutItemElevationProfilePlot : public Qgs2DPlot
{
  public:

    QgsLayoutItemElevationProfilePlot()
    {
    }

    void setRenderer( QgsProfilePlotRenderer *renderer )
    {
      mRenderer = renderer;
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      if ( mRenderer )
      {
        rc.painter()->translate( plotArea.left(), plotArea.top() );
        mRenderer->render( rc, plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum() );
        rc.painter()->translate( -plotArea.left(), -plotArea.top() );
      }
    }

  private:

    QgsProfilePlotRenderer *mRenderer = nullptr;

};
///@endcond PRIVATE

QgsLayoutItemElevationProfile::QgsLayoutItemElevationProfile( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mPlot( std::make_unique< QgsLayoutItemElevationProfilePlot >() )
{
  mBackgroundUpdateTimer = new QTimer( this );
  mBackgroundUpdateTimer->setSingleShot( true );
  connect( mBackgroundUpdateTimer, &QTimer::timeout, this, &QgsLayoutItemElevationProfile::recreateCachedImageInBackground );

  setCacheMode( QGraphicsItem::NoCache );

  if ( mLayout )
  {
    connect( mLayout, &QgsLayout::refreshed, this, &QgsLayoutItemElevationProfile::invalidateCache );
  }

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [ = ]
  {
    invalidateCache();
  } );

  //default to no background
  setBackgroundEnabled( false );
}

QgsLayoutItemElevationProfile::~QgsLayoutItemElevationProfile()
{
  if ( mRenderJob )
  {
    disconnect( mRenderJob.get(), &QgsProfilePlotRenderer::generationFinished, this, &QgsLayoutItemElevationProfile::profileGenerationFinished );
    emit backgroundTaskCountChanged( 0 );
    mRenderJob->cancelGeneration(); // blocks
    mPainter->end();
  }
}

QgsLayoutItemElevationProfile *QgsLayoutItemElevationProfile::create( QgsLayout *layout )
{
  return new QgsLayoutItemElevationProfile( layout );
}

int QgsLayoutItemElevationProfile::type() const
{
  return QgsLayoutItemRegistry::LayoutElevationProfile;
}

QIcon QgsLayoutItemElevationProfile::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mLayoutItemElevationProfile.svg" ) );
}

void QgsLayoutItemElevationProfile::refreshDataDefinedProperty( DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;

  if ( ( property == QgsLayoutObject::ElevationProfileTolerance || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileTolerance ) ) )
  {
    double value = mTolerance;

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileTolerance, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile tolerance expression eval error" ) );
    }
    else
    {
      mTolerance = value;
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMinimumDistance || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMinimumDistance ) ) )
  {
    double value = mPlot->xMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMinimumDistance, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile minimum distance expression eval error" ) );
    }
    else
    {
      mPlot->setXMinimum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMaximumDistance || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMaximumDistance ) ) )
  {
    double value = mPlot->xMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMaximumDistance, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile maximum distance expression eval error" ) );
    }
    else
    {
      mPlot->setXMaximum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMinimumElevation || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMinimumElevation ) ) )
  {
    double value = mPlot->yMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMinimumElevation, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile minimum elevation expression eval error" ) );
    }
    else
    {
      mPlot->setYMinimum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMaximumElevation || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMaximumElevation ) ) )
  {
    double value = mPlot->yMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMaximumElevation, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile maximum elevation expression eval error" ) );
    }
    else
    {
      mPlot->setYMaximum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceMajorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceMajorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceMajorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis major interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setGridIntervalMajor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceMinorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceMinorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceMinorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis minor interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setGridIntervalMinor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceLabelInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceLabelInterval ) ) )
  {
    double value = mPlot->xAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceLabelInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis label interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setLabelInterval( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationMajorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationMajorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationMajorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis major interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setGridIntervalMajor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationMinorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationMinorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationMinorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis minor interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setGridIntervalMinor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationLabelInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationLabelInterval ) ) )
  {
    double value = mPlot->yAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationLabelInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis label interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setLabelInterval( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginLeft || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginLeft ) ) )
  {
    double value = mPlot->margins().left();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginLeft, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile left margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setLeft( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginRight || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginRight ) ) )
  {
    double value = mPlot->margins().right();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginRight, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile right margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setRight( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginTop || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginTop ) ) )
  {
    double value = mPlot->margins().top();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginTop, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile top margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setTop( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginBottom || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginBottom ) ) )
  {
    double value = mPlot->margins().bottom();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginBottom, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile bottom margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setBottom( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( forceUpdate )
  {
    mCacheInvalidated = true;

    refreshItemSize();
    update();
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

QgsLayoutItem::Flags QgsLayoutItemElevationProfile::itemFlags() const
{
  return QgsLayoutItem::FlagOverridesPaint;
}

Qgs2DPlot *QgsLayoutItemElevationProfile::plot()
{
  return mPlot.get();
}

const Qgs2DPlot *QgsLayoutItemElevationProfile::plot() const
{
  return mPlot.get();
}

QList<QgsMapLayer *> QgsLayoutItemElevationProfile::layers() const
{
  return _qgis_listRefToRaw( mLayers );
}

void QgsLayoutItemElevationProfile::setLayers( const QList<QgsMapLayer *> &layers )
{
  if ( layers == _qgis_listRefToRaw( mLayers ) )
    return;

  mLayers = _qgis_listRawToRef( layers );
  invalidateCache();
}

void QgsLayoutItemElevationProfile::setProfileCurve( QgsCurve *curve )
{
  mCurve.reset( curve );
  invalidateCache();
}

QgsCurve *QgsLayoutItemElevationProfile::profileCurve() const
{
  return mCurve.get();
}

void QgsLayoutItemElevationProfile::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCrs == crs )
    return;

  mCrs = crs;
  invalidateCache();
}

QgsCoordinateReferenceSystem QgsLayoutItemElevationProfile::crs() const
{
  return mCrs;
}

void QgsLayoutItemElevationProfile::setTolerance( double tolerance )
{
  if ( mTolerance == tolerance )
    return;

  mTolerance = tolerance;
  invalidateCache();
}

double QgsLayoutItemElevationProfile::tolerance() const
{
  return mTolerance;
}

void QgsLayoutItemElevationProfile::setAtlasDriven( bool enabled )
{
  mAtlasDriven = enabled;
}

QgsProfileRequest QgsLayoutItemElevationProfile::profileRequest() const
{
  QgsProfileRequest req( mCurve ? mCurve.get()->clone() : nullptr );

  req.setCrs( mCrs );
  req.setTolerance( mTolerance );
  req.setExpressionContext( createExpressionContext() );
  if ( mLayout )
  {
    if ( QgsProject *project = mLayout->project() )
    {
      req.setTransformContext( project->transformContext() );
      if ( QgsAbstractTerrainProvider *provider = project->elevationProperties()->terrainProvider() )
      {
        req.setTerrainProvider( provider->clone() );
      }
    }
  }
  return req;
}

void QgsLayoutItemElevationProfile::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget * )
{
  if ( !mLayout || !painter || !painter->device() || !mUpdatesEnabled )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  QRectF thisPaintRect = rect();
  if ( qgsDoubleNear( thisPaintRect.width(), 0.0 ) || qgsDoubleNear( thisPaintRect.height(), 0 ) )
    return;

  if ( mLayout->renderContext().isPreviewRender() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
    rc.setExpressionContext( createExpressionContext() );

    QgsScopedQPainterState painterState( painter );
    painter->setClipRect( thisPaintRect );
    if ( !mCacheFinalImage || mCacheFinalImage->isNull() )
    {
      // No initial render available - so draw some preview text alerting user
      painter->setBrush( QBrush( QColor( 125, 125, 125, 125 ) ) );
      painter->drawRect( thisPaintRect );
      painter->setBrush( Qt::NoBrush );
      QFont messageFont;
      messageFont.setPointSize( 12 );
      painter->setFont( messageFont );
      painter->setPen( QColor( 255, 255, 255, 255 ) );
      painter->drawText( thisPaintRect, Qt::AlignCenter | Qt::AlignHCenter, tr( "Rendering profile" ) );

      if (
        ( mRenderJob && mCacheInvalidated && !mDrawingPreview ) // current job was invalidated - start a new one
        ||
        ( !mRenderJob && !mDrawingPreview ) // this is the profiles's very first paint - trigger a cache update
      )
      {

        mPreviewScaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
        mBackgroundUpdateTimer->start( 1 );
      }
    }
    else
    {
      if ( mCacheInvalidated && !mDrawingPreview )
      {
        // cache was invalidated - trigger a background update
        mPreviewScaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
        mBackgroundUpdateTimer->start( 1 );
      }

      //Background color is already included in cached image, so no need to draw

      double imagePixelWidth = mCacheFinalImage->width(); //how many pixels of the image are for the map extent?
      double scale = rect().width() / imagePixelWidth;

      QgsScopedQPainterState rotatedPainterState( painter );

      painter->scale( scale, scale );
      painter->drawImage( 0, 0, *mCacheFinalImage );
    }

    painter->setClipRect( thisPaintRect, Qt::NoClip );

    if ( frameEnabled() )
    {
      QgsLayoutItem::drawFrame( rc );
    }
  }
  else
  {
    if ( mDrawing )
      return;

    mDrawing = true;
    QPaintDevice *paintDevice = painter->device();
    if ( !paintDevice )
      return;

    QSizeF layoutSize = mLayout->convertToLayoutUnits( sizeWithUnits() );

    if ( mLayout && mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering )
      painter->setRenderHint( QPainter::LosslessImageRendering, true );

    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
    rc.setExpressionContext( createExpressionContext() );

    // Fill with background color
    if ( hasBackground() )
    {
      QgsLayoutItem::drawBackground( rc );
    }

    QgsScopedQPainterState painterState( painter );

    if ( !qgsDoubleNear( layoutSize.width(), 0.0 ) && !qgsDoubleNear( layoutSize.height(), 0.0 ) )
    {
      QgsScopedQPainterState stagedPainterState( painter );
      double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
      layoutSize *= dotsPerMM; // output size will be in dots (pixels)
      painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

      QList< QgsAbstractProfileSource * > sources;
      for ( const QgsMapLayerRef &layer : std::as_const( mLayers ) )
      {
        if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer.get() ) )
          sources.append( source );
      }

      QgsProfilePlotRenderer renderer( sources, profileRequest() );

      // TODO
      // we should be able to call renderer.start()/renderer.waitForFinished() here and
      // benefit from parallel source generation. BUT
      // for some reason the QtConcurrent::map call in start() never triggers
      // the actual background thread execution.
      // So for now just generate the results one by one
      renderer.generateSynchronously();
      mPlot->setRenderer( &renderer );

      // size must be in pixels, not layout units
      mPlot->setSize( layoutSize );

      mPlot->render( rc );

      mPlot->setRenderer( nullptr );

      painter->setClipRect( thisPaintRect, Qt::NoClip );
    }

    if ( frameEnabled() )
    {
      QgsLayoutItem::drawFrame( rc );
    }
    mDrawing = false;
  }
}

void QgsLayoutItemElevationProfile::refresh()
{
  if ( mAtlasDriven && mLayout && mLayout->reportContext().layer() )
  {
    if ( QgsVectorLayer *layer = mLayout->reportContext().layer() )
    {
      mCrs = layer->crs();
    }
    const QgsGeometry curveGeom( mLayout->reportContext().currentGeometry( mCrs ) );
    if ( const QgsAbstractGeometry *geom = curveGeom.constGet() )
    {
      if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geom->simplifiedTypeRef() ) )
      {
        mCurve.reset( curve->clone() );
      }
    }
  }
  QgsLayoutItem::refresh();
  invalidateCache();
}

void QgsLayoutItemElevationProfile::invalidateCache()
{
  if ( mDrawing )
    return;

  mCacheInvalidated = true;
  update();
}

void QgsLayoutItemElevationProfile::draw( QgsLayoutItemRenderContext & )
{
}

bool QgsLayoutItemElevationProfile::writePropertiesToElement( QDomElement &layoutProfileElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  {
    QDomElement plotElement = doc.createElement( QStringLiteral( "plot" ) );
    mPlot->writeXml( plotElement, doc, rwContext );
    layoutProfileElem.appendChild( plotElement );
  }

  layoutProfileElem.setAttribute( QStringLiteral( "tolerance" ), mTolerance );
  layoutProfileElem.setAttribute( QStringLiteral( "atlasDriven" ), mAtlasDriven ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mCrs.isValid() )
  {
    QDomElement crsElem = doc.createElement( QStringLiteral( "crs" ) );
    mCrs.writeXml( crsElem, doc );
    layoutProfileElem.appendChild( crsElem );
  }
  if ( mCurve )
  {
    QDomElement curveElem = doc.createElement( QStringLiteral( "curve" ) );
    curveElem.appendChild( doc.createTextNode( mCurve->asWkt( ) ) );
    layoutProfileElem.appendChild( curveElem );
  }

  {
    QDomElement layersElement = doc.createElement( QStringLiteral( "layers" ) );
    for ( const QgsMapLayerRef &layer : mLayers )
    {
      QDomElement layerElement = doc.createElement( QStringLiteral( "layer" ) );
      layer.writeXml( layerElement, rwContext );
      layersElement.appendChild( layerElement );
    }
    layoutProfileElem.appendChild( layersElement );
  }

  return true;
}

bool QgsLayoutItemElevationProfile::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  const QDomElement plotElement = itemElem.firstChildElement( QStringLiteral( "plot" ) );
  if ( !plotElement.isNull() )
  {
    mPlot->readXml( plotElement, context );
  }

  const QDomNodeList crsNodeList = itemElem.elementsByTagName( QStringLiteral( "crs" ) );
  QgsCoordinateReferenceSystem crs;
  if ( !crsNodeList.isEmpty() )
  {
    const QDomElement crsElem = crsNodeList.at( 0 ).toElement();
    crs.readXml( crsElem );
  }
  mCrs = crs;

  const QDomNodeList curveNodeList = itemElem.elementsByTagName( QStringLiteral( "curve" ) );
  if ( !curveNodeList.isEmpty() )
  {
    const QDomElement curveElem = curveNodeList.at( 0 ).toElement();
    const QgsGeometry curve = QgsGeometry::fromWkt( curveElem.text() );
    if ( const QgsCurve *curveGeom = qgsgeometry_cast< const QgsCurve * >( curve.constGet() ) )
    {
      mCurve.reset( curveGeom->clone() );
    }
    else
    {
      mCurve.reset();
    }
  }

  mTolerance = itemElem.attribute( QStringLiteral( "tolerance" ) ).toDouble();
  mAtlasDriven = static_cast< bool >( itemElem.attribute( QStringLiteral( "atlasDriven" ), QStringLiteral( "0" ) ).toInt() );

  {
    mLayers.clear();
    const QDomElement layersElement = itemElem.firstChildElement( QStringLiteral( "layers" ) );
    QDomElement layerElement = layersElement.firstChildElement( QStringLiteral( "layer" ) );
    while ( !layerElement.isNull() )
    {
      QgsMapLayerRef ref;
      ref.readXml( layerElement, context );
      ref.resolveWeakly( mLayout->project() );
      mLayers.append( ref );

      layerElement = layerElement.nextSiblingElement( QStringLiteral( "layer" ) );
    }
  }

  return true;
}

void QgsLayoutItemElevationProfile::recreateCachedImageInBackground()
{
  if ( mRenderJob )
  {
    disconnect( mRenderJob.get(), &QgsProfilePlotRenderer::generationFinished, this, &QgsLayoutItemElevationProfile::profileGenerationFinished );
    QgsProfilePlotRenderer *oldJob = mRenderJob.release();
    QPainter *oldPainter = mPainter.release();
    QImage *oldImage = mCacheRenderingImage.release();
    connect( oldJob, &QgsProfilePlotRenderer::generationFinished, this, [oldPainter, oldJob, oldImage]
    {
      oldJob->deleteLater();
      delete oldPainter;
      delete oldImage;
    } );
    oldJob->cancelGenerationWithoutBlocking();
  }
  else
  {
    mCacheRenderingImage.reset( nullptr );
    emit backgroundTaskCountChanged( 1 );
  }

  Q_ASSERT( !mRenderJob );
  Q_ASSERT( !mPainter );
  Q_ASSERT( !mCacheRenderingImage );

  const QSizeF layoutSize = mLayout->convertToLayoutUnits( sizeWithUnits() );
  double widthLayoutUnits = layoutSize.width();
  double heightLayoutUnits = layoutSize.height();

  int w = static_cast< int >( std::round( widthLayoutUnits * mPreviewScaleFactor ) );
  int h = static_cast< int >( std::round( heightLayoutUnits * mPreviewScaleFactor ) );

  // limit size of image for better performance
  if ( w > 5000 || h > 5000 )
  {
    if ( w > h )
    {
      w = 5000;
      h = static_cast< int>( std::round( w * heightLayoutUnits / widthLayoutUnits ) );
    }
    else
    {
      h = 5000;
      w = static_cast< int >( std::round( h * widthLayoutUnits / heightLayoutUnits ) );
    }
  }

  if ( w <= 0 || h <= 0 )
    return;

  mCacheRenderingImage.reset( new QImage( w, h, QImage::Format_ARGB32 ) );

  // set DPI of the image
  mCacheRenderingImage->setDotsPerMeterX( static_cast< int >( std::round( 1000 * w / widthLayoutUnits ) ) );
  mCacheRenderingImage->setDotsPerMeterY( static_cast< int >( std::round( 1000 * h / heightLayoutUnits ) ) );

  //start with empty fill to avoid artifacts
  mCacheRenderingImage->fill( Qt::transparent );
  if ( hasBackground() )
  {
    //Initially fill image with specified background color
    mCacheRenderingImage->fill( backgroundColor().rgba() );
  }

  mCacheInvalidated = false;
  mPainter.reset( new QPainter( mCacheRenderingImage.get() ) );

  QList< QgsAbstractProfileSource * > sources;
  for ( const QgsMapLayerRef &layer : std::as_const( mLayers ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer.get() ) )
      sources.append( source );
  }

  mRenderJob = std::make_unique< QgsProfilePlotRenderer >( sources, profileRequest() );
  connect( mRenderJob.get(), &QgsProfilePlotRenderer::generationFinished, this, &QgsLayoutItemElevationProfile::profileGenerationFinished );
  mRenderJob->startGeneration();

  mDrawingPreview = false;
}

void QgsLayoutItemElevationProfile::profileGenerationFinished()
{
  mPlot->setRenderer( mRenderJob.get() );

  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( mLayout, mPainter.get() );

  // size must be in pixels, not layout units
  mPlot->setSize( mCacheRenderingImage->size() );

  mPlot->render( rc );

  mPlot->setRenderer( nullptr );

  mPainter->end();
  mRenderJob.reset( nullptr );
  mPainter.reset( nullptr );
  mCacheFinalImage = std::move( mCacheRenderingImage );
  emit backgroundTaskCountChanged( 0 );
  update();
}

