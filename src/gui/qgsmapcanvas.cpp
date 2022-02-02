/***************************************************************************
qgsmapcanvas.cpp  -  description
------------------ -
begin                : Sun Jun 30 2002
copyright            : (C) 2002 by Gary E.Sherman
email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>

#include <QtGlobal>
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QRect>
#include <QTextStream>
#include <QResizeEvent>
#include <QScreen>
#include <QString>
#include <QStringList>
#include <QWheelEvent>
#include <QWindow>
#include <QMenu>
#include <QClipboard>
#include <QVariantAnimation>
#include <QPropertyAnimation>

#include "qgis.h"
#include "qgssettings.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsapplication.h"
#include "qgsexception.h"
#include "qgsdatumtransformdialog.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgsmaplayer.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptopixel.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderercache.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaprendererjob.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmapsettingsutils.h"
#include "qgsmessagelog.h"
#include "qgsmessageviewer.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsmapthemecollection.h"
#include "qgscoordinatetransformcontext.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmimedatautils.h"
#include "qgscustomdrophandler.h"
#include "qgsreferencedgeometry.h"
#include "qgsprojectviewsettings.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgstemporalcontroller.h"
#include "qgsruntimeprofiler.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsannotationlayer.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgslabelingresults.h"
#include "qgsmaplayerutils.h"
#include "qgssettingsregistrygui.h"
#include "qgsrendereditemresults.h"

/**
 * \ingroup gui
 * \brief Deprecated to be deleted, stuff from here should be moved elsewhere.
 * \note not available in Python bindings
*/
//TODO QGIS 4.0 - remove
class QgsMapCanvas::CanvasProperties
{
  public:

    /**
     * Constructor for CanvasProperties.
     */
    CanvasProperties() = default;

    //!Flag to indicate status of mouse button
    bool mouseButtonDown{ false };

    //! Last seen point of the mouse
    QPoint mouseLastXY;

    //! Beginning point of a rubber band
    QPoint rubberStartPoint;

    //! Flag to indicate the pan selector key is held down by user
    bool panSelectorDown{ false };
};



QgsMapCanvas::QgsMapCanvas( QWidget *parent )
  : QGraphicsView( parent )
  , mCanvasProperties( new CanvasProperties )
  , mExpressionContextScope( tr( "Map Canvas" ) )
{
  mScene = new QGraphicsScene();
  setScene( mScene );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setMouseTracking( true );
  setFocusPolicy( Qt::StrongFocus );

  mResizeTimer = new QTimer( this );
  mResizeTimer->setSingleShot( true );
  connect( mResizeTimer, &QTimer::timeout, this, &QgsMapCanvas::refresh );

  mRefreshTimer = new QTimer( this );
  mRefreshTimer->setSingleShot( true );
  connect( mRefreshTimer, &QTimer::timeout, this, &QgsMapCanvas::refreshMap );

  // create map canvas item which will show the map
  mMap = new QgsMapCanvasMap( this );

  // project handling
  connect( QgsProject::instance(), &QgsProject::readProject,
           this, &QgsMapCanvas::readProject );
  connect( QgsProject::instance(), &QgsProject::writeProject,
           this, &QgsMapCanvas::writeProject );

  connect( QgsProject::instance()->mainAnnotationLayer(), &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeChanged, this, &QgsMapCanvas::mapThemeChanged );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeRenamed, this, &QgsMapCanvas::mapThemeRenamed );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemesChanged, this, &QgsMapCanvas::projectThemesChanged );

  {
    QgsScopedRuntimeProfile profile( "Map settings initialization" );
    mSettings.setFlag( Qgis::MapSettingsFlag::DrawEditingInfo );
    mSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization );
    mSettings.setFlag( Qgis::MapSettingsFlag::RenderPartialOutput );
    mSettings.setEllipsoid( QgsProject::instance()->ellipsoid() );
    connect( QgsProject::instance(), &QgsProject::ellipsoidChanged,
             this, [ = ]
    {
      mSettings.setEllipsoid( QgsProject::instance()->ellipsoid() );
      refresh();
    } );
    mSettings.setTransformContext( QgsProject::instance()->transformContext() );
    connect( QgsProject::instance(), &QgsProject::transformContextChanged,
             this, [ = ]
    {
      mSettings.setTransformContext( QgsProject::instance()->transformContext() );
      emit transformContextChanged();
      refresh();
    } );

    connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [ = ]
    {
      QgsCoordinateReferenceSystem crs = mSettings.destinationCrs();
      crs.updateDefinition();
      if ( mSettings.destinationCrs() != crs )
      {
        // user crs has changed definition, refresh the map
        setDestinationCrs( crs );
      }
    } );
  }

  // refresh canvas when a remote svg/image has finished downloading
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsMapCanvas::redrawAllLayers );
  connect( QgsApplication::imageCache(), &QgsImageCache::remoteImageFetched, this, &QgsMapCanvas::redrawAllLayers );
  // refresh canvas when project color scheme is changed -- if layers use project colors, they need to be redrawn
  connect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, &QgsMapCanvas::redrawAllLayers );

  //segmentation parameters
  QgsSettings settings;
  double segmentationTolerance = settings.value( QStringLiteral( "qgis/segmentationTolerance" ), "0.01745" ).toDouble();
  QgsAbstractGeometry::SegmentationToleranceType toleranceType = settings.enumValue( QStringLiteral( "qgis/segmentationToleranceType" ), QgsAbstractGeometry::MaximumAngle );
  mSettings.setSegmentationTolerance( segmentationTolerance );
  mSettings.setSegmentationToleranceType( toleranceType );

  mWheelZoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  QSize s = viewport()->size();
  mSettings.setOutputSize( s );

  mSettings.setRendererUsage( Qgis::RendererUsage::View );

  setSceneRect( 0, 0, s.width(), s.height() );
  mScene->setSceneRect( QRectF( 0, 0, s.width(), s.height() ) );

  moveCanvasContents( true );

  connect( &mMapUpdateTimer, &QTimer::timeout, this, &QgsMapCanvas::mapUpdateTimeout );
  mMapUpdateTimer.setInterval( 250 );

#ifdef Q_OS_WIN
  // Enable touch event on Windows.
  // Qt on Windows needs to be told it can take touch events or else it ignores them.
  grabGesture( Qt::PinchGesture );
  grabGesture( Qt::TapAndHoldGesture );
  viewport()->setAttribute( Qt::WA_AcceptTouchEvents );
#endif

  mPreviewEffect = new QgsPreviewEffect( this );
  viewport()->setGraphicsEffect( mPreviewEffect );

  mZoomCursor = QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn );

  connect( &mAutoRefreshTimer, &QTimer::timeout, this, &QgsMapCanvas::autoRefreshTriggered );

  connect( this, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvas::updateCanvasItemPositions );

  setInteractive( false );

  // make sure we have the same default in QgsMapSettings and the scene's background brush
  // (by default map settings has white bg color, scene background brush is black)
  setCanvasColor( mSettings.backgroundColor() );

  setTemporalRange( mSettings.temporalRange() );
  refresh();
}


QgsMapCanvas::~QgsMapCanvas()
{
  if ( mMapTool )
  {
    mMapTool->deactivate();
    mMapTool = nullptr;
  }
  mLastNonZoomMapTool = nullptr;

  cancelJobs();

  // delete canvas items prior to deleting the canvas
  // because they might try to update canvas when it's
  // already being destructed, ends with segfault
  qDeleteAll( mScene->items() );

  mScene->deleteLater();  // crashes in python tests on windows

  delete mCache;
}


void QgsMapCanvas::cancelJobs()
{

  // rendering job may still end up writing into canvas map item
  // so kill it before deleting canvas items
  if ( mJob )
  {
    whileBlocking( mJob )->cancel();
    delete mJob;
    mJob = nullptr;
  }

  QList< QgsMapRendererQImageJob * >::const_iterator previewJob = mPreviewJobs.constBegin();
  for ( ; previewJob != mPreviewJobs.constEnd(); ++previewJob )
  {
    if ( *previewJob )
    {
      whileBlocking( *previewJob )->cancel();
      delete *previewJob;
    }
  }
}


void QgsMapCanvas::setMagnificationFactor( double factor, const QgsPointXY *center )
{
  // do not go higher or lower than min max magnification ratio
  double magnifierMin = QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  factor = std::clamp( factor, magnifierMin, magnifierMax );

  // the magnifier widget is in integer percent
  if ( !qgsDoubleNear( factor, mSettings.magnificationFactor(), 0.01 ) )
  {
    mSettings.setMagnificationFactor( factor, center );
    refresh();
    emit magnificationChanged( factor );
  }
}

double QgsMapCanvas::magnificationFactor() const
{
  return mSettings.magnificationFactor();
}

void QgsMapCanvas::enableAntiAliasing( bool flag )
{
  mSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, flag );
  mSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, flag );
}

bool QgsMapCanvas::antiAliasingEnabled() const
{
  return mSettings.testFlag( Qgis::MapSettingsFlag::Antialiasing );
}

void QgsMapCanvas::enableMapTileRendering( bool flag )
{
  mSettings.setFlag( Qgis::MapSettingsFlag::RenderMapTile, flag );
}

QgsMapLayer *QgsMapCanvas::layer( int index )
{
  QList<QgsMapLayer *> layers = mapSettings().layers();
  if ( index >= 0 && index < layers.size() )
    return layers[index];
  else
    return nullptr;
}

QgsMapLayer *QgsMapCanvas::layer( const QString &id )
{
  // first check for layers from canvas map settings
  const QList<QgsMapLayer *> layers = mapSettings().layers();
  for ( QgsMapLayer *layer : layers )
  {
    if ( layer && layer->id() == id )
      return layer;
  }

  // else fallback to searching project layers
  // TODO: allow a specific project to be associated with a canvas!
  return QgsProject::instance()->mapLayer( id );
}

void QgsMapCanvas::setCurrentLayer( QgsMapLayer *layer )
{
  if ( mCurrentLayer == layer )
    return;

  mCurrentLayer = layer;
  emit currentLayerChanged( layer );
}

double QgsMapCanvas::scale() const
{
  return mapSettings().scale();
}

bool QgsMapCanvas::isDrawing()
{
  return nullptr != mJob;
} // isDrawing

// return the current coordinate transform based on the extents and
// device size
const QgsMapToPixel *QgsMapCanvas::getCoordinateTransform()
{
  return &mapSettings().mapToPixel();
}

void QgsMapCanvas::setLayers( const QList<QgsMapLayer *> &layers )
{
  // following a theme => request denied!
  if ( !mTheme.isEmpty() )
    return;

  setLayersPrivate( layers );
}

void QgsMapCanvas::setLayersPrivate( const QList<QgsMapLayer *> &layers )
{
  QList<QgsMapLayer *> oldLayers = mSettings.layers();

  // update only if needed
  if ( layers == oldLayers )
    return;

  const auto constOldLayers = oldLayers;
  for ( QgsMapLayer *layer : constOldLayers )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
    disconnect( layer, &QgsMapLayer::autoRefreshIntervalChanged, this, &QgsMapCanvas::updateAutoRefreshTimer );
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &QgsMapCanvas::selectionChangedSlot );
    }
  }

  mSettings.setLayers( layers );

  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( !layer )
      continue;
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
    connect( layer, &QgsMapLayer::autoRefreshIntervalChanged, this, &QgsMapCanvas::updateAutoRefreshTimer );
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      connect( vlayer, &QgsVectorLayer::selectionChanged, this, &QgsMapCanvas::selectionChangedSlot );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Layers have changed, refreshing" ), 2 );
  emit layersChanged();

  updateAutoRefreshTimer();
  refresh();
}


const QgsMapSettings &QgsMapCanvas::mapSettings() const
{
  return mSettings;
}

