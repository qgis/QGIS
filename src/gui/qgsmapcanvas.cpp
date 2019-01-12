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
#include <cmath>

/**
 * \ingroup gui
 * Deprecated to be deleted, stuff from here should be moved elsewhere.
 * \note not available in Python bindings
*/
//TODO QGIS 3.0 - remove
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

  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemeChanged, this, &QgsMapCanvas::mapThemeChanged );
  connect( QgsProject::instance()->mapThemeCollection(), &QgsMapThemeCollection::mapThemesChanged, this, &QgsMapCanvas::projectThemesChanged );

  mSettings.setFlag( QgsMapSettings::DrawEditingInfo );
  mSettings.setFlag( QgsMapSettings::UseRenderingOptimization );
  mSettings.setFlag( QgsMapSettings::RenderPartialOutput );
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

  // refresh canvas when a remote svg/image has finished downloading
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsMapCanvas::refreshAllLayers );
  connect( QgsApplication::imageCache(), &QgsImageCache::remoteImageFetched, this, &QgsMapCanvas::refreshAllLayers );
  // refresh canvas when project color scheme is changed -- if layers use project colors, they need to be redrawn
  connect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, &QgsMapCanvas::refreshAllLayers );

  //segmentation parameters
  QgsSettings settings;
  double segmentationTolerance = settings.value( QStringLiteral( "qgis/segmentationTolerance" ), "0.01745" ).toDouble();
  QgsAbstractGeometry::SegmentationToleranceType toleranceType = settings.enumValue( QStringLiteral( "qgis/segmentationToleranceType" ), QgsAbstractGeometry::MaximumAngle );
  mSettings.setSegmentationTolerance( segmentationTolerance );
  mSettings.setSegmentationToleranceType( toleranceType );

  mWheelZoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  QSize s = viewport()->size();
  mSettings.setOutputSize( s );
  mSettings.setDevicePixelRatio( devicePixelRatio() );
  setSceneRect( 0, 0, s.width(), s.height() );
  mScene->setSceneRect( QRectF( 0, 0, s.width(), s.height() ) );

  moveCanvasContents( true );

  // keep device pixel ratio up to date on screen or resolution change
  if ( window()->windowHandle() )
  {
    connect( window()->windowHandle(), &QWindow::screenChanged, this, [ = ]( QScreen * ) {mSettings.setDevicePixelRatio( devicePixelRatio() );} );
    connect( window()->windowHandle()->screen(), &QScreen::physicalDotsPerInchChanged, this, [ = ]( qreal ) {mSettings.setDevicePixelRatio( devicePixelRatio() );} );
  }

  connect( &mMapUpdateTimer, &QTimer::timeout, this, &QgsMapCanvas::mapUpdateTimeout );
  mMapUpdateTimer.setInterval( 250 );

#ifdef Q_OS_WIN
  // Enable touch event on Windows.
  // Qt on Windows needs to be told it can take touch events or else it ignores them.
  grabGesture( Qt::PinchGesture );
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

  refresh();

} // QgsMapCanvas ctor


