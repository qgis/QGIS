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
#include "qgsapplication.h"
#include "qgscrscache.h"
#include "qgsdatumtransformdialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptopixel.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderer.h"
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
#include <math.h>


/**  @deprecated to be deleted, stuff from here should be moved elsewhere */
class QgsMapCanvas::CanvasProperties
{
  public:

    CanvasProperties() : mouseButtonDown( false ), panSelectorDown( false ) { }

    //!Flag to indicate status of mouse button
    bool mouseButtonDown;

    //! Last seen point of the mouse
    QPoint mouseLastXY;

    //! Beginning point of a rubber band
    QPoint rubberStartPoint;

    //! Flag to indicate the pan selector key is held down by user
    bool panSelectorDown;

};



QgsMapCanvasRendererSync::QgsMapCanvasRendererSync( QgsMapCanvas* canvas, QgsMapRenderer* renderer )
    : QObject( canvas )
    , mCanvas( canvas )
    , mRenderer( renderer )
    , mSyncingExtent( false )
{
  connect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( onExtentC2R() ) );
  connect( mRenderer, SIGNAL( extentsChanged() ), this, SLOT( onExtentR2C() ) );

  connect( mCanvas, SIGNAL( mapUnitsChanged() ), this, SLOT( onMapUnitsC2R() ) );
  connect( mRenderer, SIGNAL( mapUnitsChanged() ), this, SLOT( onMapUnitsR2C() ) );

  connect( mCanvas, SIGNAL( hasCrsTransformEnabledChanged( bool ) ), this, SLOT( onCrsTransformC2R() ) );
  connect( mRenderer, SIGNAL( hasCrsTransformEnabled( bool ) ), this, SLOT( onCrsTransformR2C() ) );

  connect( mCanvas, SIGNAL( destinationCrsChanged() ), this, SLOT( onDestCrsC2R() ) );
  connect( mRenderer, SIGNAL( destinationSrsChanged() ), this, SLOT( onDestCrsR2C() ) );

  connect( mCanvas, SIGNAL( layersChanged() ), this, SLOT( onLayersC2R() ) );
  // TODO: layers R2C ? (should not happen!)

}

void QgsMapCanvasRendererSync::onExtentC2R()
{
  // protection against possible bounce back
  if ( mSyncingExtent )
    return;

  mSyncingExtent = true;
  mRenderer->setExtent( mCanvas->mapSettings().extent() );
  mSyncingExtent = false;
}

void QgsMapCanvasRendererSync::onExtentR2C()
{
  // protection against possible bounce back
  if ( mSyncingExtent )
    return;

  mSyncingExtent = true;
  mCanvas->setExtent( mRenderer->extent() );
  mSyncingExtent = false;
}

void QgsMapCanvasRendererSync::onMapUnitsC2R()
{
  mRenderer->setMapUnits( mCanvas->mapSettings().mapUnits() );
}

void QgsMapCanvasRendererSync::onMapUnitsR2C()
{
  mCanvas->setMapUnits( mRenderer->mapUnits() );
}

void QgsMapCanvasRendererSync::onCrsTransformC2R()
{
  mRenderer->setProjectionsEnabled( mCanvas->mapSettings().hasCrsTransformEnabled() );
}

void QgsMapCanvasRendererSync::onCrsTransformR2C()
{
  mCanvas->setCrsTransformEnabled( mRenderer->hasCrsTransformEnabled() );
}

void QgsMapCanvasRendererSync::onDestCrsC2R()
{
  mRenderer->setDestinationCrs( mCanvas->mapSettings().destinationCrs(), true, false );
}

void QgsMapCanvasRendererSync::onDestCrsR2C()
{
  mCanvas->setDestinationCrs( mRenderer->destinationCrs() );
}

void QgsMapCanvasRendererSync::onLayersC2R()
{
  mRenderer->setLayerSet( mCanvas->mapSettings().layers() );
}