void QgsMapCanvas::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mSettings.destinationCrs() == crs )
    return;

  // try to reproject current extent to the new one
  QgsRectangle rect;
  if ( !mSettings.visibleExtent().isEmpty() )
  {
    QgsCoordinateTransform transform( mSettings.destinationCrs(), crs, QgsProject::instance() );
    transform.setBallparkTransformsAreAppropriate( true );
    try
    {
      rect = transform.transformBoundingBox( mSettings.visibleExtent() );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e )
      QgsDebugMsg( QStringLiteral( "Transform error caught: %1" ).arg( e.what() ) );
    }
  }

  if ( !rect.isEmpty() )
  {
    // we will be manually calling updateCanvasItemPositions() later, AFTER setting the updating the mSettings destination CRS, and we don't
    // want to do that twice!
    mBlockItemPositionUpdates++;
    setExtent( rect );
    mBlockItemPositionUpdates--;
  }

  mSettings.setDestinationCrs( crs );
  updateScale();
  updateCanvasItemPositions();

  QgsDebugMsgLevel( QStringLiteral( "refreshing after destination CRS changed" ), 2 );
  refresh();

  emit destinationCrsChanged();
}

void QgsMapCanvas::setTemporalController( QgsTemporalController *controller )
{
  if ( mController )
    disconnect( mController, &QgsTemporalController::updateTemporalRange, this, &QgsMapCanvas::setTemporalRange );

  mController = controller;
  connect( mController, &QgsTemporalController::updateTemporalRange, this, &QgsMapCanvas::setTemporalRange );
}

const QgsTemporalController *QgsMapCanvas::temporalController() const
{
  return mController;
}

void QgsMapCanvas::setMapSettingsFlags( Qgis::MapSettingsFlags flags )
{
  mSettings.setFlags( flags );
  clearCache();
  refresh();
}

const QgsLabelingResults *QgsMapCanvas::labelingResults( bool allowOutdatedResults ) const
{
  if ( !allowOutdatedResults && mLabelingResultsOutdated )
    return nullptr;

  return mLabelingResults.get();
}

const QgsRenderedItemResults *QgsMapCanvas::renderedItemResults( bool allowOutdatedResults ) const
{
  if ( !allowOutdatedResults && mRenderedItemResultsOutdated )
    return nullptr;

  return mRenderedItemResults.get();
}

void QgsMapCanvas::setCachingEnabled( bool enabled )
{
  if ( enabled == isCachingEnabled() )
    return;

  if ( mJob && mJob->isActive() )
  {
    // wait for the current rendering to finish, before touching the cache
    mJob->waitForFinished();
  }

  if ( enabled )
  {
    mCache = new QgsMapRendererCache;
  }
  else
  {
    delete mCache;
    mCache = nullptr;
  }
  mPreviousRenderedItemResults.reset();
}

bool QgsMapCanvas::isCachingEnabled() const
{
  return nullptr != mCache;
}

void QgsMapCanvas::clearCache()
{
  if ( mCache )
    mCache->clear();

  if ( mPreviousRenderedItemResults )
    mPreviousRenderedItemResults.reset();
  if ( mRenderedItemResults )
    mRenderedItemResults.reset();
}

void QgsMapCanvas::setParallelRenderingEnabled( bool enabled )
{
  mUseParallelRendering = enabled;
}

bool QgsMapCanvas::isParallelRenderingEnabled() const
{
  return mUseParallelRendering;
}

void QgsMapCanvas::setMapUpdateInterval( int timeMilliseconds )
{
  mMapUpdateTimer.setInterval( timeMilliseconds );
}

int QgsMapCanvas::mapUpdateInterval() const
{
  return mMapUpdateTimer.interval();
}


QgsMapLayer *QgsMapCanvas::currentLayer()
{
  return mCurrentLayer;
}

QgsExpressionContextScope *QgsMapCanvas::defaultExpressionContextScope() const
{
  QgsExpressionContextScope *s = new QgsExpressionContextScope( QObject::tr( "Map Canvas" ) );
  s->setVariable( QStringLiteral( "canvas_cursor_point" ), QgsGeometry::fromPointXY( cursorPoint() ), true );
  return s;
}

QgsExpressionContext QgsMapCanvas::createExpressionContext() const
{
  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                    << QgsExpressionContextUtils::atlasScope( nullptr )
                    << QgsExpressionContextUtils::mapSettingsScope( mSettings );
  if ( QgsExpressionContextScopeGenerator *generator = dynamic_cast< QgsExpressionContextScopeGenerator * >( mController ) )
  {
    expressionContext << generator->createExpressionContextScope();
  }
  expressionContext << defaultExpressionContextScope()
                    << new QgsExpressionContextScope( mExpressionContextScope );
  return expressionContext;
}

void QgsMapCanvas::refresh()
{
  if ( !mSettings.hasValidSettings() )
  {
    QgsDebugMsgLevel( QStringLiteral( "CANVAS refresh - invalid settings -> nothing to do" ), 2 );
    return;
  }

  if ( !mRenderFlag || mFrozen )
  {
    QgsDebugMsgLevel( QStringLiteral( "CANVAS render flag off" ), 2 );
    return;
  }

  if ( mRefreshScheduled )
  {
    QgsDebugMsgLevel( QStringLiteral( "CANVAS refresh already scheduled" ), 2 );
    return;
  }

  mRefreshScheduled = true;

  QgsDebugMsgLevel( QStringLiteral( "CANVAS refresh scheduling" ), 2 );

  // schedule a refresh
  mRefreshTimer->start( 1 );

  mLabelingResultsOutdated = true;
  mRenderedItemResultsOutdated = true;
}

void QgsMapCanvas::refreshMap()
{
  Q_ASSERT( mRefreshScheduled );

  QgsDebugMsgLevel( QStringLiteral( "CANVAS refresh!" ), 3 );

  stopRendering(); // if any...
  stopPreviewJobs();

  mSettings.setExpressionContext( createExpressionContext() );
  mSettings.setPathResolver( QgsProject::instance()->pathResolver() );

  if ( !mTheme.isEmpty() )
  {
    // IMPORTANT: we MUST set the layer style overrides here! (At the time of writing this
    // comment) retrieving layer styles from the theme collection gives an XML snapshot of the
    // current state of the style. If we had stored the style overrides earlier (such as in
    // mapThemeChanged slot) then this xml could be out of date...
    // TODO: if in future QgsMapThemeCollection::mapThemeStyleOverrides is changed to
    // just return the style name, we can instead set the overrides in mapThemeChanged and not here
    mSettings.setLayerStyleOverrides( QgsProject::instance()->mapThemeCollection()->mapThemeStyleOverrides( mTheme ) );
  }

  // render main annotation layer above all other layers
  QgsMapSettings renderSettings = mSettings;
  QList<QgsMapLayer *> allLayers = renderSettings.layers();
  allLayers.insert( 0, QgsProject::instance()->mainAnnotationLayer() );
  renderSettings.setLayers( allLayers );

  // create the renderer job
  Q_ASSERT( !mJob );
  mJobCanceled = false;
  if ( mUseParallelRendering )
    mJob = new QgsMapRendererParallelJob( renderSettings );
  else
    mJob = new QgsMapRendererSequentialJob( renderSettings );

  connect( mJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::rendererJobFinished );
  mJob->setCache( mCache );
  mJob->setLayerRenderingTimeHints( mLastLayerRenderTime );

  mJob->start();

  // from now on we can accept refresh requests again
  // this must be reset only after the job has been started, because
  // some providers (yes, it's you WCS and AMS!) during preparation
  // do network requests and start an internal event loop, which may
  // end up calling refresh() and would schedule another refresh,
  // deleting the one we have just started.
  mRefreshScheduled = false;

  mMapUpdateTimer.start();

  emit renderStarting();
}

void QgsMapCanvas::mapThemeChanged( const QString &theme )
{
  if ( theme == mTheme )
  {
    // set the canvas layers to match the new layers contained in the map theme
    // NOTE: we do this when the theme layers change and not when we are refreshing the map
    // as setLayers() sets up necessary connections to handle changes to the layers
    setLayersPrivate( QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayers( mTheme ) );
    // IMPORTANT: we don't set the layer style overrides here! (At the time of writing this
    // comment) retrieving layer styles from the theme collection gives an XML snapshot of the
    // current state of the style. If changes were made to the style then this xml
    // snapshot goes out of sync...
    // TODO: if in future QgsMapThemeCollection::mapThemeStyleOverrides is changed to
    // just return the style name, we can instead set the overrides here and not in refreshMap()

    clearCache();
    refresh();
  }
}

void QgsMapCanvas::mapThemeRenamed( const QString &theme, const QString &newTheme )
{
  if ( mTheme.isEmpty() || theme != mTheme )
  {
    return;
  }

  setTheme( newTheme );
  refresh();
}

void QgsMapCanvas::rendererJobFinished()
{
  QgsDebugMsgLevel( QStringLiteral( "CANVAS finish! %1" ).arg( !mJobCanceled ), 2 );

  mMapUpdateTimer.stop();

  notifyRendererErrors( mJob->errors() );

  if ( !mJobCanceled )
  {
    // take labeling results before emitting renderComplete, so labeling map tools
    // connected to signal work with correct results
    if ( !mJob->usedCachedLabels() )
    {
      mLabelingResults.reset( mJob->takeLabelingResults() );
    }
    mLabelingResultsOutdated = false;

    std::unique_ptr< QgsRenderedItemResults > renderedItemResults( mJob->takeRenderedItemResults() );
    // if a layer was redrawn from the cached version, we should copy any existing rendered item results from that layer
    if ( mRenderedItemResults )
    {
      renderedItemResults->transferResults( mRenderedItemResults.get(), mJob->layersRedrawnFromCache() );
    }
    if ( mPreviousRenderedItemResults )
    {
      // also transfer any results from previous renders which happened before this
      renderedItemResults->transferResults( mPreviousRenderedItemResults.get(), mJob->layersRedrawnFromCache() );
    }

    if ( mCache && !mPreviousRenderedItemResults )
      mPreviousRenderedItemResults = std::make_unique< QgsRenderedItemResults >( mJob->mapSettings().extent() );

    if ( mRenderedItemResults && mPreviousRenderedItemResults )
    {
      // for other layers which ARE present in the most recent rendered item results BUT were not part of this render, we
      // store the results in a temporary store in case they are later switched back on and the layer's image is taken
      // from the cache
      mPreviousRenderedItemResults->transferResults( mRenderedItemResults.get() );
    }
    if ( mPreviousRenderedItemResults )
    {
      mPreviousRenderedItemResults->eraseResultsFromLayers( mJob->mapSettings().layerIds() );
    }

    mRenderedItemResults = std::move( renderedItemResults );
    mRenderedItemResultsOutdated = false;

    QImage img = mJob->renderedImage();

    // emit renderComplete to get our decorations drawn
    QPainter p( &img );
    emit renderComplete( &p );

    if ( QgsMapRendererJob::settingsLogCanvasRefreshEvent.value() )
    {
      QString logMsg = tr( "Canvas refresh: %1 ms" ).arg( mJob->renderingTime() );
      QgsMessageLog::logMessage( logMsg, tr( "Rendering" ) );
    }

    if ( mDrawRenderingStats )
    {
      int w = img.width(), h = img.height();
      QFont fnt = p.font();
      fnt.setBold( true );
      p.setFont( fnt );
      int lh = p.fontMetrics().height() * 2;
      QRect r( 0, h - lh, w, lh );
      p.setPen( Qt::NoPen );
      p.setBrush( QColor( 0, 0, 0, 110 ) );
      p.drawRect( r );
      p.setPen( Qt::white );
      QString msg = QStringLiteral( "%1 :: %2 ms" ).arg( mUseParallelRendering ? QStringLiteral( "PARALLEL" ) : QStringLiteral( "SEQUENTIAL" ) ).arg( mJob->renderingTime() );
      p.drawText( r, msg, QTextOption( Qt::AlignCenter ) );
    }

    p.end();

    mMap->setContent( img, imageRect( img, mSettings ) );

    mLastLayerRenderTime.clear();
    const auto times = mJob->perLayerRenderingTime();
    for ( auto it = times.constBegin(); it != times.constEnd(); ++it )
    {
      mLastLayerRenderTime.insert( it.key()->id(), it.value() );
    }
    if ( mUsePreviewJobs && !mRefreshAfterJob )
      startPreviewJobs();
  }
  else
  {
    mRefreshAfterJob = false;
  }

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;

  emit mapCanvasRefreshed();

  if ( mRefreshAfterJob )
  {
    mRefreshAfterJob = false;
    clearTemporalCache();
    clearElevationCache();
    refresh();
  }
}

