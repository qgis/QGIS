/***************************************************************************
  qgsmapcanvas.cpp  -  description
  -------------------
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
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QRect>
#include <QSettings>
#include <QTextStream>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QWheelEvent>

#include "qgis.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsapplication.h"
#include "qgscsexception.h"
#include "qgsdatumtransformdialog.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgsmaplayer.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptopixel.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderercache.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmessagelog.h"
#include "qgsmessageviewer.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgscursors.h"
#include <cmath>


/** \ingroup gui
 * Deprecated to be deleted, stuff from here should be moved elsewhere.
 * @note not available in Python bindings
*/
//TODO QGIS 3.0 - remove
class QgsMapCanvas::CanvasProperties
{
  public:
    CanvasProperties()
        : mouseButtonDown( false )
        , panSelectorDown( false )
    { }

    //!Flag to indicate status of mouse button
    bool mouseButtonDown;

    //! Last seen point of the mouse
    QPoint mouseLastXY;

    //! Beginning point of a rubber band
    QPoint rubberStartPoint;

    //! Flag to indicate the pan selector key is held down by user
    bool panSelectorDown;
};



QgsMapCanvas::QgsMapCanvas( QWidget * parent )
    : QGraphicsView( parent )
    , mCanvasProperties( new CanvasProperties )
    , mMap( nullptr )
    , mFrozen( false )
    , mRefreshScheduled( false )
    , mRenderFlag( true ) // by default, the canvas is rendered
    , mCurrentLayer( nullptr )
    , mScene( nullptr )
    , mMapTool( nullptr )
    , mLastNonZoomMapTool( nullptr )
    , mLastExtentIndex( -1 )
    , mWheelZoomFactor( 2.0 )
    , mJob( nullptr )
    , mJobCanceled( false )
    , mLabelingResults( nullptr )
    , mUseParallelRendering( false )
    , mDrawRenderingStats( false )
    , mCache( nullptr )
    , mResizeTimer( nullptr )
    , mPreviewEffect( nullptr )
    , mSnappingUtils( nullptr )
    , mScaleLocked( false )
    , mExpressionContextScope( tr( "Map Canvas" ) )
    , mZoomDragging( false )
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

  // create map canvas item which will show the map
  mMap = new QgsMapCanvasMap( this );

  // project handling
  connect( QgsProject::instance(), &QgsProject::readProject,
           this, &QgsMapCanvas::readProject );
  connect( QgsProject::instance(), &QgsProject::writeProject,
           this, &QgsMapCanvas::writeProject );

  mSettings.setFlag( QgsMapSettings::DrawEditingInfo );
  mSettings.setFlag( QgsMapSettings::UseRenderingOptimization );
  mSettings.setFlag( QgsMapSettings::RenderPartialOutput );

  //segmentation parameters
  QSettings settings;
  double segmentationTolerance = settings.value( QStringLiteral( "/qgis/segmentationTolerance" ), "0.01745" ).toDouble();
  QgsAbstractGeometry::SegmentationToleranceType toleranceType = QgsAbstractGeometry::SegmentationToleranceType( settings.value( QStringLiteral( "/qgis/segmentationToleranceType" ), 0 ).toInt() );
  mSettings.setSegmentationTolerance( segmentationTolerance );
  mSettings.setSegmentationToleranceType( toleranceType );

  mWheelZoomFactor = settings.value( QStringLiteral( "/qgis/zoom_factor" ), 2 ).toDouble();

  QSize s = viewport()->size();
  mSettings.setOutputSize( s );
  setSceneRect( 0, 0, s.width(), s.height() );
  mScene->setSceneRect( QRectF( 0, 0, s.width(), s.height() ) );

  moveCanvasContents( true );

  connect( &mMapUpdateTimer, SIGNAL( timeout() ), SLOT( mapUpdateTimeout() ) );
  mMapUpdateTimer.setInterval( 250 );

#ifdef Q_OS_WIN
  // Enable touch event on Windows.
  // Qt on Windows needs to be told it can take touch events or else it ignores them.
  grabGesture( Qt::PinchGesture );
  viewport()->setAttribute( Qt::WA_AcceptTouchEvents );