QgsMapCanvas::~QgsMapCanvas()
{
  if ( mMapTool )
  {
    mMapTool->deactivate();
    mMapTool = nullptr;
  }
  mLastNonZoomMapTool = nullptr;

  // rendering job may still end up writing into canvas map item
  // so kill it before deleting canvas items
  if ( mJob )
  {
    whileBlocking( mJob )->cancel();
    delete mJob;
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

  // delete canvas items prior to deleting the canvas
  // because they might try to update canvas when it's
  // already being destructed, ends with segfault
  QList<QGraphicsItem *> list = mScene->items();
  QList<QGraphicsItem *>::iterator it = list.begin();
  while ( it != list.end() )
  {
    QGraphicsItem *item = *it;
    delete item;
    ++it;
  }

  mScene->deleteLater();  // crashes in python tests on windows

  delete mCache;
  delete mLabelingResults;
}

void QgsMapCanvas::setMagnificationFactor( double factor )
{
  // do not go higher or lower than min max magnification ratio
  double magnifierMin = QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  factor = qBound( magnifierMin, factor, magnifierMax );

  // the magnifier widget is in integer percent
  if ( !qgsDoubleNear( factor, mSettings.magnificationFactor(), 0.01 ) )
  {
    mSettings.setMagnificationFactor( factor );
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
  mSettings.setFlag( QgsMapSettings::Antialiasing, flag );
} // anti aliasing

void QgsMapCanvas::enableMapTileRendering( bool flag )
{
  mSettings.setFlag( QgsMapSettings::RenderMapTile, flag );
}

QgsMapLayer *QgsMapCanvas::layer( int index )
{
  QList<QgsMapLayer *> layers = mapSettings().layers();
  if ( index >= 0 && index < layers.size() )
    return layers[index];
  else
    return nullptr;
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

  Q_FOREACH ( QgsMapLayer *layer, oldLayers )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
    disconnect( layer, &QgsMapLayer::autoRefreshIntervalChanged, this, &QgsMapCanvas::updateAutoRefreshTimer );
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &QgsMapCanvas::selectionChangedSlot );
    }
  }

  mSettings.setLayers( layers );

  Q_FOREACH ( QgsMapLayer *layer, layers )
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

  QgsDebugMsg( QStringLiteral( "Layers have changed, refreshing" ) );
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
    try
    {
      rect = transform.transformBoundingBox( mSettings.visibleExtent() );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( QStringLiteral( "Transform error caught: %1" ).arg( e.what() ) );
    }
  }

  if ( !rect.isEmpty() )
  {
    setExtent( rect );
  }

  mSettings.setDestinationCrs( crs );
  updateScale();

  QgsDebugMsg( QStringLiteral( "refreshing after destination CRS changed" ) );
  refresh();

  emit destinationCrsChanged();
}

void QgsMapCanvas::setMapSettingsFlags( QgsMapSettings::Flags flags )
{
  mSettings.setFlags( flags );
  clearCache();
  refresh();
}

const QgsLabelingResults *QgsMapCanvas::labelingResults() const
{
  return mLabelingResults;
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
}

bool QgsMapCanvas::isCachingEnabled() const
{
  return nullptr != mCache;
}