void QgsMapCanvas::previewJobFinished()
{
  QgsMapRendererQImageJob *job = qobject_cast<QgsMapRendererQImageJob *>( sender() );
  Q_ASSERT( job );

  if ( mMap )
  {
    mMap->addPreviewImage( job->renderedImage(), job->mapSettings().extent() );
    mPreviewJobs.removeAll( job );

    int number = job->property( "number" ).toInt();
    if ( number < 8 )
    {
      startPreviewJob( number + 1 );
    }

    delete job;
  }
}

QgsRectangle QgsMapCanvas::imageRect( const QImage &img, const QgsMapSettings &mapSettings )
{
  // This is a hack to pass QgsMapCanvasItem::setRect what it
  // expects (encoding of position and size of the item)
  const QgsMapToPixel &m2p = mapSettings.mapToPixel();
  QgsPointXY topLeft = m2p.toMapCoordinates( 0, 0 );
#ifdef QGISDEBUG
  // do not assert this, since it might lead to crashes when changing screen while rendering
  if ( img.devicePixelRatio() != mapSettings.devicePixelRatio() )
  {
    QgsLogger::warning( QStringLiteral( "The renderer map has a wrong device pixel ratio" ) );
  }
#endif
  double res = m2p.mapUnitsPerPixel() / img.devicePixelRatioF();
  QgsRectangle rect( topLeft.x(), topLeft.y(), topLeft.x() + img.width()*res, topLeft.y() - img.height()*res );
  return rect;
}

bool QgsMapCanvas::previewJobsEnabled() const
{
  return mUsePreviewJobs;
}

void QgsMapCanvas::setPreviewJobsEnabled( bool enabled )
{
  mUsePreviewJobs = enabled;
}

void QgsMapCanvas::setCustomDropHandlers( const QVector<QPointer<QgsCustomDropHandler> > &handlers )
{
  mDropHandlers = handlers;
}

void QgsMapCanvas::clearTemporalCache()
{
  if ( mCache )
  {
    bool invalidateLabels = false;
    const QList<QgsMapLayer *> layerList = mapSettings().layers();
    for ( QgsMapLayer *layer : layerList )
    {
      if ( layer->temporalProperties() && layer->temporalProperties()->isActive() )
      {
        if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
        {
          if ( vl->labelsEnabled() || vl->diagramsEnabled() )
            invalidateLabels = true;
        }

        if ( layer->temporalProperties()->flags() & QgsTemporalProperty::FlagDontInvalidateCachedRendersWhenRangeChanges )
          continue;

        mCache->invalidateCacheForLayer( layer );
      }
    }

    if ( invalidateLabels )
    {
      mCache->clearCacheImage( QStringLiteral( "_labels_" ) );
      mCache->clearCacheImage( QStringLiteral( "_preview_labels_" ) );
    }
  }
}

void QgsMapCanvas::clearElevationCache()
{
  if ( mCache )
  {
    bool invalidateLabels = false;
    const QList<QgsMapLayer *> layerList = mapSettings().layers();
    for ( QgsMapLayer *layer : layerList )
    {
      if ( layer->elevationProperties() && layer->elevationProperties()->hasElevation() )
      {
        if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
        {
          if ( vl->labelsEnabled() || vl->diagramsEnabled() )
            invalidateLabels = true;
        }

        if ( layer->elevationProperties()->flags() & QgsMapLayerElevationProperties::FlagDontInvalidateCachedRendersWhenRangeChanges )
          continue;

        mCache->invalidateCacheForLayer( layer );
      }
    }

    if ( invalidateLabels )
    {
      mCache->clearCacheImage( QStringLiteral( "_labels_" ) );
      mCache->clearCacheImage( QStringLiteral( "_preview_labels_" ) );
    }
  }
}

void QgsMapCanvas::showContextMenu( QgsMapMouseEvent *event )
{
  const QgsPointXY mapPoint = event->originalMapPoint();

  QMenu menu;

  QMenu *copyCoordinateMenu = new QMenu( tr( "Copy Coordinate" ), &menu );
  copyCoordinateMenu->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );

  auto addCoordinateFormat = [ &, this]( const QString identifier, const QgsCoordinateReferenceSystem & crs )
  {
    const QgsCoordinateTransform ct( mSettings.destinationCrs(), crs, mSettings.transformContext() );
    try
    {
      const QgsPointXY transformedPoint = ct.transform( mapPoint );

      // calculate precision based on visible map extent -- if user is zoomed in, we get better precision!
      int displayPrecision = 0;
      try
      {
        QgsCoordinateTransform extentTransform = ct;
        extentTransform.setBallparkTransformsAreAppropriate( true );
        QgsRectangle extentReproj = extentTransform.transformBoundingBox( extent() );
        const double mapUnitsPerPixel = ( extentReproj.width() / width() + extentReproj.height() / height() ) * 0.5;
        if ( mapUnitsPerPixel > 10 )
          displayPrecision = 0;
        else if ( mapUnitsPerPixel > 1 )
          displayPrecision = 1;
        else if ( mapUnitsPerPixel > 0.1 )
          displayPrecision = 2;
        else if ( mapUnitsPerPixel > 0.01 )
          displayPrecision = 3;
        else if ( mapUnitsPerPixel > 0.001 )
          displayPrecision = 4;
        else if ( mapUnitsPerPixel > 0.0001 )
          displayPrecision = 5;
        else if ( mapUnitsPerPixel > 0.00001 )
          displayPrecision = 6;
        else if ( mapUnitsPerPixel > 0.000001 )
          displayPrecision = 7;
        else if ( mapUnitsPerPixel > 0.0000001 )
          displayPrecision = 8;
        else
          displayPrecision = 9;
      }
      catch ( QgsCsException & )
      {
        displayPrecision = crs.mapUnits() == QgsUnitTypes::DistanceDegrees ? 5 : 3;
      }

      QAction *copyCoordinateAction = new QAction( QStringLiteral( "%3 (%1, %2)" ).arg(
            QString::number( transformedPoint.x(), 'f', displayPrecision ),
            QString::number( transformedPoint.y(), 'f', displayPrecision ),
            identifier ), &menu );

      connect( copyCoordinateAction, &QAction::triggered, this, [displayPrecision, transformedPoint]
      {
        QClipboard *clipboard = QApplication::clipboard();

        const QString coordinates = QString::number( transformedPoint.x(), 'f', displayPrecision ) + ',' + QString::number( transformedPoint.y(), 'f', displayPrecision );

        //if we are on x11 system put text into selection ready for middle button pasting
        if ( clipboard->supportsSelection() )
        {
          clipboard->setText( coordinates, QClipboard::Selection );
        }
        clipboard->setText( coordinates, QClipboard::Clipboard );

      } );
      copyCoordinateMenu->addAction( copyCoordinateAction );
    }
    catch ( QgsCsException & )
    {

    }
  };

  addCoordinateFormat( tr( "Map CRS — %1" ).arg( mSettings.destinationCrs().userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) ), mSettings.destinationCrs() );
  if ( mSettings.destinationCrs() != QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) )
    addCoordinateFormat( tr( "WGS84" ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsSettings settings;
  const QString customCrsString = settings.value( QStringLiteral( "qgis/custom_coordinate_crs" ) ).toString();
  if ( !customCrsString.isEmpty() )
  {
    QgsCoordinateReferenceSystem customCrs( customCrsString );
    if ( customCrs != mSettings.destinationCrs() && customCrs != QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) )
    {
      addCoordinateFormat( customCrs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ), customCrs );
    }
  }
  copyCoordinateMenu->addSeparator();
  QAction *setCustomCrsAction = new QAction( tr( "Set Custom CRS…" ), &menu );
  connect( setCustomCrsAction, &QAction::triggered, this, [ = ]
  {
    QgsProjectionSelectionDialog selector( this );
    selector.setCrs( QgsCoordinateReferenceSystem( customCrsString ) );
    if ( selector.exec() )
    {
      QgsSettings().setValue( QStringLiteral( "qgis/custom_coordinate_crs" ), selector.crs().authid().isEmpty() ? selector.crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) : selector.crs().authid() );
    }
  } );
  copyCoordinateMenu->addAction( setCustomCrsAction );

  menu.addMenu( copyCoordinateMenu );

  if ( mMapTool )
    if ( !mapTool()->populateContextMenuWithEvent( &menu, event ) )
      mMapTool->populateContextMenu( &menu );

  emit contextMenuAboutToShow( &menu, event );

  menu.exec( event->globalPos() );
}

void QgsMapCanvas::notifyRendererErrors( const QgsMapRendererJob::Errors &errors )
{
  const QDateTime currentTime = QDateTime::currentDateTime();

  // remove errors too old
  for ( const QgsMapRendererJob::Error &error : errors )
  {
    const QString errorKey = error.layerID + ':' + error.message;
    if ( mRendererErrors.contains( errorKey ) )
    {
      const QDateTime sameErrorTime = mRendererErrors.value( errorKey );

      if ( sameErrorTime.secsTo( currentTime ) < 60 )
        continue;
    }

    mRendererErrors[errorKey] = currentTime;

    if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( error.layerID ) )
      emit renderErrorOccurred( error.message, layer );
  }
}

void QgsMapCanvas::updateDevicePixelFromScreen()
{
  mSettings.setDevicePixelRatio( devicePixelRatio() );
  // TODO: QGIS 4 -> always respect screen dpi
  if ( QgsSettingsRegistryGui::settingsRespectScreenDPI.value() )
  {
    if ( window()->windowHandle() )
    {
      mSettings.setOutputDpi( window()->windowHandle()->screen()->physicalDotsPerInch() );
      mSettings.setDpiTarget( window()->windowHandle()->screen()->physicalDotsPerInch() );
    }
  }
  else
  {
    // Fallback: compatibility with QGIS <= 3.20; always assume low dpi screens
    mSettings.setOutputDpi( window()->windowHandle()->screen()->logicalDotsPerInch() );
    mSettings.setDpiTarget( window()->windowHandle()->screen()->logicalDotsPerInch() );
  }
}

void QgsMapCanvas::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( temporalRange() == dateTimeRange )
    return;

  mSettings.setTemporalRange( dateTimeRange );
  mSettings.setIsTemporal( dateTimeRange.begin().isValid() || dateTimeRange.end().isValid() );

  emit temporalRangeChanged();

  // we need to discard any previously cached images which have temporal properties enabled, so that these will be updated when
  // the canvas is redrawn
  if ( !mJob )
    clearTemporalCache();

  autoRefreshTriggered();
}

const QgsDateTimeRange &QgsMapCanvas::temporalRange() const
{
  return mSettings.temporalRange();
}

void QgsMapCanvas::installInteractionBlocker( QgsMapCanvasInteractionBlocker *blocker )
{
  mInteractionBlockers.append( blocker );
}

void QgsMapCanvas::removeInteractionBlocker( QgsMapCanvasInteractionBlocker *blocker )
{
  mInteractionBlockers.removeAll( blocker );
}

bool QgsMapCanvas::allowInteraction( QgsMapCanvasInteractionBlocker::Interaction interaction ) const
{
  for ( const QgsMapCanvasInteractionBlocker *block : mInteractionBlockers )
  {
    if ( block->blockCanvasInteraction( interaction ) )
      return false;
  }
  return true;
}

void QgsMapCanvas::mapUpdateTimeout()
{
  if ( mJob )
  {
    const QImage &img = mJob->renderedImage();
    mMap->setContent( img, imageRect( img, mSettings ) );
  }
}

void QgsMapCanvas::stopRendering()
{
  if ( mJob )
  {
    QgsDebugMsgLevel( QStringLiteral( "CANVAS stop rendering!" ), 2 );
    mJobCanceled = true;
    disconnect( mJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::rendererJobFinished );
    connect( mJob, &QgsMapRendererQImageJob::finished, mJob, &QgsMapRendererQImageJob::deleteLater );
    mJob->cancelWithoutBlocking();
    mJob = nullptr;
    emit mapRefreshCanceled();
  }
  stopPreviewJobs();
}