QgsMapCanvas::QgsMapCanvas( QWidget * parent, const char *name )
    : QGraphicsView( parent )
    , mCanvasProperties( new CanvasProperties )
    , mJob( 0 )
    , mJobCancelled( false )
    , mLabelingResults( 0 )
    , mUseParallelRendering( false )
    , mDrawRenderingStats( false )
    , mCache( 0 )
    , mPreviewEffect( 0 )
{
  setObjectName( name );
  mScene = new QGraphicsScene();
  setScene( mScene );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mLastExtentIndex = -1;
  mCurrentLayer = NULL;
  mMapOverview = NULL;
  mMapTool = NULL;
  mLastNonZoomMapTool = NULL;

  mFrozen = false;
  mRefreshScheduled = false;

  setWheelAction( WheelZoom );

  // by default, the canvas is rendered
  mRenderFlag = true;

  setMouseTracking( true );
  setFocusPolicy( Qt::StrongFocus );

  mMapRenderer = new QgsMapRenderer;

  mResizeTimer = new QTimer( this );
  mResizeTimer->setSingleShot( true );
  connect( mResizeTimer, SIGNAL( timeout() ), this, SLOT( refresh() ) );

  // create map canvas item which will show the map
  mMap = new QgsMapCanvasMap( this );
  mScene->addItem( mMap );

  // project handling
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );

  mSettings.setFlag( QgsMapSettings::DrawEditingInfo );
  mSettings.setFlag( QgsMapSettings::UseRenderingOptimization );

  // class that will sync most of the changes between canvas and (legacy) map renderer
  // it is parented to map canvas, will be deleted automatically
  new QgsMapCanvasRendererSync( this, mMapRenderer );

  QSize s = viewport()->size();
  mSettings.setOutputSize( s );
  mMapRenderer->setOutputSize( s, mSettings.outputDpi() );
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

  setInteractive( false );

  refresh();

} // QgsMapCanvas ctor


QgsMapCanvas::~QgsMapCanvas()
{
  if ( mMapTool )
  {
    mMapTool->deactivate();
    mMapTool = NULL;
  }
  mLastNonZoomMapTool = NULL;

  // delete canvas items prior to deleteing the canvas
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

  delete mMapRenderer;
  // mCanvasProperties auto-deleted via std::auto_ptr
  // CanvasProperties struct has its own dtor for freeing resources

  if ( mJob )
  {
    mJob->cancel();
    Q_ASSERT( mJob == 0 );
  }

  delete mCache;

  delete mLabelingResults;

} // dtor

void QgsMapCanvas::enableAntiAliasing( bool theFlag )
{
  mSettings.setFlag( QgsMapSettings::Antialiasing, theFlag );

  if ( mMapOverview )
    mMapOverview->enableAntiAliasing( theFlag );
} // anti aliasing

void QgsMapCanvas::useImageToRender( bool theFlag )
{
  Q_UNUSED( theFlag );
}

QgsMapCanvasMap* QgsMapCanvas::map()
{
  return mMap;
}

QgsMapRenderer* QgsMapCanvas::mapRenderer()
{
  return mMapRenderer;
}


QgsMapLayer* QgsMapCanvas::layer( int index )
{
  const QStringList& layers = mapSettings().layers();
  if ( index >= 0 && index < ( int ) layers.size() )
    return QgsMapLayerRegistry::instance()->mapLayer( layers[index] );
  else
    return NULL;
}


void QgsMapCanvas::setCurrentLayer( QgsMapLayer* layer )
{
  mCurrentLayer = layer;
}

double QgsMapCanvas::scale()
{
  return mapSettings().scale();
} // scale

void QgsMapCanvas::setDirty( bool dirty )
{
  if ( dirty )
    refresh();
}

bool QgsMapCanvas::isDirty() const
{
  return false;
}



bool QgsMapCanvas::isDrawing()
{
  return mJob != 0;
} // isDrawing


// return the current coordinate transform based on the extents and
// device size
const QgsMapToPixel * QgsMapCanvas::getCoordinateTransform()
{
  return &mapSettings().mapToPixel();
}