void QgsMapCanvas::clearCache()
{
  if ( mCache )
    mCache->clear();
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

QgsExpressionContextScope *QgsMapCanvas::defaultExpressionContextScope()
{
  QgsExpressionContextScope *s = new QgsExpressionContextScope( QObject::tr( "Map Canvas" ) );
  s->setVariable( QStringLiteral( "canvas_cursor_point" ), QgsGeometry::fromPointXY( cursorPoint() ), true );

  return s;
}

void QgsMapCanvas::refresh()
{
  if ( !mSettings.hasValidSettings() )
  {
    QgsDebugMsg( QStringLiteral( "CANVAS refresh - invalid settings -> nothing to do" ) );
    return;
  }

  if ( !mRenderFlag || mFrozen )
  {
    QgsDebugMsg( QStringLiteral( "CANVAS render flag off" ) );
    return;
  }

  if ( mRefreshScheduled )
  {
    QgsDebugMsg( QStringLiteral( "CANVAS refresh already scheduled" ) );
    return;
  }

  mRefreshScheduled = true;

  QgsDebugMsg( QStringLiteral( "CANVAS refresh scheduling" ) );

  // schedule a refresh
  mRefreshTimer->start( 1 );
} // refresh

void QgsMapCanvas::refreshMap()
{
  Q_ASSERT( mRefreshScheduled );

  QgsDebugMsgLevel( QStringLiteral( "CANVAS refresh!" ), 3 );

  stopRendering(); // if any...
  stopPreviewJobs();

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                    << QgsExpressionContextUtils::atlasScope( nullptr )
                    << QgsExpressionContextUtils::mapSettingsScope( mSettings )
                    << defaultExpressionContextScope()
                    << new QgsExpressionContextScope( mExpressionContextScope );

  mSettings.setExpressionContext( expressionContext );
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

  // create the renderer job
  Q_ASSERT( !mJob );
  mJobCanceled = false;
  if ( mUseParallelRendering )
    mJob = new QgsMapRendererParallelJob( mSettings );
  else
    mJob = new QgsMapRendererSequentialJob( mSettings );
  connect( mJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::rendererJobFinished );
  mJob->setCache( mCache );

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


void QgsMapCanvas::rendererJobFinished()
{
  QgsDebugMsg( QStringLiteral( "CANVAS finish! %1" ).arg( !mJobCanceled ) );

  mMapUpdateTimer.stop();

  // TODO: would be better to show the errors in message bar
  Q_FOREACH ( const QgsMapRendererJob::Error &error, mJob->errors() )
  {
    QgsMessageLog::logMessage( error.layerID + " :: " + error.message, tr( "Rendering" ) );
  }

  if ( !mJobCanceled )
  {
    // take labeling results before emitting renderComplete, so labeling map tools
    // connected to signal work with correct results
    if ( !mJob->usedCachedLabels() )
    {
      delete mLabelingResults;
      mLabelingResults = mJob->takeLabelingResults();
    }

    QImage img = mJob->renderedImage();

    // emit renderComplete to get our decorations drawn
    QPainter p( &img );
    emit renderComplete( &p );

    QgsSettings settings;
    if ( settings.value( QStringLiteral( "Map/logCanvasRefreshEvent" ), false ).toBool() )
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
    if ( mUsePreviewJobs )
      startPreviewJobs();
  }

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;

  emit mapCanvasRefreshed();
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
    QgsDebugMsg( QStringLiteral( "CANVAS stop rendering!" ) );
    mJobCanceled = true;
    disconnect( mJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::rendererJobFinished );
    connect( mJob, &QgsMapRendererQImageJob::finished, mJob, &QgsMapRendererQImageJob::deleteLater );
    mJob->cancelWithoutBlocking();
    mJob = nullptr;
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

    painter.save();

    QPointF itemScenePos = item->scenePos();
    painter.translate( itemScenePos.x(), itemScenePos.y() );

    item->paint( &painter, &option );

    painter.restore();
  }

  painter.end();
  image.save( fileName, format.toLocal8Bit().data() );

  QFileInfo myInfo  = QFileInfo( fileName );

  // build the world file name
  QString outputSuffix = myInfo.suffix();
  QString myWorldFileName = myInfo.absolutePath() + '/' + myInfo.baseName() + '.'
                            + outputSuffix.at( 0 ) + outputSuffix.at( myInfo.suffix().size() - 1 ) + 'w';
  QFile myWorldFile( myWorldFileName );
  if ( !myWorldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) //don't use QIODevice::Text
  {
    return;
  }
  QTextStream myStream( &myWorldFile );
  myStream << QgsMapSettingsUtils::worldFileContent( mapSettings() );
} // saveAsImage



QgsRectangle QgsMapCanvas::extent() const
{
  return mapSettings().visibleExtent();
} // extent

QgsRectangle QgsMapCanvas::fullExtent() const
{
  return mapSettings().fullExtent();
} // extent


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
      QgsDebugMsg( QStringLiteral( "Empty extent - ignoring" ) );
      return;
    }

    // ### QGIS 3: do not allow empty extent - require users to call setCenter() explicitly
    QgsDebugMsg( QStringLiteral( "Empty extent - keeping old scale with new center!" ) );
    setCenter( r.center() );
  }
  else
  {
    mSettings.setExtent( r, magnified );
  }
  emit extentsChanged();
  updateScale();
  if ( mLastExtent.size() > 20 )
    mLastExtent.removeAt( 0 );

  //clear all extent items after current index
  for ( int i = mLastExtent.size() - 1; i > mLastExtentIndex; i-- )
  {
    mLastExtent.removeAt( i );
  }

  mLastExtent.append( extent() );

  // adjust history to no more than 20
  if ( mLastExtent.size() > 20 )
  {
    mLastExtent.removeAt( 0 );
  }

  // the last item is the current extent
  mLastExtentIndex = mLastExtent.size() - 1;

  // update controls' enabled state
  emit zoomLastStatusChanged( mLastExtentIndex > 0 );
  emit zoomNextStatusChanged( mLastExtentIndex < mLastExtent.size() - 1 );
} // setExtent