//the format defaults to "PNG" if not specified
void QgsMapCanvas::saveAsImage( const QString &fileName, QPixmap *theQPixmap, const QString &format )
{
  QPainter painter;
  QImage image;

  //
  //check if the optional QPaintDevice was supplied
  //
  if ( theQPixmap )
  {
    image = theQPixmap->toImage();
    painter.begin( &image );

    // render
    QgsMapRendererCustomPainterJob job( mSettings, &painter );
    job.start();
    job.waitForFinished();
    emit renderComplete( &painter );
  }
  else //use the map view
  {
    image = mMap->contentImage().copy();
    painter.begin( &image );
  }

  // draw annotations
  QStyleOptionGraphicsItem option;
  option.initFrom( this );
  QGraphicsItem *item = nullptr;
  QListIterator<QGraphicsItem *> i( items() );
  i.toBack();
  while ( i.hasPrevious() )
  {
    item = i.previous();

    if ( !( item && dynamic_cast< QgsMapCanvasAnnotationItem * >( item ) ) )
    {
      continue;
    }

    QgsScopedQPainterState painterState( &painter );

    QPointF itemScenePos = item->scenePos();
    painter.translate( itemScenePos.x(), itemScenePos.y() );

    item->paint( &painter, &option );
  }

  painter.end();
  image.save( fileName, format.toLocal8Bit().data() );

  QFileInfo myInfo  = QFileInfo( fileName );

  // build the world file name
  QString outputSuffix = myInfo.suffix();
  QString myWorldFileName = myInfo.absolutePath() + '/' + myInfo.completeBaseName() + '.'
                            + outputSuffix.at( 0 ) + outputSuffix.at( myInfo.suffix().size() - 1 ) + 'w';
  QFile myWorldFile( myWorldFileName );
  if ( !myWorldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) //don't use QIODevice::Text
  {
    return;
  }
  QTextStream myStream( &myWorldFile );
  myStream << QgsMapSettingsUtils::worldFileContent( mapSettings() );
}

QgsRectangle QgsMapCanvas::extent() const
{
  return mapSettings().visibleExtent();
}

QgsRectangle QgsMapCanvas::fullExtent() const
{
  return QgsMapLayerUtils::combinedExtent( mSettings.layers(), mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
}

QgsRectangle QgsMapCanvas::projectExtent() const
{
  const QgsReferencedRectangle extent = mProject ? mProject->viewSettings()->fullExtent() : QgsProject::instance()->viewSettings()->fullExtent();
  QgsCoordinateTransform ct( extent.crs(), mapSettings().destinationCrs(), mProject ? mProject->transformContext() : QgsProject::instance()->transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  QgsRectangle rect;
  try
  {
    rect = ct.transformBoundingBox( extent );
  }
  catch ( QgsCsException & )
  {
    rect = mapSettings().fullExtent();
  }

  return rect;
}

void QgsMapCanvas::setExtent( const QgsRectangle &r, bool magnified )
{
  QgsRectangle current = extent();

  if ( ( r == current ) && magnified )
    return;

  if ( r.isEmpty() )
  {
    if ( !mSettings.hasValidSettings() )
    {
      // we can't even just move the map center
      QgsDebugMsgLevel( QStringLiteral( "Empty extent - ignoring" ), 2 );
      return;
    }

    // ### QGIS 3: do not allow empty extent - require users to call setCenter() explicitly
    QgsDebugMsgLevel( QStringLiteral( "Empty extent - keeping old scale with new center!" ), 2 );
    setCenter( r.center() );
  }
  else
  {
    // If scale is locked we need to maintain the current scale, so we
    // - magnify and recenter the map
    // - restore locked scale
    if ( mScaleLocked && magnified )
    {
      ScaleRestorer restorer( this );
      const double ratio { extent().width() / extent().height() };
      const double factor { r.width() / r.height() > ratio ? extent().width() / r.width() :  extent().height() / r.height() };
      const double scaleFactor { std::clamp( mSettings.magnificationFactor() * factor, QgsGuiUtils::CANVAS_MAGNIFICATION_MIN, QgsGuiUtils::CANVAS_MAGNIFICATION_MAX ) };
      const QgsPointXY newCenter { r.center() };
      mSettings.setMagnificationFactor( scaleFactor, &newCenter );
      emit magnificationChanged( scaleFactor );
    }
    else
    {
      mSettings.setExtent( r, magnified );
    }
  }
  emit extentsChanged();
  updateScale();

  //clear all extent items after current index
  for ( int i = mLastExtent.size() - 1; i > mLastExtentIndex; i-- )
  {
    mLastExtent.removeAt( i );
  }

  if ( !mLastExtent.isEmpty() && mLastExtent.last() != extent() )
  {
    mLastExtent.append( extent() );
  }

  // adjust history to no more than 100
  if ( mLastExtent.size() > 100 )
  {
    mLastExtent.removeAt( 0 );
  }

  // the last item is the current extent
  mLastExtentIndex = mLastExtent.size() - 1;

  // update controls' enabled state
  emit zoomLastStatusChanged( mLastExtentIndex > 0 );
  emit zoomNextStatusChanged( mLastExtentIndex < mLastExtent.size() - 1 );
}

bool QgsMapCanvas::setReferencedExtent( const QgsReferencedRectangle &extent )
{
  QgsRectangle canvasExtent = extent;
  if ( extent.crs() != mapSettings().destinationCrs() )
  {
    QgsCoordinateTransform ct( extent.crs(), mapSettings().destinationCrs(), QgsProject::instance() );
    ct.setBallparkTransformsAreAppropriate( true );
    canvasExtent = ct.transformBoundingBox( extent );

    if ( canvasExtent.isEmpty() )
    {
      return false;
    }
  }

  setExtent( canvasExtent, true );
  return true;
}

void QgsMapCanvas::setCenter( const QgsPointXY &center )
{
  const QgsRectangle r = mapSettings().extent();
  const double xMin = center.x() - r.width() / 2.0;
  const double yMin = center.y() - r.height() / 2.0;
  const QgsRectangle rect(
    xMin, yMin,
    xMin + r.width(), yMin + r.height()
  );
  if ( ! rect.isEmpty() )
  {
    setExtent( rect, true );
  }
} // setCenter

QgsPointXY QgsMapCanvas::center() const
{
  QgsRectangle r = mapSettings().extent();
  return r.center();
}

QgsPointXY QgsMapCanvas::cursorPoint() const
{
  return mCursorPoint;
}

double QgsMapCanvas::rotation() const
{
  return mapSettings().rotation();
} // rotation

void QgsMapCanvas::setRotation( double degrees )
{
  double current = rotation();

  if ( qgsDoubleNear( degrees, current ) )
    return;

  mSettings.setRotation( degrees );
  emit rotationChanged( degrees );
  emit extentsChanged(); // visible extent changes with rotation
} // setRotation


void QgsMapCanvas::updateScale()
{
  emit scaleChanged( mapSettings().scale() );
}

void QgsMapCanvas::zoomToFullExtent()
{
  QgsRectangle extent = fullExtent();
  // If the full extent is an empty set, don't do the zoom
  if ( !extent.isEmpty() )
  {
    // Add a 5% margin around the full extent
    extent.scale( 1.05 );
    setExtent( extent );
  }
  refresh();
}

void QgsMapCanvas::zoomToProjectExtent()
{
  QgsRectangle extent = projectExtent();

  // If the full extent is an empty set, don't do the zoom
  if ( !extent.isEmpty() )
  {
    // Add a 5% margin around the full extent
    extent.scale( 1.05 );
    setExtent( extent );
  }
  refresh();
}

void QgsMapCanvas::zoomToPreviousExtent()
{
  if ( mLastExtentIndex > 0 )
  {
    mLastExtentIndex--;
    mSettings.setExtent( mLastExtent[mLastExtentIndex] );
    emit extentsChanged();
    updateScale();
    refresh();
    // update controls' enabled state
    emit zoomLastStatusChanged( mLastExtentIndex > 0 );
    emit zoomNextStatusChanged( mLastExtentIndex < mLastExtent.size() - 1 );
  }

} // zoomToPreviousExtent

void QgsMapCanvas::zoomToNextExtent()
{
  if ( mLastExtentIndex < mLastExtent.size() - 1 )
  {
    mLastExtentIndex++;
    mSettings.setExtent( mLastExtent[mLastExtentIndex] );
    emit extentsChanged();
    updateScale();
    refresh();
    // update controls' enabled state
    emit zoomLastStatusChanged( mLastExtentIndex > 0 );
    emit zoomNextStatusChanged( mLastExtentIndex < mLastExtent.size() - 1 );
  }
}// zoomToNextExtent

void QgsMapCanvas::clearExtentHistory()
{
  mLastExtent.clear(); // clear the zoom history list
  mLastExtent.append( extent() ) ; // set the current extent in the list
  mLastExtentIndex = mLastExtent.size() - 1;
  // update controls' enabled state
  emit zoomLastStatusChanged( mLastExtentIndex > 0 );
  emit zoomNextStatusChanged( mLastExtentIndex < mLastExtent.size() - 1 );
}// clearExtentHistory

QgsRectangle QgsMapCanvas::optimalExtentForPointLayer( QgsVectorLayer *layer, const QgsPointXY &center, int scaleFactor )
{
  QgsRectangle rect( center, center );

  if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    QgsPointXY centerLayerCoordinates = mSettings.mapToLayerCoordinates( layer, center );
    QgsRectangle extentRect = mSettings.mapToLayerCoordinates( layer, extent() ).scaled( 1.0 / scaleFactor, &centerLayerCoordinates );
    QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( extentRect ).setLimit( 1000 ).setNoAttributes();
    QgsFeatureIterator fit = layer->getFeatures( req );
    QgsFeature f;
    QgsPointXY closestPoint;
    double closestSquaredDistance = pow( extentRect.width(), 2.0 ) + pow( extentRect.height(), 2.0 );
    bool pointFound = false;
    while ( fit.nextFeature( f ) )
    {
      QgsPointXY point = f.geometry().asPoint();
      double sqrDist = point.sqrDist( centerLayerCoordinates );
      if ( sqrDist > closestSquaredDistance || sqrDist < 4 * std::numeric_limits<double>::epsilon() )
        continue;
      pointFound = true;
      closestPoint = point;
      closestSquaredDistance = sqrDist;
    }
    if ( pointFound )
    {
      // combine selected point with closest point and scale this rect
      rect.combineExtentWith( mSettings.layerToMapCoordinates( layer, closestPoint ) );
      rect.scale( scaleFactor, &center );
    }
  }
  return rect;
}

void QgsMapCanvas::zoomToSelected( QgsVectorLayer *layer )
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  if ( !layer )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( !layer || !layer->isSpatial() || layer->selectedFeatureCount() == 0 )
    return;

  QgsRectangle rect = layer->boundingBoxOfSelected();
  if ( rect.isNull() )
  {
    cursorOverride.release();
    emit messageEmitted( tr( "Cannot zoom to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::MessageLevel::Warning );
    return;
  }

  rect = mapSettings().layerExtentToOutputExtent( layer, rect );

  // zoom in if point cannot be distinguished from others
  // also check that rect is empty, as it might not in case of multi points
  if ( layer->geometryType() == QgsWkbTypes::PointGeometry && rect.isEmpty() )
  {
    rect = optimalExtentForPointLayer( layer, rect.center() );
  }
  zoomToFeatureExtent( rect );
}

void QgsMapCanvas::zoomToSelected( const QList<QgsMapLayer *> &layers )
{
  QgsRectangle rect;
  rect.setMinimal();
  QgsRectangle selectionExtent;
  selectionExtent.setMinimal();

  for ( QgsMapLayer *mapLayer : layers )
  {
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mapLayer );

    if ( !layer || !layer->isSpatial() || layer->selectedFeatureCount() == 0 )
      continue;

    rect = layer->boundingBoxOfSelected();

    if ( rect.isNull() )
      continue;

    rect = mapSettings().layerExtentToOutputExtent( layer, rect );

    if ( layer->geometryType() == QgsWkbTypes::PointGeometry && rect.isEmpty() )
      rect = optimalExtentForPointLayer( layer, rect.center() );

    selectionExtent.combineExtentWith( rect );
  }

  if ( selectionExtent.isNull() )
  {
    emit messageEmitted( tr( "Cannot zoom to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::MessageLevel::Warning );
    return;
  }

  zoomToFeatureExtent( selectionExtent );
}

QgsDoubleRange QgsMapCanvas::zRange() const
{
  return mSettings.zRange();
}

void QgsMapCanvas::setZRange( const QgsDoubleRange &range )
{
  if ( zRange() == range )
    return;

  mSettings.setZRange( range );

  emit zRangeChanged();

  // we need to discard any previously cached images which are elevation aware, so that these will be updated when
  // the canvas is redrawn
  if ( !mJob )
    clearElevationCache();

  autoRefreshTriggered();
}

void QgsMapCanvas::zoomToFeatureExtent( QgsRectangle &rect )
{
  // no selected features, only one selected point feature
  //or two point features with the same x- or y-coordinates
  if ( rect.isEmpty() )
  {
    // zoom in
    QgsPointXY c = rect.center();
    rect = extent();
    rect.scale( 1.0, &c );
  }
  //zoom to an area
  else
  {
    // Expand rect to give a bit of space around the selected
    // objects so as to keep them clear of the map boundaries
    // The same 5% should apply to all margins.
    rect.scale( 1.05 );
  }

  setExtent( rect );
  refresh();
}

void QgsMapCanvas::zoomToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids )
{
  if ( !layer )
  {
    return;
  }

  QgsRectangle bbox;
  QString errorMsg;
  if ( boundingBoxOfFeatureIds( ids, layer, bbox, errorMsg ) )
  {
    if ( bbox.isEmpty() )
    {
      bbox = optimalExtentForPointLayer( layer, bbox.center() );
    }
    zoomToFeatureExtent( bbox );
  }
  else
  {
    emit messageEmitted( tr( "Zoom to feature id failed" ), errorMsg, Qgis::MessageLevel::Warning );
  }

}

void QgsMapCanvas::panToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids, bool alwaysRecenter )
{
  if ( !layer )
  {
    return;
  }

  QgsRectangle bbox;
  QString errorMsg;
  if ( boundingBoxOfFeatureIds( ids, layer, bbox, errorMsg ) )
  {
    if ( alwaysRecenter || !mapSettings().extent().contains( bbox ) )
      setCenter( bbox.center() );
    refresh();
  }
  else
  {
    emit messageEmitted( tr( "Pan to feature id failed" ), errorMsg, Qgis::MessageLevel::Warning );
  }
}

bool QgsMapCanvas::boundingBoxOfFeatureIds( const QgsFeatureIds &ids, QgsVectorLayer *layer, QgsRectangle &bbox, QString &errorMsg ) const
{
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest().setFilterFids( ids ).setNoAttributes() );
  bbox.setMinimal();
  QgsFeature fet;
  int featureCount = 0;
  errorMsg.clear();

  while ( it.nextFeature( fet ) )
  {
    QgsGeometry geom = fet.geometry();
    if ( geom.isNull() )
    {
      errorMsg = tr( "Feature does not have a geometry" );
    }
    else if ( geom.constGet()->isEmpty() )
    {
      errorMsg = tr( "Feature geometry is empty" );
    }
    if ( !errorMsg.isEmpty() )
    {
      return false;
    }
    QgsRectangle r = mapSettings().layerExtentToOutputExtent( layer, geom.boundingBox() );
    bbox.combineExtentWith( r );
    featureCount++;
  }

  if ( featureCount != ids.count() )
  {
    errorMsg = tr( "Feature not found" );
    return false;
  }

  return true;
}