#endif

  mPreviewEffect = new QgsPreviewEffect( this );
  viewport()->setGraphicsEffect( mPreviewEffect );

  QPixmap zoomPixmap = QPixmap(( const char ** )( zoom_in ) );
  mZoomCursor = QCursor( zoomPixmap, 7, 7 );

  connect( &mAutoRefreshTimer, &QTimer::timeout, this, &QgsMapCanvas::autoRefreshTriggered );

  setInteractive( false );

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

  // delete canvas items prior to deleting the canvas
  // because they might try to update canvas when it's
  // already being destructed, ends with segfault
  QList<QGraphicsItem*> list = mScene->items();
  QList<QGraphicsItem*>::iterator it = list.begin();
  while ( it != list.end() )
  {
    QGraphicsItem* item = *it;
    delete item;
    ++it;
  }

  mScene->deleteLater();  // crashes in python tests on windows

  // mCanvasProperties auto-deleted via QScopedPointer
  // CanvasProperties struct has its own dtor for freeing resources

  if ( mJob )
  {
    mJob->cancel();
    Q_ASSERT( !mJob );
  }

  delete mCache;

  delete mLabelingResults;

} // dtor

void QgsMapCanvas::setMagnificationFactor( double factor )
{
  // do not go higher or lower than min max magnification ratio
  double magnifierMin = QgisGui::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = QgisGui::CANVAS_MAGNIFICATION_MAX;
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

QgsMapLayer* QgsMapCanvas::layer( int index )
{
  QList<QgsMapLayer*> layers = mapSettings().layers();
  if ( index >= 0 && index < ( int ) layers.size() )
    return layers[index];
  else
    return nullptr;
}


void QgsMapCanvas::setCurrentLayer( QgsMapLayer* layer )
{
  mCurrentLayer = layer;
  emit currentLayerChanged( layer );
}

double QgsMapCanvas::scale()
{
  return mapSettings().scale();
} // scale

bool QgsMapCanvas::isDrawing()
{
  return nullptr != mJob;
} // isDrawing

// return the current coordinate transform based on the extents and
// device size
const QgsMapToPixel * QgsMapCanvas::getCoordinateTransform()
{
  return &mapSettings().mapToPixel();
}

void QgsMapCanvas::setLayers( const QList<QgsMapLayer*>& layers )
{
  QList<QgsMapLayer*> oldLayers = mSettings.layers();

  // update only if needed
  if ( layers == oldLayers )
    return;

  Q_FOREACH ( QgsMapLayer* layer, oldLayers )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
    disconnect( layer, &QgsMapLayer::crsChanged, this, &QgsMapCanvas::layerCrsChange );
    disconnect( layer, &QgsMapLayer::autoRefreshIntervalChanged, this, &QgsMapCanvas::updateAutoRefreshTimer );
    if ( QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &QgsMapCanvas::selectionChangedSlot );
    }
  }

  mSettings.setLayers( layers );

  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapCanvas::layerRepaintRequested );
    connect( layer, &QgsMapLayer::crsChanged, this, &QgsMapCanvas::layerCrsChange );
    connect( layer, &QgsMapLayer::autoRefreshIntervalChanged, this, &QgsMapCanvas::updateAutoRefreshTimer );
    if ( QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      connect( vlayer, &QgsVectorLayer::selectionChanged, this, &QgsMapCanvas::selectionChangedSlot );
    }
  }
  updateDatumTransformEntries();

  QgsDebugMsg( "Layers have changed, refreshing" );
  emit layersChanged();

  updateAutoRefreshTimer();
  refresh();
}


const QgsMapSettings &QgsMapCanvas::mapSettings() const
{
  return mSettings;
}

void QgsMapCanvas::setCrsTransformEnabled( bool enabled )
{
  if ( mSettings.hasCrsTransformEnabled() == enabled )
    return;

  mSettings.setCrsTransformEnabled( enabled );

  updateDatumTransformEntries();

  refresh();

  emit hasCrsTransformEnabledChanged( enabled );
}