void QgsMapCanvas::setLayerSet( QList<QgsMapCanvasLayer> &layers )
{
  // create layer set
  QStringList layerSet, layerSetOverview;

  int i;
  for ( i = 0; i < layers.size(); i++ )
  {
    QgsMapCanvasLayer &lyr = layers[i];
    if ( !lyr.layer() )
    {
      continue;
    }

    if ( lyr.isVisible() )
    {
      layerSet.push_back( lyr.layer()->id() );
    }

    if ( lyr.isInOverview() )
    {
      layerSetOverview.push_back( lyr.layer()->id() );
    }
  }

  const QStringList& layerSetOld = mapSettings().layers();

  bool layerSetChanged = layerSetOld != layerSet;

  // update only if needed
  if ( layerSetChanged )
  {
    QgsDebugMsg( "Layers changed to: " + layerSet.join( ", " ) );

    for ( i = 0; i < layerCount(); i++ )
    {
      // Add check if vector layer when disconnecting from selectionChanged slot
      // Ticket #811 - racicot
      QgsMapLayer *currentLayer = layer( i );
      if ( !currentLayer )
        continue;
      disconnect( currentLayer, SIGNAL( repaintRequested() ), this, SLOT( refresh() ) );
      QgsVectorLayer *isVectLyr = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( isVectLyr )
      {
        disconnect( currentLayer, SIGNAL( selectionChanged() ), this, SLOT( selectionChangedSlot() ) );
      }
    }

    mSettings.setLayers( layerSet );

    for ( i = 0; i < layerCount(); i++ )
    {
      // Add check if vector layer when connecting to selectionChanged slot
      // Ticket #811 - racicot
      QgsMapLayer *currentLayer = layer( i );
      connect( currentLayer, SIGNAL( repaintRequested() ), this, SLOT( refresh() ) );
      QgsVectorLayer *isVectLyr = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( isVectLyr )
      {
        connect( currentLayer, SIGNAL( selectionChanged() ), this, SLOT( selectionChangedSlot() ) );
      }
    }

    updateDatumTransformEntries();

    QgsDebugMsg( "Layers have changed, refreshing" );
    emit layersChanged();

    refresh();
  }

  if ( mMapOverview )
  {
    const QStringList& layerSetOvOld = mMapOverview->layerSet();
    if ( layerSetOvOld != layerSetOverview )
    {
      mMapOverview->setLayerSet( layerSetOverview );
    }

    // refresh overview maplayers even if layer set is the same
    // because full extent might have changed
    updateOverview();
  }
} // setLayerSet

void QgsMapCanvas::enableOverviewMode( QgsMapOverviewCanvas* overview )
{
  if ( mMapOverview )
  {
    // disconnect old map overview if exists
    disconnect( this,       SIGNAL( hasCrsTransformEnabledChanged( bool ) ),
                mMapOverview, SLOT( hasCrsTransformEnabled( bool ) ) );
    disconnect( this,       SIGNAL( destinationCrsChanged() ),
                mMapOverview, SLOT( destinationSrsChanged() ) );

    // map overview is not owned by map canvas so don't delete it...
  }

  mMapOverview = overview;

  if ( overview )
  {
    // connect to the map render to copy its projection settings
    connect( this,       SIGNAL( hasCrsTransformEnabledChanged( bool ) ),
             overview,     SLOT( hasCrsTransformEnabled( bool ) ) );
    connect( this,       SIGNAL( destinationCrsChanged() ),
             overview,     SLOT( destinationSrsChanged() ) );
  }
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

  if ( mSettings.hasCrsTransformEnabled() )
  {
    // try to reproject current extent to the new one
    QgsRectangle rect;
    if ( !mSettings.visibleExtent().isEmpty() )
    {
      QgsCoordinateTransform transform( mSettings.destinationCrs(), crs );
      rect = transform.transformBoundingBox( mSettings.visibleExtent() );
    }
    if ( !rect.isEmpty() )
    {
      setExtent( rect );
    }

    QgsDebugMsg( "refreshing after destination CRS changed" );
    refresh();
  }

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

  if ( enabled )
  {
    mCache = new QgsMapRendererCache;
  }
  else
  {
    delete mCache;
    mCache = 0;
  }
}

bool QgsMapCanvas::isCachingEnabled() const
{
  return mCache != 0;
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

void QgsMapCanvas::setMapUpdateInterval( int timeMiliseconds )
{
  mMapUpdateTimer.setInterval( timeMiliseconds );
}

int QgsMapCanvas::mapUpdateInterval() const
{
  return mMapUpdateTimer.interval();
}


void QgsMapCanvas::updateOverview()
{
  // redraw overview
  if ( mMapOverview )
  {
    mMapOverview->refresh();
  }
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

  QgsDebugMsg( "CANVAS refresh!" );

  stopRendering(); // if any...

  // from now on we can accept refresh requests again
  mRefreshScheduled = false;

  //update $map variable to canvas
  QgsExpression::setSpecialColumn( "$map", tr( "canvas" ) );

  // create the renderer job
  Q_ASSERT( mJob == 0 );
  mJobCancelled = false;
  if ( mUseParallelRendering )
    mJob = new QgsMapRendererParallelJob( mSettings );
  else
    mJob = new QgsMapRendererSequentialJob( mSettings );
  connect( mJob, SIGNAL( finished() ), SLOT( rendererJobFinished() ) );
  mJob->setCache( mCache );

  QStringList layersForGeometryCache;
  foreach ( QString id, mSettings.layers() )
  {
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( id ) ) )
    {
      if ( vl->isEditable() )
        layersForGeometryCache << id;
    }
  }
  mJob->setRequestedGeometryCacheForLayers( layersForGeometryCache );

  mJob->start();

  mMapUpdateTimer.start();

  emit renderStarting();
}