void QgsMapCanvas::panToSelected( QgsVectorLayer *layer )
{
  if ( !layer )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( !layer || !layer->isSpatial() || layer->selectedFeatureCount() == 0 )
    return;

  QgsRectangle rect = layer->boundingBoxOfSelected();
  if ( rect.isNull() )
  {
    emit messageEmitted( tr( "Cannot pan to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::MessageLevel::Warning );
    return;
  }

  rect = mapSettings().layerExtentToOutputExtent( layer, rect );
  setCenter( rect.center() );
  refresh();
}

void QgsMapCanvas::panToSelected( const QList<QgsMapLayer *> &layers )
{
  QgsRectangle rect;
  rect.setMinimal();
  QgsRectangle selectionExtent;
  selectionExtent.setMinimal();

  for ( QgsMapLayer *mapLayer : layers )
  {
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mapLayer );

    if ( !layer || !layer->isSpatial() || layer->selectedFeatureCount() == 0 )
      continue;

    rect = layer->boundingBoxOfSelected();

    if ( rect.isNull() )
      continue;

    rect = mapSettings().layerExtentToOutputExtent( layer, rect );

    if ( layer->geometryType() == QgsWkbTypes::PointGeometry && rect.isEmpty() )
      rect = optimalExtentForPointLayer( layer, rect.center() );

    selectionExtent.combineExtentWith( rect );
  }

  if ( selectionExtent.isNull() )
  {
    emit messageEmitted( tr( "Cannot pan to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::MessageLevel::Warning );
    return;
  }

  setCenter( selectionExtent.center() );
  refresh();
}

void QgsMapCanvas::flashFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids,
                                    const QColor &color1, const QColor &color2,
                                    int flashes, int duration )
{
  if ( !layer )
  {
    return;
  }

  QList< QgsGeometry > geoms;

  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest().setFilterFids( ids ).setNoAttributes() );
  QgsFeature fet;
  while ( it.nextFeature( fet ) )
  {
    if ( !fet.hasGeometry() )
      continue;
    geoms << fet.geometry();
  }

  flashGeometries( geoms, layer->crs(), color1, color2, flashes, duration );
}

void QgsMapCanvas::flashGeometries( const QList<QgsGeometry> &geometries, const QgsCoordinateReferenceSystem &crs, const QColor &color1, const QColor &color2, int flashes, int duration )
{
  if ( geometries.isEmpty() )
    return;

  QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( geometries.at( 0 ).wkbType() );
  QgsRubberBand *rb = new QgsRubberBand( this, geomType );
  for ( const QgsGeometry &geom : geometries )
    rb->addGeometry( geom, crs, false );
  rb->updatePosition();
  rb->update();

  if ( geomType == QgsWkbTypes::LineGeometry || geomType == QgsWkbTypes::PointGeometry )
  {
    rb->setWidth( 2 );
    rb->setSecondaryStrokeColor( QColor( 255, 255, 255 ) );
  }
  if ( geomType == QgsWkbTypes::PointGeometry )
    rb->setIcon( QgsRubberBand::ICON_CIRCLE );

  QColor startColor = color1;
  if ( !startColor.isValid() )
  {
    if ( geomType == QgsWkbTypes::PolygonGeometry )
    {
      startColor = rb->fillColor();
    }
    else
    {
      startColor = rb->strokeColor();
    }
    startColor.setAlpha( 255 );
  }
  QColor endColor = color2;
  if ( !endColor.isValid() )
  {
    endColor = startColor;
    endColor.setAlpha( 0 );
  }


  QVariantAnimation *animation = new QVariantAnimation( this );
  connect( animation, &QVariantAnimation::finished, this, [animation, rb]
  {
    animation->deleteLater();
    delete rb;
  } );
  connect( animation, &QPropertyAnimation::valueChanged, this, [rb, geomType]( const QVariant & value )
  {
    QColor c = value.value<QColor>();
    if ( geomType == QgsWkbTypes::PolygonGeometry )
    {
      rb->setFillColor( c );
    }
    else
    {
      rb->setStrokeColor( c );
      QColor c = rb->secondaryStrokeColor();
      c.setAlpha( c.alpha() );
      rb->setSecondaryStrokeColor( c );
    }
    rb->update();
  } );

  animation->setDuration( duration * flashes );
  animation->setStartValue( endColor );
  double midStep = 0.2 / flashes;
  for ( int i = 0; i < flashes; ++i )
  {
    double start = static_cast< double >( i ) / flashes;
    animation->setKeyValueAt( start + midStep, startColor );
    double end = static_cast< double >( i + 1 ) / flashes;
    if ( !qgsDoubleNear( end, 1.0 ) )
      animation->setKeyValueAt( end, endColor );
  }
  animation->setEndValue( endColor );
  animation->start();
}

void QgsMapCanvas::keyPressEvent( QKeyEvent *e )
{
  if ( mCanvasProperties->mouseButtonDown || mCanvasProperties->panSelectorDown )
  {
    emit keyPressed( e );
    return;
  }

  // Don't want to interfer with mouse events
  if ( ! mCanvasProperties->mouseButtonDown )
  {
    // this is backwards, but we can't change now without breaking api because
    // forever QgsMapTools have had to explicitly mark events as ignored in order to
    // indicate that they've consumed the event and that the default behavior should not
    // be applied..!
    e->accept();
    if ( mMapTool )
    {
      mMapTool->keyPressEvent( e );
      if ( !e->isAccepted() ) // map tool consumed event
        return;
    }

    QgsRectangle currentExtent = mapSettings().visibleExtent();
    double dx = std::fabs( currentExtent.width() / 4 );
    double dy = std::fabs( currentExtent.height() / 4 );

    switch ( e->key() )
    {
      case Qt::Key_Left:
        QgsDebugMsgLevel( QStringLiteral( "Pan left" ), 2 );
        setCenter( center() - QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Right:
        QgsDebugMsgLevel( QStringLiteral( "Pan right" ), 2 );
        setCenter( center() + QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Up:
        QgsDebugMsgLevel( QStringLiteral( "Pan up" ), 2 );
        setCenter( center() + QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Down:
        QgsDebugMsgLevel( QStringLiteral( "Pan down" ), 2 );
        setCenter( center() - QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Space:
        QgsDebugMsgLevel( QStringLiteral( "Pressing pan selector" ), 2 );

        //mCanvasProperties->dragging = true;
        if ( ! e->isAutoRepeat() )
        {
          mTemporaryCursorOverride.reset( new QgsTemporaryCursorOverride( Qt::ClosedHandCursor ) );
          mCanvasProperties->panSelectorDown = true;
          panActionStart( mCanvasProperties->mouseLastXY );
        }
        break;

      case Qt::Key_PageUp:
        QgsDebugMsgLevel( QStringLiteral( "Zoom in" ), 2 );
        zoomIn();
        break;

      case Qt::Key_PageDown:
        QgsDebugMsgLevel( QStringLiteral( "Zoom out" ), 2 );
        zoomOut();
        break;

#if 0
      case Qt::Key_P:
        mUseParallelRendering = !mUseParallelRendering;
        refresh();
        break;

      case Qt::Key_S:
        mDrawRenderingStats = !mDrawRenderingStats;
        refresh();
        break;
#endif

      default:
        // Pass it on
        if ( !mMapTool )
        {
          e->ignore();
          QgsDebugMsgLevel( "Ignoring key: " + QString::number( e->key() ), 2 );
        }
    }
  }

  emit keyPressed( e );
}

void QgsMapCanvas::keyReleaseEvent( QKeyEvent *e )
{
  QgsDebugMsgLevel( QStringLiteral( "keyRelease event" ), 2 );

  switch ( e->key() )
  {
    case Qt::Key_Space:
      if ( !e->isAutoRepeat() && mCanvasProperties->panSelectorDown )
      {
        QgsDebugMsgLevel( QStringLiteral( "Releasing pan selector" ), 2 );
        mTemporaryCursorOverride.reset();
        mCanvasProperties->panSelectorDown = false;
        panActionEnd( mCanvasProperties->mouseLastXY );
      }
      break;

    default:
      // Pass it on
      if ( mMapTool )
      {
        mMapTool->keyReleaseEvent( e );
      }
      else e->ignore();

      QgsDebugMsgLevel( "Ignoring key release: " + QString::number( e->key() ), 2 );
  }

  emit keyReleased( e );

} //keyReleaseEvent()


void QgsMapCanvas::mouseDoubleClickEvent( QMouseEvent *e )
{
  // call handler of current map tool
  if ( mMapTool )
  {
    std::unique_ptr<QgsMapMouseEvent> me( new QgsMapMouseEvent( this, e ) );
    mMapTool->canvasDoubleClickEvent( me.get() );
  }
}// mouseDoubleClickEvent


void QgsMapCanvas::beginZoomRect( QPoint pos )
{
  mZoomRect.setRect( 0, 0, 0, 0 );
  mTemporaryCursorOverride.reset( new QgsTemporaryCursorOverride( mZoomCursor ) );
  mZoomDragging = true;
  mZoomRubberBand.reset( new QgsRubberBand( this, QgsWkbTypes::PolygonGeometry ) );
  QColor color( Qt::blue );
  color.setAlpha( 63 );
  mZoomRubberBand->setColor( color );
  mZoomRect.setTopLeft( pos );
}

void QgsMapCanvas::endZoomRect( QPoint pos )
{
  mZoomDragging = false;
  mZoomRubberBand.reset( nullptr );
  mTemporaryCursorOverride.reset();

  // store the rectangle
  mZoomRect.setRight( pos.x() );
  mZoomRect.setBottom( pos.y() );

  //account for bottom right -> top left dragging
  mZoomRect = mZoomRect.normalized();

  if ( mZoomRect.width() < 5 && mZoomRect.height() < 5 )
  {
    //probably a mistake - would result in huge zoom!
    return;
  }

  // set center and zoom
  const QSize &zoomRectSize = mZoomRect.size();
  const QSize &canvasSize = mSettings.outputSize();
  double sfx = static_cast< double >( zoomRectSize.width() ) / canvasSize.width();
  double sfy = static_cast< double >( zoomRectSize.height() ) / canvasSize.height();
  double sf = std::max( sfx, sfy );

  QgsPointXY c = mSettings.mapToPixel().toMapCoordinates( mZoomRect.center() );

  zoomByFactor( sf, &c );
  refresh();
}

void QgsMapCanvas::mousePressEvent( QMouseEvent *e )
{
  // use shift+middle mouse button for zooming, map tools won't receive any events in that case
  if ( e->button() == Qt::MiddleButton &&
       e->modifiers() & Qt::ShiftModifier )
  {
    beginZoomRect( e->pos() );
    return;
  }
  //use middle mouse button for panning, map tools won't receive any events in that case
  else if ( e->button() == Qt::MiddleButton )
  {
    if ( !mCanvasProperties->panSelectorDown )
    {
      mCanvasProperties->panSelectorDown = true;
      mTemporaryCursorOverride.reset( new QgsTemporaryCursorOverride( Qt::ClosedHandCursor ) );
      panActionStart( mCanvasProperties->mouseLastXY );
    }
  }
  else
  {
    // call handler of current map tool
    if ( mMapTool )
    {
      if ( mMapTool->flags() & QgsMapTool::AllowZoomRect && e->button() == Qt::LeftButton
           && e->modifiers() & Qt::ShiftModifier )
      {
        beginZoomRect( e->pos() );
        return;
      }
      else if ( mMapTool->flags() & QgsMapTool::ShowContextMenu && e->button() == Qt::RightButton )
      {
        std::unique_ptr<QgsMapMouseEvent> me( new QgsMapMouseEvent( this, e ) );
        showContextMenu( me.get() );
        return;
      }
      else
      {
        std::unique_ptr<QgsMapMouseEvent> me( new QgsMapMouseEvent( this, e ) );
        mMapTool->canvasPressEvent( me.get() );
      }
    }
  }

  if ( mCanvasProperties->panSelectorDown )
  {
    return;
  }

  mCanvasProperties->mouseButtonDown = true;
  mCanvasProperties->rubberStartPoint = e->pos();
}

void QgsMapCanvas::mouseReleaseEvent( QMouseEvent *e )
{
  // if using shift+middle mouse button for zooming, end zooming and return
  if ( mZoomDragging &&
       e->button() == Qt::MiddleButton )
  {
    endZoomRect( e->pos() );
    return;
  }
  //use middle mouse button for panning, map tools won't receive any events in that case
  else if ( e->button() == Qt::MiddleButton )
  {
    if ( mCanvasProperties->panSelectorDown )
    {
      mCanvasProperties->panSelectorDown = false;
      mTemporaryCursorOverride.reset();
      panActionEnd( mCanvasProperties->mouseLastXY );
    }
  }
  else if ( e->button() == Qt::BackButton )
  {
    zoomToPreviousExtent();
    return;
  }
  else if ( e->button() == Qt::ForwardButton )
  {
    zoomToNextExtent();
    return;
  }
  else
  {
    if ( mZoomDragging && e->button() == Qt::LeftButton )
    {
      endZoomRect( e->pos() );
      return;
    }

    // call handler of current map tool
    if ( mMapTool )
    {
      // right button was pressed in zoom tool? return to previous non zoom tool
      if ( e->button() == Qt::RightButton && mMapTool->flags() & QgsMapTool::Transient )
      {
        QgsDebugMsgLevel( QStringLiteral( "Right click in map tool zoom or pan, last tool is %1." ).arg(
                            mLastNonZoomMapTool ? QStringLiteral( "not null" ) : QStringLiteral( "null" ) ), 2 );

        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );

        // change to older non-zoom tool
        if ( mLastNonZoomMapTool
             && ( !( mLastNonZoomMapTool->flags() & QgsMapTool::EditTool )
                  || ( vlayer && vlayer->isEditable() ) ) )
        {
          QgsMapTool *t = mLastNonZoomMapTool;
          mLastNonZoomMapTool = nullptr;
          setMapTool( t );
        }
        return;
      }
      std::unique_ptr<QgsMapMouseEvent> me( new QgsMapMouseEvent( this, e ) );
      mMapTool->canvasReleaseEvent( me.get() );
    }
  }


  mCanvasProperties->mouseButtonDown = false;

  if ( mCanvasProperties->panSelectorDown )
    return;

}

void QgsMapCanvas::resizeEvent( QResizeEvent *e )
{
  QGraphicsView::resizeEvent( e );
  mResizeTimer->start( 500 ); // in charge of refreshing canvas

  double oldScale = mSettings.scale();
  QSize lastSize = viewport()->size();
  mSettings.setOutputSize( lastSize );

  mScene->setSceneRect( QRectF( 0, 0, lastSize.width(), lastSize.height() ) );

  moveCanvasContents( true );

  if ( mScaleLocked )
  {
    double scaleFactor = oldScale / mSettings.scale();
    QgsRectangle r = mSettings.extent();
    QgsPointXY center = r.center();
    r.scale( scaleFactor, &center );
    mSettings.setExtent( r );
  }
  else
  {
    updateScale();
  }

  emit extentsChanged();
}

void QgsMapCanvas::paintEvent( QPaintEvent *e )
{
  // no custom event handling anymore

  QGraphicsView::paintEvent( e );
} // paintEvent

void QgsMapCanvas::updateCanvasItemPositions()
{
  if ( mBlockItemPositionUpdates )
    return;

  const QList<QGraphicsItem *> items = mScene->items();
  for ( QGraphicsItem *gi : items )
  {
    QgsMapCanvasItem *item = dynamic_cast<QgsMapCanvasItem *>( gi );

    if ( item )
    {
      item->updatePosition();
    }
  }
}


void QgsMapCanvas::wheelEvent( QWheelEvent *e )
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in

  QgsDebugMsgLevel( "Wheel event delta " + QString::number( e->angleDelta().y() ), 2 );

  if ( mMapTool )
  {
    mMapTool->wheelEvent( e );
    if ( e->isAccepted() )
      return;
  }

  if ( e->angleDelta().y() == 0 )
  {
    e->accept();
    return;
  }

  double zoomFactor = e->angleDelta().y() > 0 ? 1. / zoomInFactor() : zoomOutFactor();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( e->angleDelta().y() );

  if ( e->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  double signedWheelFactor = e->angleDelta().y() > 0 ? 1 / zoomFactor : zoomFactor;

  // zoom map to mouse cursor by scaling
  QgsPointXY oldCenter = center();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QgsPointXY mousePos( getCoordinateTransform()->toMapCoordinates( e->pos().x(), e->pos().y() ) );
#else
  QgsPointXY mousePos( getCoordinateTransform()->toMapCoordinates( e->position().x(), e->position().y() ) );
#endif
  QgsPointXY newCenter( mousePos.x() + ( ( oldCenter.x() - mousePos.x() ) * signedWheelFactor ),
                        mousePos.y() + ( ( oldCenter.y() - mousePos.y() ) * signedWheelFactor ) );

  zoomByFactor( signedWheelFactor, &newCenter );
  e->accept();
}

void QgsMapCanvas::setWheelFactor( double factor )
{
  mWheelZoomFactor = factor;
}

void QgsMapCanvas::zoomIn()
{
  // magnification is alreday handled in zoomByFactor
  zoomByFactor( zoomInFactor() );
}

void QgsMapCanvas::zoomOut()
{
  // magnification is alreday handled in zoomByFactor
  zoomByFactor( zoomOutFactor() );
}

void QgsMapCanvas::zoomScale( double newScale, bool ignoreScaleLock )
{
  zoomByFactor( newScale / scale(), nullptr, ignoreScaleLock );
}

void QgsMapCanvas::zoomWithCenter( int x, int y, bool zoomIn )
{
  double scaleFactor = ( zoomIn ? zoomInFactor() : zoomOutFactor() );

  // transform the mouse pos to map coordinates
  QgsPointXY center  = getCoordinateTransform()->toMapCoordinates( x, y );

  if ( mScaleLocked )
  {
    ScaleRestorer restorer( this );
    setMagnificationFactor( mapSettings().magnificationFactor() / scaleFactor, &center );
  }
  else
  {
    QgsRectangle r = mapSettings().visibleExtent();
    r.scale( scaleFactor, &center );
    setExtent( r, true );
    refresh();
  }
}

void QgsMapCanvas::setScaleLocked( bool isLocked )
{
  if ( mScaleLocked != isLocked )
  {
    mScaleLocked = isLocked;
    emit scaleLockChanged( mScaleLocked );
  }
}

void QgsMapCanvas::mouseMoveEvent( QMouseEvent *e )
{
  mCanvasProperties->mouseLastXY = e->pos();

  if ( mCanvasProperties->panSelectorDown )
  {
    panAction( e );
  }
  else if ( mZoomDragging )
  {
    mZoomRect.setBottomRight( e->pos() );
    mZoomRubberBand->setToCanvasRectangle( mZoomRect );
    mZoomRubberBand->show();
  }
  else
  {
    // call handler of current map tool
    if ( mMapTool )
    {
      std::unique_ptr<QgsMapMouseEvent> me( new QgsMapMouseEvent( this, e ) );
      mMapTool->canvasMoveEvent( me.get() );
    }
  }

  // show x y on status bar (if we are mid pan operation, then the cursor point hasn't changed!)
  if ( !panOperationInProgress() )
  {
    mCursorPoint = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->mouseLastXY );
    emit xyCoordinates( mCursorPoint );
  }
}

void QgsMapCanvas::setMapTool( QgsMapTool *tool, bool clean )
{
  if ( !tool )
    return;

  if ( mMapTool )
  {
    if ( clean )
      mMapTool->clean();

    disconnect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    mMapTool->deactivate();
  }

  if ( ( tool->flags() & QgsMapTool::Transient )
       && mMapTool && !( mMapTool->flags() & QgsMapTool::Transient ) )
  {
    // if zoom or pan tool will be active, save old tool
    // to bring it back on right click
    // (but only if it wasn't also zoom or pan tool)
    mLastNonZoomMapTool = mMapTool;
  }
  else
  {
    mLastNonZoomMapTool = nullptr;
  }

  QgsMapTool *oldTool = mMapTool;

  // set new map tool and activate it
  mMapTool = tool;
  emit mapToolSet( mMapTool, oldTool );
  if ( mMapTool )
  {
    connect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    mMapTool->activate();
  }

} // setMapTool

void QgsMapCanvas::unsetMapTool( QgsMapTool *tool )
{
  if ( mMapTool && mMapTool == tool )
  {
    disconnect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    QgsMapTool *oldTool = mMapTool;
    mMapTool = nullptr;
    oldTool->deactivate();
    emit mapToolSet( nullptr, oldTool );
    setCursor( Qt::ArrowCursor );
  }

  if ( mLastNonZoomMapTool && mLastNonZoomMapTool == tool )
  {
    mLastNonZoomMapTool = nullptr;
  }
}

void QgsMapCanvas::setProject( QgsProject *project )
{
  mProject = project;
}

void QgsMapCanvas::setCanvasColor( const QColor &color )
{
  if ( canvasColor() == color )
    return;

  // background of map's pixmap
  mSettings.setBackgroundColor( color );

  // background of the QGraphicsView
  QBrush bgBrush( color );
  setBackgroundBrush( bgBrush );
#if 0
  QPalette palette;
  palette.setColor( backgroundRole(), color );
  setPalette( palette );
#endif

  // background of QGraphicsScene
  mScene->setBackgroundBrush( bgBrush );

  refresh();

  emit canvasColorChanged();
}

QColor QgsMapCanvas::canvasColor() const
{
  return mScene->backgroundBrush().color();
}

void QgsMapCanvas::setSelectionColor( const QColor &color )
{
  if ( mSettings.selectionColor() == color )
    return;

  mSettings.setSelectionColor( color );

  if ( mCache )
  {
    bool hasSelectedFeatures = false;
    const auto layers = mSettings.layers();
    for ( QgsMapLayer *layer : layers )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( vlayer && vlayer->selectedFeatureCount() )
      {
        hasSelectedFeatures = true;
        break;
      }
    }

    if ( hasSelectedFeatures )
    {
      mCache->clear();
      refresh();
    }
  }
}

QColor QgsMapCanvas::selectionColor() const
{
  return mSettings.selectionColor();
}

int QgsMapCanvas::layerCount() const
{
  return mapSettings().layers().size();
}

QList<QgsMapLayer *> QgsMapCanvas::layers( bool expandGroupLayers ) const
{
  return mapSettings().layers( expandGroupLayers );
}

void QgsMapCanvas::layerStateChange()
{
  // called when a layer has changed visibility setting
  refresh();
}

void QgsMapCanvas::freeze( bool frozen )
{
  mFrozen = frozen;
}

bool QgsMapCanvas::isFrozen() const
{
  return mFrozen;
}

double QgsMapCanvas::mapUnitsPerPixel() const
{
  return mapSettings().mapUnitsPerPixel();
}

QgsUnitTypes::DistanceUnit QgsMapCanvas::mapUnits() const
{
  return mapSettings().mapUnits();
}

QMap<QString, QString> QgsMapCanvas::layerStyleOverrides() const
{
  return mSettings.layerStyleOverrides();
}

void QgsMapCanvas::setLayerStyleOverrides( const QMap<QString, QString> &overrides )
{
  if ( overrides == mSettings.layerStyleOverrides() )
    return;

  mSettings.setLayerStyleOverrides( overrides );
  clearCache();
  emit layerStyleOverridesChanged();
}

void QgsMapCanvas::setTheme( const QString &theme )
{
  if ( mTheme == theme )
    return;

  clearCache();
  if ( theme.isEmpty() || !QgsProject::instance()->mapThemeCollection()->hasMapTheme( theme ) )
  {
    mTheme.clear();
    mSettings.setLayerStyleOverrides( QMap< QString, QString>() );
    setLayers( QgsProject::instance()->mapThemeCollection()->masterVisibleLayers() );
    emit themeChanged( QString() );
  }
  else
  {
    mTheme = theme;
    setLayersPrivate( QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayers( mTheme ) );
    emit themeChanged( theme );
  }
}

void QgsMapCanvas::setRenderFlag( bool flag )
{
  mRenderFlag = flag;

  if ( mRenderFlag )
  {
    refresh();
  }
  else
    stopRendering();
}

#if 0
void QgsMapCanvas::connectNotify( const char *signal )
{
  Q_UNUSED( signal )
  QgsDebugMsg( "QgsMapCanvas connected to " + QString( signal ) );
} //connectNotify
#endif

void QgsMapCanvas::layerRepaintRequested( bool deferred )
{
  if ( !deferred )
    refresh();
}

void QgsMapCanvas::autoRefreshTriggered()
{
  if ( mJob )
  {
    // canvas is currently being redrawn, so we defer the last requested
    // auto refresh until current rendering job finishes
    mRefreshAfterJob = true;
    return;
  }

  refresh();
}

void QgsMapCanvas::updateAutoRefreshTimer()
{
  // min auto refresh interval stores the smallest interval between layer auto refreshes. We automatically
  // trigger a map refresh on this minimum interval
  int minAutoRefreshInterval = -1;
  const auto layers = mSettings.layers();
  for ( QgsMapLayer *layer : layers )
  {
    if ( layer->hasAutoRefreshEnabled() && layer->autoRefreshInterval() > 0 )
      minAutoRefreshInterval = minAutoRefreshInterval > 0 ? std::min( layer->autoRefreshInterval(), minAutoRefreshInterval ) : layer->autoRefreshInterval();
  }

  if ( minAutoRefreshInterval > 0 )
  {
    mAutoRefreshTimer.setInterval( minAutoRefreshInterval );
    mAutoRefreshTimer.start();
  }
  else
  {
    mAutoRefreshTimer.stop();
  }
}

void QgsMapCanvas::projectThemesChanged()
{
  if ( mTheme.isEmpty() )
    return;

  if ( !QgsProject::instance()->mapThemeCollection()->hasMapTheme( mTheme ) )
  {
    // theme has been removed - stop following
    setTheme( QString() );
  }

}

QgsMapTool *QgsMapCanvas::mapTool()
{
  return mMapTool;
}

QgsProject *QgsMapCanvas::project()
{
  return mProject;
}

void QgsMapCanvas::panActionEnd( QPoint releasePoint )
{
  // move map image and other items to standard position
  moveCanvasContents( true ); // true means reset

  // use start and end box points to calculate the extent
  QgsPointXY start = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->rubberStartPoint );
  QgsPointXY end = getCoordinateTransform()->toMapCoordinates( releasePoint );

  // modify the center
  double dx = end.x() - start.x();
  double dy = end.y() - start.y();
  QgsPointXY c = center();
  c.set( c.x() - dx, c.y() - dy );
  setCenter( c );

  refresh();
}

void QgsMapCanvas::panActionStart( QPoint releasePoint )
{
  mCanvasProperties->rubberStartPoint = releasePoint;

  mDa = QgsDistanceArea();
  mDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mDa.setSourceCrs( mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
}

void QgsMapCanvas::panAction( QMouseEvent *e )
{
  Q_UNUSED( e )

  QgsPointXY currentMapPoint = getCoordinateTransform()->toMapCoordinates( e->pos() );
  QgsPointXY startMapPoint = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->rubberStartPoint );
  try
  {
    emit panDistanceBearingChanged( mDa.measureLine( currentMapPoint, startMapPoint ), mDa.lengthUnits(), mDa.bearing( currentMapPoint, startMapPoint ) * 180 / M_PI );
  }
  catch ( QgsCsException & )
  {}

  // move all map canvas items
  moveCanvasContents();
}

void QgsMapCanvas::moveCanvasContents( bool reset )
{
  QPoint pnt( 0, 0 );
  if ( !reset )
    pnt += mCanvasProperties->mouseLastXY - mCanvasProperties->rubberStartPoint;

  setSceneRect( -pnt.x(), -pnt.y(), viewport()->size().width(), viewport()->size().height() );
}

void QgsMapCanvas::dropEvent( QDropEvent *event )
{
  if ( QgsMimeDataUtils::isUriList( event->mimeData() ) )
  {
    const QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( event->mimeData() );
    bool allHandled = true;
    for ( const QgsMimeDataUtils::Uri &uri : lst )
    {
      bool handled = false;
      for ( QgsCustomDropHandler *handler : std::as_const( mDropHandlers ) )
      {
        if ( handler && handler->customUriProviderKey() == uri.providerKey )
        {
          if ( handler->handleCustomUriCanvasDrop( uri, this ) )
          {
            handled = true;
            break;
          }
        }
      }
      if ( !handled )
        allHandled = false;
    }
    if ( allHandled )
      event->accept();
    else
      event->ignore();
  }
  else
  {
    event->ignore();
  }
}

void QgsMapCanvas::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )
  updateDevicePixelFromScreen();
  // keep device pixel ratio up to date on screen or resolution change
  if ( window()->windowHandle() )
  {
    connect( window()->windowHandle(), &QWindow::screenChanged, this, [ = ]( QScreen * )
    {
      disconnect( mScreenDpiChangedConnection );
      mScreenDpiChangedConnection = connect( window()->windowHandle()->screen(), &QScreen::physicalDotsPerInchChanged, this, &QgsMapCanvas::updateDevicePixelFromScreen );
      updateDevicePixelFromScreen();
    } );

    mScreenDpiChangedConnection = connect( window()->windowHandle()->screen(), &QScreen::physicalDotsPerInchChanged, this, &QgsMapCanvas::updateDevicePixelFromScreen );
  }
}