void QgsMapCanvas::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mSettings.destinationCrs() == crs )
    return;

  // try to reproject current extent to the new one
  QgsRectangle rect;
  if ( !mSettings.visibleExtent().isEmpty() )
  {
    QgsCoordinateTransform transform( mSettings.destinationCrs(), crs );
    try
    {
      rect = transform.transformBoundingBox( mSettings.visibleExtent() );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( QString( "Transform error caught: %1" ).arg( e.what() ) );
    }
  }

  if ( !mSettings.hasCrsTransformEnabled() )
  {
    mSettings.setMapUnits( crs.mapUnits() );
  }
  if ( !rect.isEmpty() )
  {
    setExtent( rect );
  }

  QgsDebugMsg( "refreshing after destination CRS changed" );
  refresh();

  mSettings.setDestinationCrs( crs );

  updateDatumTransformEntries();

  emit destinationCrsChanged();
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


QgsMapLayer* QgsMapCanvas::currentLayer()
{
  return mCurrentLayer;
}


void QgsMapCanvas::refresh()
{
  if ( !mSettings.hasValidSettings() )
  {
    QgsDebugMsg( "CANVAS refresh - invalid settings -> nothing to do" );
    return;
  }

  if ( !mRenderFlag || mFrozen )  // do we really need two flags controlling rendering?
  {
    QgsDebugMsg( "CANVAS render flag off" );
    return;
  }

  if ( mRefreshScheduled )
  {
    QgsDebugMsg( "CANVAS refresh already scheduled" );
    return;
  }

  mRefreshScheduled = true;

  QgsDebugMsg( "CANVAS refresh scheduling" );

  // schedule a refresh
  QTimer::singleShot( 1, this, SLOT( refreshMap() ) );
} // refresh

void QgsMapCanvas::refreshMap()
{
  Q_ASSERT( mRefreshScheduled );

  QgsDebugMsgLevel( "CANVAS refresh!", 3 );

  stopRendering(); // if any...

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
  << QgsExpressionContextUtils::mapSettingsScope( mSettings )
  << new QgsExpressionContextScope( mExpressionContextScope );

  mSettings.setExpressionContext( expressionContext );

  // create the renderer job
  Q_ASSERT( !mJob );
  mJobCanceled = false;
  if ( mUseParallelRendering )
    mJob = new QgsMapRendererParallelJob( mSettings );
  else
    mJob = new QgsMapRendererSequentialJob( mSettings );
  connect( mJob, &QgsMapRendererJob::finished, this, &QgsMapCanvas::rendererJobFinished );
  mJob->setCache( mCache );

  QStringList layersForGeometryCache;
  Q_FOREACH ( QgsMapLayer* layer, mSettings.layers() )
  {
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer ) )
    {
      if ( vl->isEditable() )
        layersForGeometryCache << vl->id();
    }
  }
  mJob->setRequestedGeometryCacheForLayers( layersForGeometryCache );

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


void QgsMapCanvas::rendererJobFinished()
{
  QgsDebugMsg( QString( "CANVAS finish! %1" ).arg( !mJobCanceled ) );

  mMapUpdateTimer.stop();

  // TODO: would be better to show the errors in message bar
  Q_FOREACH ( const QgsMapRendererJob::Error& error, mJob->errors() )
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

    QSettings settings;
    if ( settings.value( QStringLiteral( "/Map/logCanvasRefreshEvent" ), false ).toBool() )
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
      QString msg = QStringLiteral( "%1 :: %2 ms" ).arg( mUseParallelRendering ? "PARALLEL" : "SEQUENTIAL" ).arg( mJob->renderingTime() );
      p.drawText( r, msg, QTextOption( Qt::AlignCenter ) );
    }

    p.end();

    mMap->setContent( img, imageRect( img, mSettings ) );
  }

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;

  emit mapCanvasRefreshed();
}

QgsRectangle QgsMapCanvas::imageRect( const QImage& img, const QgsMapSettings& mapSettings )
{
  // This is a hack to pass QgsMapCanvasItem::setRect what it
  // expects (encoding of position and size of the item)
  const QgsMapToPixel& m2p = mapSettings.mapToPixel();
  QgsPoint topLeft = m2p.toMapPoint( 0, 0 );
  double res = m2p.mapUnitsPerPixel();
  QgsRectangle rect( topLeft.x(), topLeft.y(), topLeft.x() + img.width()*res, topLeft.y() - img.height()*res );
  return rect;
}