void QgsMapCanvas::rendererJobFinished()
{
  QgsDebugMsg( QString( "CANVAS finish! %1" ).arg( !mJobCancelled ) );

  mMapUpdateTimer.stop();

  // TODO: would be better to show the errors in message bar
  foreach ( const QgsMapRendererJob::Error& error, mJob->errors() )
  {
    QgsMessageLog::logMessage( error.layerID + " :: " + error.message, tr( "Rendering" ) );
  }

  if ( !mJobCancelled )
  {
    // take labeling results before emitting renderComplete, so labeling map tools
    // connected to signal work with correct results
    delete mLabelingResults;
    mLabelingResults = mJob->takeLabelingResults();

    QImage img = mJob->renderedImage();

    // emit renderComplete to get our decorations drawn
    QPainter p( &img );
    emit renderComplete( &p );

    QSettings settings;
    if ( settings.value( "/Map/logCanvasRefreshEvent", false ).toBool() )
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
      QString msg = QString( "%1 :: %2 ms" ).arg( mUseParallelRendering ? "PARALLEL" : "SEQUENTIAL" ).arg( mJob->renderingTime() );
      p.drawText( r, msg, QTextOption( Qt::AlignCenter ) );
    }

    p.end();

    mMap->setContent( img, mSettings.visibleExtent() );
  }

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = 0;

  emit mapCanvasRefreshed();
}

void QgsMapCanvas::mapUpdateTimeout()
{
  mMap->setContent( mJob->renderedImage(), mSettings.visibleExtent() );
}


void QgsMapCanvas::stopRendering()
{
  if ( mJob )
  {
    QgsDebugMsg( "CANVAS stop rendering!" );
    mJobCancelled = true;
    mJob->cancel();
    Q_ASSERT( mJob == 0 ); // no need to delete here: already deleted in finished()
  }
}

void QgsMapCanvas::updateMap()
{
}

//the format defaults to "PNG" if not specified
void QgsMapCanvas::saveAsImage( QString theFileName, QPixmap * theQPixmap, QString theFormat )
{
  //
  //check if the optional QPaintDevice was supplied
  //
  if ( theQPixmap != NULL )
  {
    // render
    QPainter painter;
    painter.begin( theQPixmap );
    QgsMapRendererCustomPainterJob job( mSettings, &painter );
    job.start();
    job.waitForFinished();
    emit renderComplete( &painter );
    painter.end();

    theQPixmap->save( theFileName, theFormat.toLocal8Bit().data() );
  }
  else //use the map view
  {
    mMap->contentImage().save( theFileName, theFormat.toLocal8Bit().data() );
  }
  //create a world file to go with the image...
  QgsRectangle myRect = mapSettings().visibleExtent();
  QString myHeader;
  // note: use 17 places of precision for all numbers output
  //Pixel XDim
  myHeader += qgsDoubleToString( mapUnitsPerPixel() ) + "\r\n";
  //Rotation on y axis - hard coded
  myHeader += "0 \r\n";
  //Rotation on x axis - hard coded
  myHeader += "0 \r\n";
  //Pixel YDim - almost always negative - see
  //http://en.wikipedia.org/wiki/World_file#cite_note-2
  myHeader += "-" + qgsDoubleToString( mapUnitsPerPixel() ) + "\r\n";
  //Origin X (center of top left cell)
  myHeader += qgsDoubleToString( myRect.xMinimum() + ( mapUnitsPerPixel() / 2 ) ) + "\r\n";
  //Origin Y (center of top left cell)
  myHeader += qgsDoubleToString( myRect.yMaximum() - ( mapUnitsPerPixel() / 2 ) ) + "\r\n";
  QFileInfo myInfo  = QFileInfo( theFileName );
  // allow dotted names
  QString myWorldFileName = myInfo.absolutePath() + "/" + myInfo.completeBaseName() + "." + theFormat + "w";
  QFile myWorldFile( myWorldFileName );
  if ( !myWorldFile.open( QIODevice::WriteOnly ) ) //don't use QIODevice::Text
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


void QgsMapCanvas::setExtent( QgsRectangle const & r )
{
  QgsRectangle current = extent();

  if ( r == current )
    return;

  if ( r.isEmpty() )
  {
    QgsDebugMsg( "Empty extent - keeping old extent with new center!" );
    QgsRectangle e( QgsPoint( r.center().x() - current.width() / 2.0, r.center().y() - current.height() / 2.0 ),
                    QgsPoint( r.center().x() + current.width() / 2.0, r.center().y() + current.height() / 2.0 ) );
    mSettings.setExtent( e );
  }
  else
  {
    mSettings.setExtent( r );
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

  mLastExtent.append( extent() ) ;

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


void QgsMapCanvas::updateScale()
{
  emit scaleChanged( mapSettings().scale() );
}


void QgsMapCanvas::clear()
{
  refresh();
} // clear



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
  if ( layer == NULL )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( layer == NULL )
  {
    return;
  }

  if ( layer->selectedFeatureCount() == 0 )
  {
    return;
  }

  QgsRectangle rect = mapSettings().layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );

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
} // zoomToSelected

void QgsMapCanvas::panToSelected( QgsVectorLayer* layer )
{
  if ( layer == NULL )
  {
    // use current layer by default
    layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
  }

  if ( layer == NULL )
  {
    return;
  }

  if ( layer->selectedFeatureCount() == 0 )
  {
    return;
  }

  QgsRectangle rect = mapSettings().layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );
  setExtent( QgsRectangle( rect.center(), rect.center() ) );
  refresh();
} // panToSelected

