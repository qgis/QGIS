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
#include "moc_qgslayoutitemelevationprofile.cpp"
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
#include "qgsprofilesourceregistry.h"

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
      // cppcheck-suppress danglingLifetime
      mRenderer = renderer;
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      if ( mRenderer )
      {
        rc.painter()->translate( plotArea.left(), plotArea.top() );
        mRenderer->render( rc, plotArea.width(), plotArea.height(), xMinimum() * xScale, xMaximum() * xScale, yMinimum(), yMaximum() );
        rc.painter()->translate( -plotArea.left(), -plotArea.top() );
      }
    }

    double xScale = 1;

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

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [this]
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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance ) ) )
  {
    double value = mTolerance;

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance ) ) )
  {
    double value = mPlot->xMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance ) ) )
  {
    double value = mPlot->xMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation ) ) )
  {
    double value = mPlot->yMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation ) ) )
  {
    double value = mPlot->yMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval ) ) )
  {
    double value = mPlot->xAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval ) ) )
  {
    double value = mPlot->yAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::MarginLeft || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MarginLeft ) ) )
  {
    double value = mPlot->margins().left();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MarginLeft, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::MarginRight || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MarginRight ) ) )
  {
    double value = mPlot->margins().right();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MarginRight, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::MarginTop || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MarginTop ) ) )
  {
    double value = mPlot->margins().top();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MarginTop, context, value, &ok );

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

  if ( ( property == QgsLayoutObject::DataDefinedProperty::MarginBottom || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::MarginBottom ) ) )
  {
    double value = mPlot->margins().bottom();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::MarginBottom, context, value, &ok );

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
  return QgsLayoutItem::FlagOverridesPaint | QgsLayoutItem::FlagDisableSceneCaching;
}

bool QgsLayoutItemElevationProfile::requiresRasterization() const
{
  return blendMode() != QPainter::CompositionMode_SourceOver;
}