void QgsMapCanvas::mapUpdateTimeout()
{
  const QImage& img = mJob->renderedImage();
  mMap->setContent( img, imageRect( img, mSettings ) );
}

void QgsMapCanvas::stopRendering()
{
  if ( mJob )
  {
    QgsDebugMsg( "CANVAS stop rendering!" );
    mJobCanceled = true;
    mJob->cancel();
    Q_ASSERT( !mJob ); // no need to delete here: already deleted in finished()
  }
}

//the format defaults to "PNG" if not specified
void QgsMapCanvas::saveAsImage( const QString& fileName, QPixmap * theQPixmap, const QString& format )
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
  QGraphicsItem* item = nullptr;
  QListIterator<QGraphicsItem*> i( items() );
  i.toBack();
  while ( i.hasPrevious() )
  {
    item = i.previous();

    if ( !item || dynamic_cast< QgsMapCanvasAnnotationItem* >( item ) )
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

  //create a world file to go with the image...
  QgsRectangle myRect = mapSettings().visibleExtent();
  QString myHeader;
  // note: use 17 places of precision for all numbers output
  //Pixel XDim
  myHeader += qgsDoubleToString( mapUnitsPerPixel() ) + "\r\n";
  //Rotation on y axis - hard coded
  myHeader += QLatin1String( "0 \r\n" );
  //Rotation on x axis - hard coded
  myHeader += QLatin1String( "0 \r\n" );
  //Pixel YDim - almost always negative - see
  //http://en.wikipedia.org/wiki/World_file#cite_note-2
  myHeader += '-' + qgsDoubleToString( mapUnitsPerPixel() ) + "\r\n";
  //Origin X (center of top left cell)
  myHeader += qgsDoubleToString( myRect.xMinimum() + ( mapUnitsPerPixel() / 2 ) ) + "\r\n";
  //Origin Y (center of top left cell)
  myHeader += qgsDoubleToString( myRect.yMaximum() - ( mapUnitsPerPixel() / 2 ) ) + "\r\n";
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
  myStream << myHeader;
} // saveAsImage



QgsRectangle QgsMapCanvas::extent() const
{
  return mapSettings().visibleExtent();
} // extent

QgsRectangle QgsMapCanvas::fullExtent() const
{
  return mapSettings().fullExtent();
} // extent


void QgsMapCanvas::setExtent( const QgsRectangle& r, bool magnified )
{
  QgsRectangle current = extent();

  if (( r == current ) && magnified )
    return;

  if ( r.isEmpty() )
  {
    if ( !mSettings.hasValidSettings() )
    {
      // we can't even just move the map center
      QgsDebugMsg( "Empty extent - ignoring" );
      return;
    }

    // ### QGIS 3: do not allow empty extent - require users to call setCenter() explicitly
    QgsDebugMsg( "Empty extent - keeping old scale with new center!" );
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
  // notify canvas items of change
  updateCanvasItemPositions();

} // setExtent

void QgsMapCanvas::setCenter( const QgsPoint& center )
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

QgsPoint QgsMapCanvas::center() const
{
  QgsRectangle r = mapSettings().extent();
  return r.center();
}


double QgsMapCanvas::rotation() const
{
  return mapSettings().rotation();
} // rotation

void QgsMapCanvas::setRotation( double degrees )
{
  double current = rotation();

  if ( degrees == current )
    return;

  mSettings.setRotation( degrees );
  emit rotationChanged( degrees );
  emit extentsChanged(); // visible extent changes with rotation

  // notify canvas items of change (needed?)
  updateCanvasItemPositions();

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
    // notify canvas items of change
    updateCanvasItemPositions();
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
    // notify canvas items of change
    updateCanvasItemPositions();
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


bool QgsMapCanvas::hasCrsTransformEnabled()
{
  return mapSettings().hasCrsTransformEnabled();
}

void QgsMapCanvas::zoomToSelected( QgsVectorLayer* layer )
{
  if ( !layer )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( !layer || layer->selectedFeatureCount() == 0 )
    return;

  QgsRectangle rect = mapSettings().layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );
  zoomToFeatureExtent( rect );
} // zoomToSelected

void QgsMapCanvas::zoomToFeatureExtent( QgsRectangle& rect )
{
  // no selected features, only one selected point feature
  //or two point features with the same x- or y-coordinates
  if ( rect.isEmpty() )
  {
    // zoom in
    QgsPoint c = rect.center();
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

void QgsMapCanvas::zoomToFeatureIds( QgsVectorLayer* layer, const QgsFeatureIds& ids )
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
    emit messageEmitted( tr( "Zoom to feature id failed" ), errorMsg, QgsMessageBar::WARNING );
  }

}

void QgsMapCanvas::panToFeatureIds( QgsVectorLayer* layer, const QgsFeatureIds& ids )
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
    emit messageEmitted( tr( "Pan to feature id failed" ), errorMsg, QgsMessageBar::WARNING );
  }
}