void QgsMapCanvas::setCenter( const QgsPointXY &center )
{
  QgsRectangle r = mapSettings().extent();
  double x = center.x();
  double y = center.y();
  setExtent(
    QgsRectangle(
      x - r.width() / 2.0, y - r.height() / 2.0,
      x + r.width() / 2.0, y + r.height() / 2.0
    ),
    true
  );
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

} // zoomToFullExtent


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

void QgsMapCanvas::zoomToSelected( QgsVectorLayer *layer )
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
    emit messageEmitted( tr( "Cannot zoom to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::Warning );
    return;
  }

  rect = mapSettings().layerExtentToOutputExtent( layer, rect );

  // zoom in if point cannot be distinguished from others
  // also check that rect is empty, as it might not in case of multi points
  if ( layer->geometryType() == QgsWkbTypes::PointGeometry && rect.isEmpty() )
  {
    int scaleFactor = 5;
    QgsPointXY center = mSettings.mapToLayerCoordinates( layer, rect.center() );
    QgsRectangle extentRect = mSettings.mapToLayerCoordinates( layer, extent() ).scaled( 1.0 / scaleFactor, &center );
    QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( extentRect ).setLimit( 1000 ).setNoAttributes();
    QgsFeatureIterator fit = layer->getFeatures( req );
    QgsFeature f;
    QgsPointXY closestPoint;
    double closestSquaredDistance = extentRect.width() + extentRect.height();
    bool pointFound = false;
    while ( fit.nextFeature( f ) )
    {
      QgsPointXY point = f.geometry().asPoint();
      double sqrDist = point.sqrDist( center );
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

  zoomToFeatureExtent( rect );
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
    zoomToFeatureExtent( bbox );
  }
  else
  {
    emit messageEmitted( tr( "Zoom to feature id failed" ), errorMsg, Qgis::Warning );
  }

}

void QgsMapCanvas::panToFeatureIds( QgsVectorLayer *layer, const QgsFeatureIds &ids )
{
  if ( !layer )
  {
    return;
  }

  QgsRectangle bbox;
  QString errorMsg;
  if ( boundingBoxOfFeatureIds( ids, layer, bbox, errorMsg ) )
  {
    setCenter( bbox.center() );
    refresh();
  }
  else
  {
    emit messageEmitted( tr( "Pan to feature id failed" ), errorMsg, Qgis::Warning );
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
    emit messageEmitted( tr( "Cannot pan to selected feature(s)" ), tr( "No extent could be determined." ), Qgis::Warning );
    return;
  }

  rect = mapSettings().layerExtentToOutputExtent( layer, rect );
  setCenter( rect.center() );
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
    rb->addGeometry( geom, crs );

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

  if ( ! mCanvasProperties->mouseButtonDown )
  {
    // Don't want to interfer with mouse events

    QgsRectangle currentExtent = mapSettings().visibleExtent();
    double dx = std::fabs( currentExtent.width() / 4 );
    double dy = std::fabs( currentExtent.height() / 4 );

    switch ( e->key() )
    {
      case Qt::Key_Left:
        QgsDebugMsg( QStringLiteral( "Pan left" ) );
        setCenter( center() - QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Right:
        QgsDebugMsg( QStringLiteral( "Pan right" ) );
        setCenter( center() + QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Up:
        QgsDebugMsg( QStringLiteral( "Pan up" ) );
        setCenter( center() + QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Down:
        QgsDebugMsg( QStringLiteral( "Pan down" ) );
        setCenter( center() - QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;



      case Qt::Key_Space:
        QgsDebugMsg( QStringLiteral( "Pressing pan selector" ) );

        //mCanvasProperties->dragging = true;
        if ( ! e->isAutoRepeat() )
        {
          QApplication::setOverrideCursor( Qt::ClosedHandCursor );
          mCanvasProperties->panSelectorDown = true;
          mCanvasProperties->rubberStartPoint = mCanvasProperties->mouseLastXY;
        }
        break;

      case Qt::Key_PageUp:
        QgsDebugMsg( QStringLiteral( "Zoom in" ) );
        zoomIn();
        break;

      case Qt::Key_PageDown:
        QgsDebugMsg( QStringLiteral( "Zoom out" ) );
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
        if ( mMapTool )
        {
          mMapTool->keyPressEvent( e );
        }
        else e->ignore();

        QgsDebugMsg( "Ignoring key: " + QString::number( e->key() ) );
    }
  }

  emit keyPressed( e );

} //keyPressEvent()

void QgsMapCanvas::keyReleaseEvent( QKeyEvent *e )
{
  QgsDebugMsg( QStringLiteral( "keyRelease event" ) );

  switch ( e->key() )
  {
    case Qt::Key_Space:
      if ( !e->isAutoRepeat() && mCanvasProperties->panSelectorDown )
      {
        QgsDebugMsg( QStringLiteral( "Releasing pan selector" ) );
        QApplication::restoreOverrideCursor();
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

      QgsDebugMsg( "Ignoring key release: " + QString::number( e->key() ) );
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
  QApplication::setOverrideCursor( mZoomCursor );
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
  QApplication::restoreOverrideCursor();

  // store the rectangle
  mZoomRect.setRight( pos.x() );
  mZoomRect.setBottom( pos.y() );

  if ( mZoomRect.width() < 5 && mZoomRect.height() < 5 )
  {
    //probably a mistake - would result in huge zoom!
    return;
  }

  //account for bottom right -> top left dragging
  mZoomRect = mZoomRect.normalized();

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
  //use middle mouse button for panning, map tools won't receive any events in that case
  if ( e->button() == Qt::MidButton )
  {
    mCanvasProperties->panSelectorDown = true;
    mCanvasProperties->rubberStartPoint = mCanvasProperties->mouseLastXY;
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

} // mousePressEvent


void QgsMapCanvas::mouseReleaseEvent( QMouseEvent *e )
{
  //use middle mouse button for panning, map tools won't receive any events in that case
  if ( e->button() == Qt::MidButton )
  {
    mCanvasProperties->panSelectorDown = false;
    panActionEnd( mCanvasProperties->mouseLastXY );
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
        QgsDebugMsg( QStringLiteral( "Right click in map tool zoom or pan, last tool is %1." ).arg(
                       mLastNonZoomMapTool ? QStringLiteral( "not null" ) : QStringLiteral( "null" ) ) );

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

} // mouseReleaseEvent

void QgsMapCanvas::resizeEvent( QResizeEvent *e )
{
  QGraphicsView::resizeEvent( e );
  mResizeTimer->start( 500 );

  QSize lastSize = viewport()->size();

  mSettings.setOutputSize( lastSize );

  mScene->setSceneRect( QRectF( 0, 0, lastSize.width(), lastSize.height() ) );

  moveCanvasContents( true );

  updateScale();

  //refresh();

  emit extentsChanged();
}

void QgsMapCanvas::paintEvent( QPaintEvent *e )
{
  // no custom event handling anymore

  QGraphicsView::paintEvent( e );
} // paintEvent

void QgsMapCanvas::updateCanvasItemPositions()
{
  QList<QGraphicsItem *> list = mScene->items();
  QList<QGraphicsItem *>::iterator it = list.begin();
  while ( it != list.end() )
  {
    QgsMapCanvasItem *item = dynamic_cast<QgsMapCanvasItem *>( *it );

    if ( item )
    {
      item->updatePosition();
    }

    ++it;
  }
}


void QgsMapCanvas::wheelEvent( QWheelEvent *e )
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in

  QgsDebugMsg( "Wheel event delta " + QString::number( e->delta() ) );

  if ( mMapTool )
  {
    mMapTool->wheelEvent( e );
    if ( e->isAccepted() )
      return;
  }

  if ( e->delta() == 0 )
  {
    e->accept();
    return;
  }

  double zoomFactor = mWheelZoomFactor;

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
  QgsPointXY mousePos( getCoordinateTransform()->toMapCoordinates( e->x(), e->y() ) );
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
  zoomByFactor( 1 / mWheelZoomFactor );
}

void QgsMapCanvas::zoomOut()
{
  // magnification is alreday handled in zoomByFactor
  zoomByFactor( mWheelZoomFactor );
}

void QgsMapCanvas::zoomScale( double newScale )
{
  zoomByFactor( newScale / scale() );
}

void QgsMapCanvas::zoomWithCenter( int x, int y, bool zoomIn )
{
  double scaleFactor = ( zoomIn ? 1 / mWheelZoomFactor : mWheelZoomFactor );

  if ( mScaleLocked )
  {
    setMagnificationFactor( mapSettings().magnificationFactor() / scaleFactor );
  }
  else
  {
    // transform the mouse pos to map coordinates
    QgsPointXY center  = getCoordinateTransform()->toMapCoordinates( x, y );
    QgsRectangle r = mapSettings().visibleExtent();
    r.scale( scaleFactor, &center );
    setExtent( r, true );
    refresh();
  }
}

void QgsMapCanvas::setScaleLocked( bool isLocked )
{
  mScaleLocked = isLocked;
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

  // show x y on status bar
  mCursorPoint = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->mouseLastXY );
  emit xyCoordinates( mCursorPoint );
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
  if ( mMapTool )
  {
    connect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    mMapTool->activate();
  }

  emit mapToolSet( mMapTool, oldTool );
} // setMapTool

void QgsMapCanvas::unsetMapTool( QgsMapTool *tool )
{
  if ( mMapTool && mMapTool == tool )
  {
    mMapTool->deactivate();
    mMapTool = nullptr;
    emit mapToolSet( nullptr, mMapTool );
    setCursor( Qt::ArrowCursor );
  }

  if ( mLastNonZoomMapTool && mLastNonZoomMapTool == tool )
  {
    mLastNonZoomMapTool = nullptr;
  }
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

  emit canvasColorChanged();
}

QColor QgsMapCanvas::canvasColor() const
{
  return mScene->backgroundBrush().color();
}

void QgsMapCanvas::setSelectionColor( const QColor &color )
{
  mSettings.setSelectionColor( color );
}

QColor QgsMapCanvas::selectionColor() const
{
  return mSettings.selectionColor();
}

int QgsMapCanvas::layerCount() const
{
  return mapSettings().layers().size();
} // layerCount


QList<QgsMapLayer *> QgsMapCanvas::layers() const
{
  return mapSettings().layers();
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
  Q_UNUSED( signal );
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
    // canvas is currently being redrawn, so we skip this auto refresh
    // otherwise we could get stuck in the situation where an auto refresh is triggered
    // too often to allow the canvas to ever finish rendering
    return;
  }

  refresh();
}

void QgsMapCanvas::updateAutoRefreshTimer()
{
  // min auto refresh interval stores the smallest interval between layer auto refreshes. We automatically
  // trigger a map refresh on this minimum interval
  int minAutoRefreshInterval = -1;
  Q_FOREACH ( QgsMapLayer *layer, mSettings.layers() )
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

void QgsMapCanvas::panAction( QMouseEvent *e )
{
  Q_UNUSED( e );

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
    if ( objectName() != QStringLiteral( "theMapCanvas" ) )
    {
      // never manually set the crs for the main canvas - this is instead connected to the project CRS
      setDestinationCrs( tmpSettings.destinationCrs() );
    }
    setExtent( tmpSettings.extent() );
    setRotation( tmpSettings.rotation() );
    enableMapTileRendering( tmpSettings.testFlag( QgsMapSettings::RenderMapTile ) );

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
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Couldn't read mapcanvas information from project" ) );
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
  // TODO: store only units, extent, projections, dest CRS
}

#if 0
void QgsMapCanvas::getDatumTransformInfo( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination )
{
  if ( !source.isValid() || !destination.isValid() )
    return;

  //check if default datum transformation available
  QgsSettings s;
  QString settingsString = "/Projections/" + source.authid() + "//" + destination.authid();
  QVariant defaultSrcTransform = s.value( settingsString + "_srcTransform" );
  QVariant defaultDestTransform = s.value( settingsString + "_destTransform" );
  if ( defaultSrcTransform.isValid() && defaultDestTransform.isValid() )
  {
    int sourceDatumTransform = defaultSrcTransform.toInt();
    int destinationDatumTransform = defaultDestTransform.toInt();

    QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
    context.addSourceDestinationDatumTransform( source, destination, sourceDatumTransform, destinationDatumTransform );
    QgsProject::instance()->setTransformContext( context );
    return;
  }

  if ( !s.value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool() )
  {
    return;
  }

  //if several possibilities:  present dialog
  QgsDatumTransformDialog d( source, destination );
  if ( d.availableTransformationCount() > 1 )
    d.exec();
}
#endif

void QgsMapCanvas::zoomByFactor( double scaleFactor, const QgsPointXY *center )
{
  if ( mScaleLocked )
  {
    // zoom map to mouse cursor by magnifying
    setMagnificationFactor( mapSettings().magnificationFactor() / scaleFactor );
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

void QgsMapCanvas::dragEnterEvent( QDragEnterEvent *e )
{
  // By default graphics view delegates the drag events to graphics items.
  // But we do not want that and by ignoring the drag enter we let the
  // parent (e.g. QgisApp) to handle drops of map layers etc.
  e->ignore();
}

void QgsMapCanvas::mapToolDestroyed()
{
  QgsDebugMsg( QStringLiteral( "maptool destroyed" ) );
  mMapTool = nullptr;
}

bool QgsMapCanvas::event( QEvent *e )
{
  if ( !QTouchDevice::devices().empty() )
  {
    if ( e->type() == QEvent::Gesture )
    {
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
  for ( int i = 0; i < layerCount(); i++ )
  {
    QgsMapLayer *l = layer( i );
    if ( l )
      l->reload();
  }

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
  QList<QGraphicsItem *> itemList = mScene->items();
  QList<QGraphicsItem *>::iterator it = itemList.begin();
  for ( ; it != itemList.end(); ++it )
  {
    QgsMapCanvasAnnotationItem *aItem = dynamic_cast< QgsMapCanvasAnnotationItem *>( *it );
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
  Q_FOREACH ( QgsMapCanvasAnnotationItem *item, annotationItems() )
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
  jobSettings.setFlag( QgsMapSettings::DrawLabeling, false );
  jobSettings.setFlag( QgsMapSettings::RenderPreviewJob, true );

  // truncate preview layers to fast layers
  const QList<QgsMapLayer *> layers = jobSettings.layers();
  QList< QgsMapLayer * > previewLayers;
  QgsDataProvider::PreviewContext context;
  context.maxRenderingTimeMs = MAXIMUM_LAYER_PREVIEW_TIME_MS;
  for ( QgsMapLayer *layer : layers )
  {
    context.lastRenderingTimeMs = mLastLayerRenderTime.value( layer->id(), 0 );
    if ( !layer->dataProvider()->renderInPreview( context ) )
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
  QList< QgsMapRendererQImageJob * >::const_iterator it = mPreviewJobs.constBegin();
  for ( ; it != mPreviewJobs.constEnd(); ++it )
  {
    if ( *it )
    {
      disconnect( *it, &QgsMapRendererJob::finished, this, &QgsMapCanvas::previewJobFinished );
      connect( *it, &QgsMapRendererQImageJob::finished, *it, &QgsMapRendererQImageJob::deleteLater );
      ( *it )->cancelWithoutBlocking();
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