bool QgsLayoutItemElevationProfile::containsAdvancedEffects() const
{
  return mEvaluatedOpacity < 1.0;
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
      painter->setCompositionMode( blendModeForRender() );
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

    if ( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering )
      painter->setRenderHint( QPainter::LosslessImageRendering, true );

    mPlot->xScale = QgsUnitTypes::fromUnitToUnitFactor( mDistanceUnit, mCrs.mapUnits() );

    if ( !qgsDoubleNear( layoutSize.width(), 0.0 ) && !qgsDoubleNear( layoutSize.height(), 0.0 ) )
    {
      if ( ( containsAdvancedEffects() || ( blendModeForRender() != QPainter::CompositionMode_SourceOver ) )
           && ( !( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagForceVectorOutput ) ) )
      {
        // rasterize
        double destinationDpi = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter ) * 25.4;
        double layoutUnitsInInches = mLayout ? mLayout->convertFromLayoutUnits( 1, Qgis::LayoutUnit::Inches ).length() : 1;
        int widthInPixels = static_cast< int >( std::round( boundingRect().width() * layoutUnitsInInches * destinationDpi ) );
        int heightInPixels = static_cast< int >( std::round( boundingRect().height() * layoutUnitsInInches * destinationDpi ) );
        QImage image = QImage( widthInPixels, heightInPixels, QImage::Format_ARGB32 );

        image.fill( Qt::transparent );
        image.setDotsPerMeterX( static_cast< int >( std::round( 1000 * destinationDpi / 25.4 ) ) );
        image.setDotsPerMeterY( static_cast< int >( std::round( 1000 * destinationDpi / 25.4 ) ) );
        double dotsPerMM = destinationDpi / 25.4;
        layoutSize *= dotsPerMM; // output size will be in dots (pixels)
        QPainter p( &image );
        preparePainter( &p );

        QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( mLayout, &p );
        rc.setExpressionContext( createExpressionContext() );

        p.scale( dotsPerMM, dotsPerMM );
        if ( hasBackground() )
        {
          QgsLayoutItem::drawBackground( rc );
        }

        p.scale( 1.0 / dotsPerMM, 1.0 / dotsPerMM );

        const double mapUnitsPerPixel = static_cast<double>( mPlot->xMaximum() - mPlot->xMinimum() ) * mPlot->xScale / layoutSize.width();
        rc.setMapToPixel( QgsMapToPixel( mapUnitsPerPixel ) );

        QList< QgsAbstractProfileSource * > sources;
        sources << QgsApplication::profileSourceRegistry()->profileSources();
        for ( const QgsMapLayerRef &layer : std::as_const( mLayers ) )
        {
          if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer.get() ) )
            sources.append( source );
        }

        QgsProfilePlotRenderer renderer( sources, profileRequest() );

        renderer.generateSynchronously();
        mPlot->setRenderer( &renderer );

        // size must be in pixels, not layout units
        mPlot->setSize( layoutSize );

        mPlot->render( rc );

        mPlot->setRenderer( nullptr );

        p.scale( dotsPerMM, dotsPerMM );

        if ( frameEnabled() )
        {
          QgsLayoutItem::drawFrame( rc );
        }

        QgsScopedQPainterState painterState( painter );
        painter->setCompositionMode( blendModeForRender() );
        painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots
        painter->drawImage( 0, 0, image );
        painter->scale( dotsPerMM, dotsPerMM );
      }
      else
      {
        QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
        rc.setExpressionContext( createExpressionContext() );

        // Fill with background color
        if ( hasBackground() )
        {
          QgsLayoutItem::drawBackground( rc );
        }

        QgsScopedQPainterState painterState( painter );
        QgsScopedQPainterState stagedPainterState( painter );
        double dotsPerMM = paintDevice->logicalDpiX() / 25.4;
        layoutSize *= dotsPerMM; // output size will be in dots (pixels)
        painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

        const double mapUnitsPerPixel = static_cast<double>( mPlot->xMaximum() - mPlot->xMinimum() ) * mPlot->xScale / layoutSize.width();
        rc.setMapToPixel( QgsMapToPixel( mapUnitsPerPixel ) );

        QList< QgsAbstractProfileSource * > sources;
        sources << QgsApplication::profileSourceRegistry()->profileSources();
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

        if ( frameEnabled() )
        {
          QgsLayoutItem::drawFrame( rc );
        }
      }
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

  layoutProfileElem.setAttribute( QStringLiteral( "distanceUnit" ), qgsEnumValueToKey( mDistanceUnit ) );

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

  setDistanceUnit( qgsEnumKeyToValue( itemElem.attribute( QStringLiteral( "distanceUnit" ) ), mCrs.mapUnits() ) );

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
  sources << QgsApplication::profileSourceRegistry()->profileSources();
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

  mPlot->xScale = QgsUnitTypes::fromUnitToUnitFactor( mDistanceUnit, mCrs.mapUnits() );

  const double mapUnitsPerPixel = static_cast< double >( mPlot->xMaximum() - mPlot->xMinimum() ) * mPlot->xScale /
                                  mCacheRenderingImage->size().width();
  rc.setMapToPixel( QgsMapToPixel( mapUnitsPerPixel ) );

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
  emit previewRefreshed();
}

Qgis::DistanceUnit QgsLayoutItemElevationProfile::distanceUnit() const
{
  return mDistanceUnit;
}

void QgsLayoutItemElevationProfile::setDistanceUnit( Qgis::DistanceUnit unit )
{
  mDistanceUnit = unit;

  switch ( mDistanceUnit )
  {
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Inches:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
    case Qgis::DistanceUnit::MetersGermanLegal:
      mPlot->xAxis().setLabelSuffix( QStringLiteral( " %1" ).arg( QgsUnitTypes::toAbbreviatedString( mDistanceUnit ) ) );
      break;

    case Qgis::DistanceUnit::Degrees:
      mPlot->xAxis().setLabelSuffix( QObject::tr( "°" ) );
      break;

    case Qgis::DistanceUnit::Unknown:
      mPlot->xAxis().setLabelSuffix( QString() );
      break;
  }
}