QPoint QgsMapCanvas::mouseLastXY()
{
  return mCanvasProperties->mouseLastXY;
}

void QgsMapCanvas::setPreviewModeEnabled( bool previewEnabled )
{
  if ( !mPreviewEffect )
  {
    return;
  }

  mPreviewEffect->setEnabled( previewEnabled );
}

bool QgsMapCanvas::previewModeEnabled() const
{
  if ( !mPreviewEffect )
  {
    return false;
  }

  return mPreviewEffect->isEnabled();
}

void QgsMapCanvas::setPreviewMode( QgsPreviewEffect::PreviewMode mode )
{
  if ( !mPreviewEffect )
  {
    return;
  }

  mPreviewEffect->setMode( mode );
}

QgsPreviewEffect::PreviewMode QgsMapCanvas::previewMode() const
{
  if ( !mPreviewEffect )
  {
    return QgsPreviewEffect::PreviewGrayscale;
  }

  return mPreviewEffect->mode();
}

QgsSnappingUtils *QgsMapCanvas::snappingUtils() const
{
  if ( !mSnappingUtils )
  {
    // associate a dummy instance, but better than null pointer
    QgsMapCanvas *c = const_cast<QgsMapCanvas *>( this );
    c->mSnappingUtils = new QgsMapCanvasSnappingUtils( c, c );
  }
  return mSnappingUtils;
}