bool QgsMapCanvas::boundingBoxOfFeatureIds( const QgsFeatureIds& ids, QgsVectorLayer* layer, QgsRectangle& bbox, QString& errorMsg ) const
{
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest().setFilterFids( ids ).setSubsetOfAttributes( QgsAttributeList() ) );
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
    else if ( geom.geometry()->isEmpty() )
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

void QgsMapCanvas::panToSelected( QgsVectorLayer* layer )
{
  if ( !layer )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( !layer || layer->selectedFeatureCount() == 0 )
    return;

  QgsRectangle rect = mapSettings().layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );
  if ( !rect.isNull() )
  {
    setCenter( rect.center() );
    refresh();
  }
  else
  {
    emit messageEmitted( tr( "Cannot pan to selected feature(s)" ), tr( "Geometry is NULL" ), QgsMessageBar::WARNING );
  }
} // panToSelected

void QgsMapCanvas::keyPressEvent( QKeyEvent * e )
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
    double dx = qAbs( currentExtent.width() / 4 );
    double dy = qAbs( currentExtent.height() / 4 );

    switch ( e->key() )
    {
      case Qt::Key_Left:
        QgsDebugMsg( "Pan left" );
        setCenter( center() - QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Right:
        QgsDebugMsg( "Pan right" );
        setCenter( center() + QgsVector( dx, 0 ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Up:
        QgsDebugMsg( "Pan up" );
        setCenter( center() + QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;

      case Qt::Key_Down:
        QgsDebugMsg( "Pan down" );
        setCenter( center() - QgsVector( 0, dy ).rotateBy( rotation() * M_PI / 180.0 ) );
        refresh();
        break;



      case Qt::Key_Space:
        QgsDebugMsg( "Pressing pan selector" );

        //mCanvasProperties->dragging = true;
        if ( ! e->isAutoRepeat() )
        {
          QApplication::setOverrideCursor( Qt::ClosedHandCursor );
          mCanvasProperties->panSelectorDown = true;
          mCanvasProperties->rubberStartPoint = mCanvasProperties->mouseLastXY;
        }
        break;

      case Qt::Key_PageUp:
        QgsDebugMsg( "Zoom in" );
        zoomIn();
        break;

      case Qt::Key_PageDown:
        QgsDebugMsg( "Zoom out" );
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

void QgsMapCanvas::keyReleaseEvent( QKeyEvent * e )
{
  QgsDebugMsg( "keyRelease event" );

  switch ( e->key() )
  {
    case Qt::Key_Space:
      if ( !e->isAutoRepeat() && mCanvasProperties->panSelectorDown )
      {
        QgsDebugMsg( "Releasing pan selector" );
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


void QgsMapCanvas::mouseDoubleClickEvent( QMouseEvent* e )
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
  const QSize& zoomRectSize = mZoomRect.size();
  const QSize& canvasSize = mSettings.outputSize();
  double sfx = ( double )zoomRectSize.width() / canvasSize.width();
  double sfy = ( double )zoomRectSize.height() / canvasSize.height();
  double sf = qMax( sfx, sfy );

  QgsPoint c = mSettings.mapToPixel().toMapCoordinates( mZoomRect.center() );

  zoomByFactor( sf, &c );
  refresh();
}

void QgsMapCanvas::mousePressEvent( QMouseEvent* e )
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


void QgsMapCanvas::mouseReleaseEvent( QMouseEvent* e )
{
  //use middle mouse button for panning, map tools won't receive any events in that case
  if ( e->button() == Qt::MidButton )
  {
    mCanvasProperties->panSelectorDown = false;
    panActionEnd( mCanvasProperties->mouseLastXY );
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
        QgsDebugMsg( "Right click in map tool zoom or pan, last tool is " +
                     QString( mLastNonZoomMapTool ? "not null." : "null." ) );

        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );

        // change to older non-zoom tool
        if ( mLastNonZoomMapTool
             && ( !( mLastNonZoomMapTool->flags() & QgsMapTool::EditTool )
                  || ( vlayer && vlayer->isEditable() ) ) )
        {
          QgsMapTool* t = mLastNonZoomMapTool;
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

void QgsMapCanvas::resizeEvent( QResizeEvent * e )
{
  QGraphicsView::resizeEvent( e );
  mResizeTimer->start( 500 );

  QSize lastSize = viewport()->size();

  mSettings.setOutputSize( lastSize );

  mScene->setSceneRect( QRectF( 0, 0, lastSize.width(), lastSize.height() ) );

  moveCanvasContents( true );

  // notify canvas items of change
  updateCanvasItemPositions();

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
  QList<QGraphicsItem*> list = mScene->items();
  QList<QGraphicsItem*>::iterator it = list.begin();
  while ( it != list.end() )
  {
    QgsMapCanvasItem* item = dynamic_cast<QgsMapCanvasItem *>( *it );

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

  double zoomFactor = mWheelZoomFactor;

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * qAbs( e->angleDelta().y() );

  if ( e->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  double signedWheelFactor = e->angleDelta().y() > 0 ? 1 / zoomFactor : zoomFactor;

  // zoom map to mouse cursor by scaling
  QgsPoint oldCenter = center();
  QgsPoint mousePos( getCoordinateTransform()->toMapPoint( e->x(), e->y() ) );
  QgsPoint newCenter( mousePos.x() + (( oldCenter.x() - mousePos.x() ) * signedWheelFactor ),
                      mousePos.y() + (( oldCenter.y() - mousePos.y() ) * signedWheelFactor ) );

  zoomByFactor( signedWheelFactor, &newCenter );
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
    QgsPoint center  = getCoordinateTransform()->toMapPoint( x, y );
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

void QgsMapCanvas::mouseMoveEvent( QMouseEvent * e )
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
  QPoint xy = e->pos();
  QgsPoint coord = getCoordinateTransform()->toMapCoordinates( xy );
  emit xyCoordinates( coord );
} // mouseMoveEvent



//! Sets the map tool currently being used on the canvas
void QgsMapCanvas::setMapTool( QgsMapTool* tool )
{
  if ( !tool )
    return;

  if ( mMapTool )
  {
    disconnect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    mMapTool->deactivate();
  }

  if (( tool->flags() & QgsMapTool::Transient )
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

  QgsMapTool* oldTool = mMapTool;

  // set new map tool and activate it
  mMapTool = tool;
  if ( mMapTool )
  {
    connect( mMapTool, &QObject::destroyed, this, &QgsMapCanvas::mapToolDestroyed );
    mMapTool->activate();
  }

  emit mapToolSet( mMapTool, oldTool );
} // setMapTool

void QgsMapCanvas::unsetMapTool( QgsMapTool* tool )
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

//! Write property of QColor bgColor.
void QgsMapCanvas::setCanvasColor( const QColor & color )
{
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
} // setBackgroundColor

QColor QgsMapCanvas::canvasColor() const
{
  return mScene->backgroundBrush().color();
}

void QgsMapCanvas::setSelectionColor( const QColor& color )
{
  mSettings.setSelectionColor( color );
}

int QgsMapCanvas::layerCount() const
{
  return mapSettings().layers().size();
} // layerCount


QList<QgsMapLayer*> QgsMapCanvas::layers() const
{
  return mapSettings().layers();
}


void QgsMapCanvas::layerStateChange()
{
  // called when a layer has changed visibility setting

  refresh();

} // layerStateChange

void QgsMapCanvas::layerCrsChange()
{
  // called when a layer's CRS has been changed
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  QString destAuthId = mSettings.destinationCrs().authid();
  getDatumTransformInfo( layer, layer->crs().authid(), destAuthId );

} // layerCrsChange


void QgsMapCanvas::freeze( bool frz )
{
  mFrozen = frz;
} // freeze

bool QgsMapCanvas::isFrozen()
{
  return mFrozen;
} // freeze


double QgsMapCanvas::mapUnitsPerPixel() const
{
  return mapSettings().mapUnitsPerPixel();
} // mapUnitsPerPixel


void QgsMapCanvas::setMapUnits( QgsUnitTypes::DistanceUnit u )
{
  if ( mSettings.mapUnits() == u )
    return;

  QgsDebugMsg( "Setting map units to " + QString::number( static_cast<int>( u ) ) );
  mSettings.setMapUnits( u );

  updateScale();

  refresh(); // this will force the scale bar to be updated

  emit mapUnitsChanged();
}


QgsUnitTypes::DistanceUnit QgsMapCanvas::mapUnits() const
{
  return mapSettings().mapUnits();
}

QMap<QString, QString> QgsMapCanvas::layerStyleOverrides() const
{
  return mSettings.layerStyleOverrides();
}

void QgsMapCanvas::setLayerStyleOverrides( const QMap<QString, QString>& overrides )
{
  if ( overrides == mSettings.layerStyleOverrides() )
    return;

  mSettings.setLayerStyleOverrides( overrides );
  emit layerStyleOverridesChanged();
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
void QgsMapCanvas::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
  QgsDebugMsg( "QgsMapCanvas connected to " + QString( signal ) );
} //connectNotify
#endif

void QgsMapCanvas::updateDatumTransformEntries()
{
  if ( !mSettings.hasCrsTransformEnabled() )
    return;

  QString destAuthId = mSettings.destinationCrs().authid();
  Q_FOREACH ( QgsMapLayer* layer, mSettings.layers() )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( vl && vl->geometryType() == QgsWkbTypes::NullGeometry )
      continue;

    // if there are more options, ask the user which datum transform to use
    if ( !mSettings.datumTransformStore().hasEntryForLayer( layer ) )
      getDatumTransformInfo( layer, layer->crs().authid(), destAuthId );
  }
}

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
  Q_FOREACH ( QgsMapLayer* layer, mSettings.layers() )
  {
    if ( layer->hasAutoRefreshEnabled() && layer->autoRefreshInterval() > 0 )
      minAutoRefreshInterval = minAutoRefreshInterval > 0 ? qMin( layer->autoRefreshInterval(), minAutoRefreshInterval ) : layer->autoRefreshInterval();
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

QgsMapTool* QgsMapCanvas::mapTool()
{
  return mMapTool;
}

void QgsMapCanvas::panActionEnd( QPoint releasePoint )
{
  // move map image and other items to standard position
  moveCanvasContents( true ); // true means reset

  // use start and end box points to calculate the extent
  QgsPoint start = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->rubberStartPoint );
  QgsPoint end = getCoordinateTransform()->toMapCoordinates( releasePoint );

  // modify the center
  double dx = end.x() - start.x();
  double dy = end.y() - start.y();
  QgsPoint c = center();
  c.set( c.x() - dx, c.y() - dy );
  setCenter( c );

  refresh();
}

void QgsMapCanvas::panAction( QMouseEvent * e )
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

QgsSnappingUtils* QgsMapCanvas::snappingUtils() const
{
  if ( !mSnappingUtils )
  {
    // associate a dummy instance, but better than null pointer
    QgsMapCanvas* c = const_cast<QgsMapCanvas*>( this );
    c->mSnappingUtils = new QgsMapCanvasSnappingUtils( c, c );
  }
  return mSnappingUtils;
}

void QgsMapCanvas::setSnappingUtils( QgsSnappingUtils* utils )
{
  mSnappingUtils = utils;
}

void QgsMapCanvas::readProject( const QDomDocument & doc )
{
  QDomNodeList nodes = doc.elementsByTagName( QStringLiteral( "mapcanvas" ) );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );

    QgsMapSettings tmpSettings;
    tmpSettings.readXml( node );
    setMapUnits( tmpSettings.mapUnits() );
    setCrsTransformEnabled( tmpSettings.hasCrsTransformEnabled() );
    setDestinationCrs( tmpSettings.destinationCrs() );
    setExtent( tmpSettings.extent() );
    setRotation( tmpSettings.rotation() );
    mSettings.datumTransformStore() = tmpSettings.datumTransformStore();
    enableMapTileRendering( tmpSettings.testFlag( QgsMapSettings::RenderMapTile ) );

    clearExtentHistory(); // clear the extent history on project load
  }
  else
  {
    QgsDebugMsg( "Couldn't read mapcanvas information from project" );
  }
}

void QgsMapCanvas::writeProject( QDomDocument & doc )
{
  // create node "mapcanvas" and call mMapRenderer->writeXml()

  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );
  if ( !nl.count() )
  {
    QgsDebugMsg( "Unable to find qgis element in project file" );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  QDomElement mapcanvasNode = doc.createElement( QStringLiteral( "mapcanvas" ) );
  qgisNode.appendChild( mapcanvasNode );

  mSettings.writeXml( mapcanvasNode, doc );
  // TODO: store only units, extent, projections, dest CRS
}

//! Ask user which datum transform to use
void QgsMapCanvas::getDatumTransformInfo( const QgsMapLayer* ml, const QString& srcAuthId, const QString& destAuthId )
{
  if ( !ml )
  {
    return;
  }

  //check if default datum transformation available
  QSettings s;
  QString settingsString = "/Projections/" + srcAuthId + "//" + destAuthId;
  QVariant defaultSrcTransform = s.value( settingsString + "_srcTransform" );
  QVariant defaultDestTransform = s.value( settingsString + "_destTransform" );
  if ( defaultSrcTransform.isValid() && defaultDestTransform.isValid() )
  {
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, defaultSrcTransform.toInt(), defaultDestTransform.toInt() );
    return;
  }

  QgsCoordinateReferenceSystem srcCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srcAuthId );
  QgsCoordinateReferenceSystem destCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( destAuthId );

  if ( !s.value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool() )
  {
    // just use the default transform
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, -1, -1 );
    return;
  }

  //get list of datum transforms
  QList< QList< int > > dt = QgsCoordinateTransform::datumTransformations( srcCRS, destCRS );
  if ( dt.size() < 2 )
  {
    return;
  }

  //if several possibilities:  present dialog
  QgsDatumTransformDialog d( ml->name(), dt );
  d.setDatumTransformInfo( srcCRS.authid(), destCRS.authid() );
  if ( d.exec() == QDialog::Accepted )
  {
    int srcTransform = -1;
    int destTransform = -1;
    QList<int> t = d.selectedDatumTransform();
    if ( !t.isEmpty() )
    {
      srcTransform = t.at( 0 );
    }
    if ( t.size() > 1 )
    {
      destTransform = t.at( 1 );
    }
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, srcTransform, destTransform );
    if ( d.rememberSelection() )
    {
      s.setValue( settingsString + "_srcTransform", srcTransform );
      s.setValue( settingsString + "_destTransform", destTransform );
    }
  }
  else
  {
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, -1, -1 );
  }
}

void QgsMapCanvas::zoomByFactor( double scaleFactor, const QgsPoint* center )
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
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  emit selectionChanged( layer );
  refresh();
}

void QgsMapCanvas::dragEnterEvent( QDragEnterEvent * e )
{
  // By default graphics view delegates the drag events to graphics items.
  // But we do not want that and by ignoring the drag enter we let the
  // parent (e.g. QgisApp) to handle drops of map layers etc.
  e->ignore();
}

void QgsMapCanvas::mapToolDestroyed()
{
  QgsDebugMsg( "maptool destroyed" );
  mMapTool = nullptr;
}

bool QgsMapCanvas::event( QEvent * e )
{
  if ( !QTouchDevice::devices().empty() )
  {
    if ( e->type() == QEvent::Gesture )
    {
      // call handler of current map tool
      if ( mMapTool )
      {
        return mMapTool->gestureEvent( static_cast<QGestureEvent*>( e ) );
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

void QgsMapCanvas::setSegmentationTolerance( double tolerance )
{
  mSettings.setSegmentationTolerance( tolerance );
}

void QgsMapCanvas::setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type )
{
  mSettings.setSegmentationToleranceType( type );
}