void QgsMapCanvas::keyPressEvent( QKeyEvent * e )
{
  if ( mCanvasProperties->mouseButtonDown || mCanvasProperties->panSelectorDown )
  {
    emit keyPressed( e );
    return;
  }

  QPainter paint;
  QPen     pen( Qt::gray );
  QgsPoint ll, ur;

  if ( ! mCanvasProperties->mouseButtonDown )
  {
    // Don't want to interfer with mouse events

    QgsRectangle currentExtent = mapSettings().visibleExtent();
    double dx = qAbs(( currentExtent.xMaximum() - currentExtent.xMinimum() ) / 4 );
    double dy = qAbs(( currentExtent.yMaximum() - currentExtent.yMinimum() ) / 4 );

    switch ( e->key() )
    {
      case Qt::Key_Left:
        QgsDebugMsg( "Pan left" );

        currentExtent.setXMinimum( currentExtent.xMinimum() - dx );
        currentExtent.setXMaximum( currentExtent.xMaximum() - dx );
        setExtent( currentExtent );
        refresh();
        break;

      case Qt::Key_Right:
        QgsDebugMsg( "Pan right" );

        currentExtent.setXMinimum( currentExtent.xMinimum() + dx );
        currentExtent.setXMaximum( currentExtent.xMaximum() + dx );
        setExtent( currentExtent );
        refresh();
        break;

      case Qt::Key_Up:
        QgsDebugMsg( "Pan up" );

        currentExtent.setYMaximum( currentExtent.yMaximum() + dy );
        currentExtent.setYMinimum( currentExtent.yMinimum() + dy );
        setExtent( currentExtent );
        refresh();
        break;

      case Qt::Key_Down:
        QgsDebugMsg( "Pan down" );

        currentExtent.setYMaximum( currentExtent.yMaximum() - dy );
        currentExtent.setYMinimum( currentExtent.yMinimum() - dy );
        setExtent( currentExtent );
        refresh();
        break;



      case Qt::Key_Space:
        QgsDebugMsg( "Pressing pan selector" );

        //mCanvasProperties->dragging = true;
        if ( ! e->isAutoRepeat() )
        {
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


void QgsMapCanvas::mouseDoubleClickEvent( QMouseEvent * e )
{
  // call handler of current map tool
  if ( mMapTool )
    mMapTool->canvasDoubleClickEvent( e );
} // mouseDoubleClickEvent


void QgsMapCanvas::mousePressEvent( QMouseEvent * e )
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
      mMapTool->canvasPressEvent( e );
  }

  if ( mCanvasProperties->panSelectorDown )
  {
    return;
  }

  mCanvasProperties->mouseButtonDown = true;
  mCanvasProperties->rubberStartPoint = e->pos();

} // mousePressEvent


void QgsMapCanvas::mouseReleaseEvent( QMouseEvent * e )
{
  //use middle mouse button for panning, map tools won't receive any events in that case
  if ( e->button() == Qt::MidButton )
  {
    mCanvasProperties->panSelectorDown = false;
    panActionEnd( mCanvasProperties->mouseLastXY );
  }
  else
  {
    // call handler of current map tool
    if ( mMapTool )
    {
      // right button was pressed in zoom tool? return to previous non zoom tool
      if ( e->button() == Qt::RightButton && mMapTool->isTransient() )
      {
        QgsDebugMsg( "Right click in map tool zoom or pan, last tool is " +
                     QString( mLastNonZoomMapTool ? "not null." : "null." ) );

        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );

        // change to older non-zoom tool
        if ( mLastNonZoomMapTool
             && ( !mLastNonZoomMapTool->isEditTool() || ( vlayer && vlayer->isEditable() ) ) )
        {
          QgsMapTool* t = mLastNonZoomMapTool;
          mLastNonZoomMapTool = NULL;
          setMapTool( t );
        }
        return;
      }
      mMapTool->canvasReleaseEvent( e );
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
  mMapRenderer->setOutputSize( lastSize, mSettings.outputDpi() );

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
  }

  if ( QgsApplication::keyboardModifiers() )
  {
    // leave the wheel for map tools if any modifier pressed
    return;
  }

  switch ( mWheelAction )
  {
    case WheelZoom:
      // zoom without changing extent
      if ( e->delta() > 0 )
        zoomIn();
      else
        zoomOut();
      break;

    case WheelZoomAndRecenter:
      // zoom and don't change extent
      zoomWithCenter( e->x(), e->y(), e->delta() > 0 );
      break;

    case WheelZoomToMouseCursor:
    {
      // zoom map to mouse cursor
      double scaleFactor = e->delta() > 0 ? 1 / mWheelZoomFactor : mWheelZoomFactor;

      QgsPoint oldCenter( mapSettings().visibleExtent().center() );
      QgsPoint mousePos( getCoordinateTransform()->toMapPoint( e->x(), e->y() ) );
      QgsPoint newCenter( mousePos.x() + (( oldCenter.x() - mousePos.x() ) * scaleFactor ),
                          mousePos.y() + (( oldCenter.y() - mousePos.y() ) * scaleFactor ) );

      // same as zoomWithCenter (no coordinate transformations are needed)
      QgsRectangle extent = mapSettings().visibleExtent();
      extent.scale( scaleFactor, &newCenter );
      setExtent( extent );
      refresh();
      break;
    }

    case WheelNothing:
      // well, nothing!
      break;
  }
}

void QgsMapCanvas::setWheelAction( WheelAction action, double factor )
{
  mWheelAction = action;
  mWheelZoomFactor = factor;
}

void QgsMapCanvas::zoomIn()
{
  zoomByFactor( 1 / mWheelZoomFactor );
}

void QgsMapCanvas::zoomOut()
{
  zoomByFactor( mWheelZoomFactor );
}

void QgsMapCanvas::zoomScale( double newScale )
{
  zoomByFactor( newScale / scale() );
}

void QgsMapCanvas::zoomWithCenter( int x, int y, bool zoomIn )
{
  double scaleFactor = ( zoomIn ? 1 / mWheelZoomFactor : mWheelZoomFactor );

  // transform the mouse pos to map coordinates
  QgsPoint center  = getCoordinateTransform()->toMapPoint( x, y );
  QgsRectangle r = mapSettings().visibleExtent();
  r.scale( scaleFactor, &center );
  setExtent( r );
  refresh();
}

void QgsMapCanvas::mouseMoveEvent( QMouseEvent * e )
{
  mCanvasProperties->mouseLastXY = e->pos();

  if ( mCanvasProperties->panSelectorDown )
  {
    panAction( e );
  }
  else
  {
    // call handler of current map tool
    if ( mMapTool )
      mMapTool->canvasMoveEvent( e );
  }

  // show x y on status bar
  QPoint xy = e->pos();
  QgsPoint coord = getCoordinateTransform()->toMapCoordinates( xy );
  emit xyCoordinates( coord );
} // mouseMoveEvent



/** Sets the map tool currently being used on the canvas */
void QgsMapCanvas::setMapTool( QgsMapTool* tool )
{
  if ( !tool )
    return;

  if ( mMapTool )
  {
    disconnect( mMapTool, SIGNAL( destroyed() ), this, SLOT( mapToolDestroyed() ) );
    mMapTool->deactivate();
  }

  if ( tool->isTransient() && mMapTool && !mMapTool->isTransient() )
  {
    // if zoom or pan tool will be active, save old tool
    // to bring it back on right click
    // (but only if it wasn't also zoom or pan tool)
    mLastNonZoomMapTool = mMapTool;
  }
  else
  {
    mLastNonZoomMapTool = NULL;
  }

  QgsMapTool* oldTool = mMapTool;

  // set new map tool and activate it
  mMapTool = tool;
  if ( mMapTool )
  {
    connect( mMapTool, SIGNAL( destroyed() ), this, SLOT( mapToolDestroyed() ) );
    mMapTool->activate();
  }

  emit mapToolSet( mMapTool );
  emit mapToolSet( mMapTool, oldTool );
} // setMapTool

void QgsMapCanvas::unsetMapTool( QgsMapTool* tool )
{
  if ( mMapTool && mMapTool == tool )
  {
    mMapTool->deactivate();
    mMapTool = NULL;
    emit mapToolSet( NULL );
    emit mapToolSet( NULL, mMapTool );
    setCursor( Qt::ArrowCursor );
  }

  if ( mLastNonZoomMapTool && mLastNonZoomMapTool == tool )
  {
    mLastNonZoomMapTool = NULL;
  }
}

/** Write property of QColor bgColor. */
void QgsMapCanvas::setCanvasColor( const QColor & theColor )
{
  // background of map's pixmap
  mSettings.setBackgroundColor( theColor );

  // background of the QGraphicsView
  QBrush bgBrush( theColor );
  setBackgroundBrush( bgBrush );
#if 0
  QPalette palette;
  palette.setColor( backgroundRole(), theColor );
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
  QList<QgsMapLayer*> lst;
  foreach ( QString layerID, mapSettings().layers() )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
    if ( layer )
      lst.append( layer );
  }
  return lst;
}


void QgsMapCanvas::layerStateChange()
{
  // called when a layer has changed visibility setting

  refresh();

} // layerStateChange



void QgsMapCanvas::freeze( bool frz )
{
  mFrozen = frz;
} // freeze

bool QgsMapCanvas::isFrozen()
{
  return mFrozen;
} // freeze


QPaintDevice &QgsMapCanvas::canvasPaintDevice()
{
  Q_NOWARN_DEPRECATED_PUSH
  return mMap->paintDevice();
  Q_NOWARN_DEPRECATED_POP
}

double QgsMapCanvas::mapUnitsPerPixel() const
{
  return mapSettings().mapUnitsPerPixel();
} // mapUnitsPerPixel


void QgsMapCanvas::setMapUnits( QGis::UnitType u )
{
  if ( mSettings.mapUnits() == u )
    return;

  QgsDebugMsg( "Setting map units to " + QString::number( static_cast<int>( u ) ) );
  mSettings.setMapUnits( u );

  updateScale();

  refresh(); // this will force the scale bar to be updated

  emit mapUnitsChanged();
}


QGis::UnitType QgsMapCanvas::mapUnits() const
{
  return mapSettings().mapUnits();
}


void QgsMapCanvas::setRenderFlag( bool theFlag )
{
  mRenderFlag = theFlag;

  if ( mRenderFlag )
  {
    refresh();
  }
  else
    stopRendering();
}

void QgsMapCanvas::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
  QgsDebugMsg( "QgsMapCanvas connected to " + QString( signal ) );
} //connectNotify