void QgsMapCanvas::setSnappingUtils( QgsSnappingUtils *utils )
{
  mSnappingUtils = utils;
}

void QgsMapCanvas::readProject( const QDomDocument &doc )
{
  QgsProject *project = qobject_cast< QgsProject * >( sender() );

  QDomNodeList nodes = doc.elementsByTagName( QStringLiteral( "mapcanvas" ) );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );

    // Search the specific MapCanvas node using the name
    if ( nodes.count() > 1 )
    {
      for ( int i = 0; i < nodes.size(); ++i )
      {
        QDomElement elementNode = nodes.at( i ).toElement();

        if ( elementNode.hasAttribute( QStringLiteral( "name" ) ) && elementNode.attribute( QStringLiteral( "name" ) ) == objectName() )
        {
          node = nodes.at( i );
          break;
        }
      }
    }

    QgsMapSettings tmpSettings;
    tmpSettings.readXml( node );
    if ( objectName() != QLatin1String( "theMapCanvas" ) )
    {
      // never manually set the crs for the main canvas - this is instead connected to the project CRS
      setDestinationCrs( tmpSettings.destinationCrs() );
    }
    setExtent( tmpSettings.extent() );
    setRotation( tmpSettings.rotation() );
    enableMapTileRendering( tmpSettings.testFlag( Qgis::MapSettingsFlag::RenderMapTile ) );

    clearExtentHistory(); // clear the extent history on project load

    QDomElement elem = node.toElement();
    if ( elem.hasAttribute( QStringLiteral( "theme" ) ) )
    {
      if ( QgsProject::instance()->mapThemeCollection()->hasMapTheme( elem.attribute( QStringLiteral( "theme" ) ) ) )
      {
        setTheme( elem.attribute( QStringLiteral( "theme" ) ) );
      }
    }
    setAnnotationsVisible( elem.attribute( QStringLiteral( "annotationsVisible" ), QStringLiteral( "1" ) ).toInt() );

    // restore canvas expression context
    const QDomNodeList scopeElements = elem.elementsByTagName( QStringLiteral( "expressionContextScope" ) );
    if ( scopeElements.size() > 0 )
    {
      const QDomElement scopeElement = scopeElements.at( 0 ).toElement();
      mExpressionContextScope.readXml( scopeElement, QgsReadWriteContext() );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Couldn't read mapcanvas information from project" ) );
    if ( !project->viewSettings()->defaultViewExtent().isNull() )
    {
      setReferencedExtent( project->viewSettings()->defaultViewExtent() );
      clearExtentHistory(); // clear the extent history on project load
    }
  }
}