void QgsMapCanvas::updateDatumTransformEntries()
{
  if ( !mSettings.hasCrsTransformEnabled() )
    return;

  QString destAuthId = mSettings.destinationCrs().authid();
  foreach ( QString layerID, mSettings.layers() )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerID );
    if ( !layer )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( vl && vl->geometryType() == QGis::NoGeometry )
      continue;

    // if there are more options, ask the user which datum transform to use
    if ( !mSettings.datumTransformStore().hasEntryForLayer( layer ) )
      getDatumTransformInfo( layer, layer->crs().authid(), destAuthId );
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

  double dx = qAbs( end.x() - start.x() );
  double dy = qAbs( end.y() - start.y() );

  // modify the extent
  QgsRectangle r = mapSettings().visibleExtent();

  if ( end.x() < start.x() )
  {
    r.setXMinimum( r.xMinimum() + dx );
    r.setXMaximum( r.xMaximum() + dx );
  }
  else
  {
    r.setXMinimum( r.xMinimum() - dx );
    r.setXMaximum( r.xMaximum() - dx );
  }

  if ( end.y() < start.y() )
  {
    r.setYMaximum( r.yMaximum() + dy );
    r.setYMinimum( r.yMinimum() + dy );

  }
  else
  {
    r.setYMaximum( r.yMaximum() - dy );
    r.setYMinimum( r.yMinimum() - dy );

  }

  setExtent( r );

  r = mapSettings().visibleExtent();

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

void QgsMapCanvas::showError( QgsMapLayer * mapLayer )
{
  Q_UNUSED( mapLayer );
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

void QgsMapCanvas::readProject( const QDomDocument & doc )
{
  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );

    QgsMapSettings tmpSettings;
    tmpSettings.readXML( node );
    setMapUnits( tmpSettings.mapUnits() );
    setCrsTransformEnabled( tmpSettings.hasCrsTransformEnabled() );
    setDestinationCrs( tmpSettings.destinationCrs() );
    setExtent( tmpSettings.extent() );
    mSettings.datumTransformStore() = tmpSettings.datumTransformStore();

    clearExtentHistory(); // clear the extent history on project load
  }
  else
  {
    QgsDebugMsg( "Couldn't read mapcanvas information from project" );
  }
}

void QgsMapCanvas::writeProject( QDomDocument & doc )
{
  // create node "mapcanvas" and call mMapRenderer->writeXML()

  QDomNodeList nl = doc.elementsByTagName( "qgis" );
  if ( !nl.count() )
  {
    QgsDebugMsg( "Unable to find qgis element in project file" );
    return;
  }
  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  QDomElement mapcanvasNode = doc.createElement( "mapcanvas" );
  qgisNode.appendChild( mapcanvasNode );

  mSettings.writeXML( mapcanvasNode, doc );
  // TODO: store only units, extent, projections, dest CRS
}