void QgsMapCanvas::writeProject( QDomDocument &doc )
{
  // create node "mapcanvas" and call mMapRenderer->writeXml()

  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );
  if ( !nl.count() )
  {
    QgsDebugMsg( QStringLiteral( "Unable to find qgis element in project file" ) );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element OK

  QDomElement mapcanvasNode = doc.createElement( QStringLiteral( "mapcanvas" ) );
  mapcanvasNode.setAttribute( QStringLiteral( "name" ), objectName() );
  if ( !mTheme.isEmpty() )
    mapcanvasNode.setAttribute( QStringLiteral( "theme" ), mTheme );
  mapcanvasNode.setAttribute( QStringLiteral( "annotationsVisible" ), mAnnotationsVisible );
  qgisNode.appendChild( mapcanvasNode );

  mSettings.writeXml( mapcanvasNode, doc );

  // store canvas expression context
  QDomElement scopeElement = doc.createElement( QStringLiteral( "expressionContextScope" ) );
  QgsExpressionContextScope tmpScope( mExpressionContextScope );
  tmpScope.removeVariable( QStringLiteral( "atlas_featurenumber" ) );
  tmpScope.removeVariable( QStringLiteral( "atlas_pagename" ) );
  tmpScope.removeVariable( QStringLiteral( "atlas_feature" ) );
  tmpScope.removeVariable( QStringLiteral( "atlas_featureid" ) );
  tmpScope.removeVariable( QStringLiteral( "atlas_geometry" ) );
  tmpScope.writeXml( scopeElement, doc, QgsReadWriteContext() );
  mapcanvasNode.appendChild( scopeElement );

  // TODO: store only units, extent, projections, dest CRS
}

void QgsMapCanvas::zoomByFactor( double scaleFactor, const QgsPointXY *center, bool ignoreScaleLock )
{
  if ( mScaleLocked && !ignoreScaleLock )
  {
    ScaleRestorer restorer( this );
    setMagnificationFactor( mapSettings().magnificationFactor() / scaleFactor, center );
  }
  else
  {
    QgsRectangle r = mapSettings().extent();
    r.scale( scaleFactor, center );
    setExtent( r, true );
    refresh();
  }
}

void QgsMapCanvas::selectionChangedSlot()
{
  // Find out which layer it was that sent the signal.
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );
  if ( layer )
  {
    emit selectionChanged( layer );
    refresh();
  }
}

void QgsMapCanvas::dragEnterEvent( QDragEnterEvent *event )
{
  // By default graphics view delegates the drag events to graphics items.
  // But we do not want that and by ignoring the drag enter we let the
  // parent (e.g. QgisApp) to handle drops of map layers etc.

  // so we ONLY accept the event if we know in advance that a custom drop handler
  // wants it

  if ( QgsMimeDataUtils::isUriList( event->mimeData() ) )
  {
    const QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( event->mimeData() );
    bool allHandled = true;
    for ( const QgsMimeDataUtils::Uri &uri : lst )
    {
      bool handled = false;
      for ( QgsCustomDropHandler *handler : std::as_const( mDropHandlers ) )
      {
        if ( handler->canHandleCustomUriCanvasDrop( uri, this ) )
        {
          handled = true;
          break;
        }
      }
      if ( !handled )
        allHandled = false;
    }
    if ( allHandled )
      event->accept();
    else
      event->ignore();
  }
  else
  {
    event->ignore();
  }
}

bool QgsMapCanvas::viewportEvent( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip && mMapTool && mMapTool->canvasToolTipEvent( qgis::down_cast<QHelpEvent *>( event ) ) )
  {
    return true;
  }
  return QGraphicsView::viewportEvent( event );
}

void QgsMapCanvas::mapToolDestroyed()
{
  QgsDebugMsgLevel( QStringLiteral( "maptool destroyed" ), 2 );
  mMapTool = nullptr;
}

bool QgsMapCanvas::event( QEvent *e )
{
  if ( !QTouchDevice::devices().empty() )
  {
    if ( e->type() == QEvent::Gesture )
    {
      if ( QTapAndHoldGesture *tapAndHoldGesture = qobject_cast< QTapAndHoldGesture * >( static_cast<QGestureEvent *>( e )->gesture( Qt::TapAndHoldGesture ) ) )
      {
        QPointF pos = tapAndHoldGesture->position();
        pos = mapFromGlobal( QPoint( pos.x(), pos.y() ) );
        QgsPointXY mapPoint = getCoordinateTransform()->toMapCoordinates( pos.x(), pos.y() );
        emit tapAndHoldGestureOccurred( mapPoint, tapAndHoldGesture );
      }

      // call handler of current map tool
      if ( mMapTool )
      {
        return mMapTool->gestureEvent( static_cast<QGestureEvent *>( e ) );
      }
    }
  }

  // pass other events to base class
  return QGraphicsView::event( e );
}

void QgsMapCanvas::refreshAllLayers()
{
  // reload all layers in canvas
  const QList<QgsMapLayer *> layers = mapSettings().layers();
  for ( QgsMapLayer *layer : layers )
  {
    layer->reload();
  }

  redrawAllLayers();
}

void QgsMapCanvas::redrawAllLayers()
{
  // clear the cache
  clearCache();

  // and then refresh
  refresh();
}

void QgsMapCanvas::waitWhileRendering()
{
  while ( mRefreshScheduled || mJob )
  {
    QgsApplication::processEvents();
  }
}

void QgsMapCanvas::setSegmentationTolerance( double tolerance )
{
  mSettings.setSegmentationTolerance( tolerance );
}

void QgsMapCanvas::setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type )
{
  mSettings.setSegmentationToleranceType( type );
}

QList<QgsMapCanvasAnnotationItem *> QgsMapCanvas::annotationItems() const
{
  QList<QgsMapCanvasAnnotationItem *> annotationItemList;
  const QList<QGraphicsItem *> items = mScene->items();
  for ( QGraphicsItem *gi : items )
  {
    QgsMapCanvasAnnotationItem *aItem = dynamic_cast< QgsMapCanvasAnnotationItem *>( gi );
    if ( aItem )
    {
      annotationItemList.push_back( aItem );
    }
  }

  return annotationItemList;
}

void QgsMapCanvas::setAnnotationsVisible( bool show )
{
  mAnnotationsVisible = show;
  const QList<QgsMapCanvasAnnotationItem *> items = annotationItems();
  for ( QgsMapCanvasAnnotationItem *item : items )
  {
    item->setVisible( show );
  }
}

void QgsMapCanvas::setLabelingEngineSettings( const QgsLabelingEngineSettings &settings )
{
  mSettings.setLabelingEngineSettings( settings );
}

const QgsLabelingEngineSettings &QgsMapCanvas::labelingEngineSettings() const
{
  return mSettings.labelingEngineSettings();
}

void QgsMapCanvas::startPreviewJobs()
{
  stopPreviewJobs(); //just in case still running

  //canvas preview jobs aren't compatible with rotation
  // TODO fix this
  if ( !qgsDoubleNear( mSettings.rotation(), 0.0 ) )
    return;

  schedulePreviewJob( 0 );
}

void QgsMapCanvas::startPreviewJob( int number )
{
  QgsRectangle mapRect = mSettings.visibleExtent();

  if ( number == 4 )
    number += 1;

  int j = number / 3;
  int i = number % 3;

  //copy settings, only update extent
  QgsMapSettings jobSettings = mSettings;

  double dx = ( i - 1 ) * mapRect.width();
  double dy = ( 1 - j ) * mapRect.height();
  QgsRectangle jobExtent = mapRect;

  jobExtent.setXMaximum( jobExtent.xMaximum() + dx );
  jobExtent.setXMinimum( jobExtent.xMinimum() + dx );
  jobExtent.setYMaximum( jobExtent.yMaximum() + dy );
  jobExtent.setYMinimum( jobExtent.yMinimum() + dy );

  jobSettings.setExtent( jobExtent );
  jobSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );
  jobSettings.setFlag( Qgis::MapSettingsFlag::RenderPreviewJob, true );

  // truncate preview layers to fast layers
  const QList<QgsMapLayer *> layers = jobSettings.layers();
  QList< QgsMapLayer * > previewLayers;
  QgsDataProvider::PreviewContext context;
  context.maxRenderingTimeMs = MAXIMUM_LAYER_PREVIEW_TIME_MS;
  for ( QgsMapLayer *layer : layers )
  {
    context.lastRenderingTimeMs = mLastLayerRenderTime.value( layer->id(), 0 );
    QgsDataProvider *provider = layer->dataProvider();
    if ( provider && !provider->renderInPreview( context ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Layer %1 not rendered because it does not match the renderInPreview criterion %2" ).arg( layer->id() ).arg( mLastLayerRenderTime.value( layer->id() ) ), 3 );
      continue;
    }

    previewLayers << layer;
  }
  jobSettings.setLayers( previewLayers );

  QgsMapRendererQImageJob *job = new QgsMapRendererSequentialJob( jobSettings );
  job->setProperty( "number", number );
  mPreviewJobs.append( job );
  connect( job, &QgsMapRendererJob::finished, this, &QgsMapCanvas::previewJobFinished );
  job->start();
}

void QgsMapCanvas::stopPreviewJobs()
{
  mPreviewTimer.stop();
  const auto previewJobs = mPreviewJobs;
  for ( auto previewJob : previewJobs )
  {
    if ( previewJob )
    {
      disconnect( previewJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::previewJobFinished );
      connect( previewJob, &QgsMapRendererQImageJob::finished, previewJob, &QgsMapRendererQImageJob::deleteLater );
      previewJob->cancelWithoutBlocking();
    }
  }
  mPreviewJobs.clear();
}

void QgsMapCanvas::schedulePreviewJob( int number )
{
  mPreviewTimer.setSingleShot( true );
  mPreviewTimer.setInterval( PREVIEW_JOB_DELAY_MS );
  disconnect( mPreviewTimerConnection );
  mPreviewTimerConnection = connect( &mPreviewTimer, &QTimer::timeout, this, [ = ]()
  {
    startPreviewJob( number );
  } );
  mPreviewTimer.start();
}

bool QgsMapCanvas::panOperationInProgress()
{
  if ( mCanvasProperties->panSelectorDown )
    return true;

  if ( QgsMapToolPan *panTool = qobject_cast< QgsMapToolPan *>( mMapTool ) )
  {
    if ( panTool->isDragging() )
      return true;
  }

  return false;
}

int QgsMapCanvas::nextZoomLevel( const QList<double> &resolutions, bool zoomIn ) const
{
  int resolutionLevel = -1;
  double currentResolution = mapUnitsPerPixel();

  for ( int i = 0, n = resolutions.size(); i < n; ++i )
  {
    if ( qgsDoubleNear( resolutions[i], currentResolution, 0.0001 ) )
    {
      resolutionLevel = zoomIn ? ( i - 1 ) : ( i + 1 );
      break;
    }
    else if ( currentResolution <= resolutions[i] )
    {
      resolutionLevel = zoomIn ? ( i - 1 ) : i;
      break;
    }
  }
  return ( resolutionLevel < 0 || resolutionLevel >= resolutions.size() ) ? -1 : resolutionLevel;
}

double QgsMapCanvas::zoomInFactor() const
{
  if ( !mZoomResolutions.isEmpty() )
  {
    int zoomLevel = nextZoomLevel( mZoomResolutions, true );
    if ( zoomLevel != -1 )
    {
      return mZoomResolutions.at( zoomLevel ) / mapUnitsPerPixel();
    }
  }
  return 1 / mWheelZoomFactor;
}

double QgsMapCanvas::zoomOutFactor() const
{
  if ( !mZoomResolutions.isEmpty() )
  {
    int zoomLevel = nextZoomLevel( mZoomResolutions, false );
    if ( zoomLevel != -1 )
    {
      return mZoomResolutions.at( zoomLevel ) / mapUnitsPerPixel();
    }
  }
  return mWheelZoomFactor;
}