/**Ask user which datum transform to use*/
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
    mMapRenderer->addLayerCoordinateTransform( ml->id(), srcAuthId, destAuthId, defaultSrcTransform.toInt(), defaultDestTransform.toInt() );
    return;
  }

  const QgsCoordinateReferenceSystem& srcCRS = QgsCRSCache::instance()->crsByAuthId( srcAuthId );
  const QgsCoordinateReferenceSystem& destCRS = QgsCRSCache::instance()->crsByAuthId( destAuthId );

  if ( !s.value( "/Projections/showDatumTransformDialog", false ).toBool() )
  {
    // just use the default transform
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, -1, -1 );
    mMapRenderer->addLayerCoordinateTransform( ml->id(), srcAuthId, destAuthId, -1, -1 );
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
    if ( t.size() > 0 )
    {
      srcTransform = t.at( 0 );
    }
    if ( t.size() > 1 )
    {
      destTransform = t.at( 1 );
    }
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, srcTransform, destTransform );
    mMapRenderer->addLayerCoordinateTransform( ml->id(), srcAuthId, destAuthId, srcTransform, destTransform );
    if ( d.rememberSelection() )
    {
      s.setValue( settingsString + "_srcTransform", srcTransform );
      s.setValue( settingsString + "_destTransform", destTransform );
    }
  }
  else
  {
    mSettings.datumTransformStore().addEntry( ml->id(), srcAuthId, destAuthId, -1, -1 );
    mMapRenderer->addLayerCoordinateTransform( ml->id(), srcAuthId, destAuthId, -1, -1 );
  }
}

void QgsMapCanvas::zoomByFactor( double scaleFactor , const QgsPoint* center )
{
  QgsRectangle r = mapSettings().visibleExtent();
  r.scale( scaleFactor, center );
  setExtent( r );
  refresh();
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
  mMapTool = 0;
}

#ifdef HAVE_TOUCH
bool QgsMapCanvas::event( QEvent * e )
{
  bool done = false;
  if ( e->type() == QEvent::Gesture )
  {
    // call handler of current map tool
    if ( mMapTool )
    {
      done = mMapTool->gestureEvent( static_cast<QGestureEvent*>( e ) );
    }
  }
  else
  {
    // pass other events to base class
    done = QGraphicsView::event( e );
  }
  return done;
}
#endif
