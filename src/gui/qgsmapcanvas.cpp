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
#include <QTextStream>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QWheelEvent>

#include "qgis.h"
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
#include "qgsmessageviewer.h"
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



QgsMapCanvas::QgsMapCanvas( QWidget * parent, const char *name )
    : QGraphicsView( parent )
    , mCanvasProperties( new CanvasProperties )
    , mNewSize( QSize() )
    , mPainting( false )
    , mAntiAliasing( false )
{
  setObjectName( name );

  //disable the update that leads to the resize crash
  if ( viewport() )
  {
#ifndef ANDROID
    viewport()->setAttribute( Qt::WA_PaintOnScreen, true );
#endif //ANDROID
  }
#endif

  mScene = new QGraphicsScene();
  setScene( mScene );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mLastExtentIndex = -1;
  mCurrentLayer = NULL;
  mMapOverview = NULL;
  mMapTool = NULL;
  mLastNonZoomMapTool = NULL;

  mDrawing = false;
  mFrozen = false;
  mDirty = true;

  setWheelAction( WheelZoom );

  // by default, the canvas is rendered
  mRenderFlag = true;

  setMouseTracking( true );
  setFocusPolicy( Qt::StrongFocus );

  mMapRenderer = new QgsMapRenderer;

  // create map canvas item which will show the map
  mMap = new QgsMapCanvasMap( this );
  mScene->addItem( mMap );
  mScene->update(); // porting??

  moveCanvasContents( true );

  connect( mMapRenderer, SIGNAL( drawError( QgsMapLayer* ) ), this, SLOT( showError( QgsMapLayer* ) ) );
  connect( mMapRenderer, SIGNAL( hasCrsTransformEnabled( bool ) ), this, SLOT( crsTransformEnabled( bool ) ) );

  crsTransformEnabled( hasCrsTransformEnabled() );

  // project handling
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );
  mMap->resize( size() );
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
    it++;
  }

  mScene->deleteLater();  // crashes in python tests on windows

  delete mMapRenderer;
  // mCanvasProperties auto-deleted via std::auto_ptr
  // CanvasProperties struct has its own dtor for freeing resources

} // dtor

void QgsMapCanvas::enableAntiAliasing( bool theFlag )
{
  mAntiAliasing = theFlag;
  mMap->enableAntiAliasing( theFlag );
  if ( mMapOverview )
    mMapOverview->enableAntiAliasing( theFlag );
} // anti aliasing

void QgsMapCanvas::useImageToRender( bool theFlag )
{
  mMap->useImageToRender( theFlag );
  refresh(); // redraw the map on change - prevents black map view
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
  QStringList& layers = mMapRenderer->layerSet();
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
  return mMapRenderer->scale();
} // scale

void QgsMapCanvas::setDirty( bool dirty )
{
  mDirty = dirty;
}

bool QgsMapCanvas::isDirty() const
{
  return mDirty;
}



bool QgsMapCanvas::isDrawing()
{
  return mDrawing;
} // isDrawing


// return the current coordinate transform based on the extents and
// device size
const QgsMapToPixel * QgsMapCanvas::getCoordinateTransform()
{
  return mMapRenderer->coordinateTransform();
}

void QgsMapCanvas::setLayerSet( QList<QgsMapCanvasLayer> &layers )
{
  if ( mDrawing )
  {
    QgsDebugMsg( "NOT updating layer set while drawing" );
    return;
  }

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

  QStringList& layerSetOld = mMapRenderer->layerSet();

  bool layerSetChanged = layerSetOld != layerSet;

  // update only if needed
  if ( !layerSetChanged )
    return;

  QgsDebugMsg( "Layer changed to: " + layerSet.join( ", " ) );

  for ( i = 0; i < layerCount(); i++ )
  {
    // Add check if vector layer when disconnecting from selectionChanged slot
    // Ticket #811 - racicot
    QgsMapLayer *currentLayer = layer( i );
    disconnect( currentLayer, SIGNAL( repaintRequested() ), this, SLOT( refresh() ) );
    disconnect( currentLayer, SIGNAL( screenUpdateRequested() ), this, SLOT( updateMap() ) );
    QgsVectorLayer *isVectLyr = qobject_cast<QgsVectorLayer *>( currentLayer );
    if ( isVectLyr )
    {
      disconnect( currentLayer, SIGNAL( selectionChanged() ), this, SLOT( selectionChangedSlot() ) );
    }
  }

  mMapRenderer->setLayerSet( layerSet );

  for ( i = 0; i < layerCount(); i++ )
  {
    // Add check if vector layer when connecting to selectionChanged slot
    // Ticket #811 - racicot
    QgsMapLayer *currentLayer = layer( i );
    connect( currentLayer, SIGNAL( repaintRequested() ), this, SLOT( refresh() ) );
    connect( currentLayer, SIGNAL( screenUpdateRequested() ), this, SLOT( updateMap() ) );
    QgsVectorLayer *isVectLyr = qobject_cast<QgsVectorLayer *>( currentLayer );
    if ( isVectLyr )
    {
      connect( currentLayer, SIGNAL( selectionChanged() ), this, SLOT( selectionChangedSlot() ) );
    }
  }

  if ( mMapOverview )
  {
    mMapOverview->updateFullExtent( fullExtent() );

    QStringList& layerSetOvOld = mMapOverview->layerSet();
    if ( layerSetOvOld != layerSetOverview )
    {
      mMapOverview->setLayerSet( layerSetOverview );
    }

    // refresh overview maplayers even if layer set is the same
    // because full extent might have changed
    updateOverview();
  }

  QgsDebugMsg( "Layers have changed, refreshing" );
  emit layersChanged();

  refresh();

} // setLayerSet

void QgsMapCanvas::enableOverviewMode( QgsMapOverviewCanvas* overview )
{
  if ( mMapOverview )
  {
    // disconnect old map overview if exists
    disconnect( mMapRenderer, SIGNAL( hasCrsTransformEnabled( bool ) ),
                mMapOverview, SLOT( hasCrsTransformEnabled( bool ) ) );
    disconnect( mMapRenderer, SIGNAL( destinationSrsChanged() ),
                mMapOverview, SLOT( destinationSrsChanged() ) );

    // map overview is not owned by map canvas so don't delete it...
  }

  mMapOverview = overview;

  if ( overview )
  {
    // connect to the map render to copy its projection settings
    connect( mMapRenderer, SIGNAL( hasCrsTransformEnabled( bool ) ),
             overview,     SLOT( hasCrsTransformEnabled( bool ) ) );
    connect( mMapRenderer, SIGNAL( destinationSrsChanged() ),
             overview,     SLOT( destinationSrsChanged() ) );
  }
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
  // we can't draw again if already drawing...
  if ( mDrawing )
    return;

  mDrawing = true;

  if ( mRenderFlag && !mFrozen )
  {
    clear();

    // Tell the user we're going to be a while
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit renderStarting();

    mMap->render();

    mDirty = false;

    // notify any listeners that rendering is complete
    QPainter p;
    p.begin( &mMap->paintDevice() );
    emit renderComplete( &p );
    p.end();

    // notifies current map tool
    if ( mMapTool )
      mMapTool->renderComplete();

    // Tell the user we've finished going to be a while
    QApplication::restoreOverrideCursor();
  }

  mDrawing = false;
} // refresh

void QgsMapCanvas::updateMap()
{
  if ( mMap )
  {
    mMap->updateContents();
  }
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
    mMapRenderer->render( &painter );
    emit renderComplete( &painter );
    painter.end();

    theQPixmap->save( theFileName, theFormat.toLocal8Bit().data() );
  }
  else //use the map view
  {
    QPixmap *pixmap = dynamic_cast<QPixmap *>( &mMap->paintDevice() );
    if ( !pixmap )
      return;

    pixmap->save( theFileName, theFormat.toLocal8Bit().data() );
  }
  //create a world file to go with the image...
  QgsRectangle myRect = mMapRenderer->extent();
  QString myHeader;
  // note: use 17 places of precision for all numbers output
  //Pixel XDim
  myHeader += QString::number( mapUnitsPerPixel(), 'g', 17 ) + "\r\n";
  //Rotation on y axis - hard coded
  myHeader += "0 \r\n";
  //Rotation on x axis - hard coded
  myHeader += "0 \r\n";
  //Pixel YDim - almost always negative - see
  //http://en.wikipedia.org/wiki/World_file#cite_note-2
  myHeader += "-" + QString::number( mapUnitsPerPixel(), 'g', 17 ) + "\r\n";
  //Origin X (center of top left cell)
  myHeader += QString::number( myRect.xMinimum() + ( mapUnitsPerPixel() / 2 ), 'g', 17 ) + "\r\n";
  //Origin Y (center of top left cell)
  myHeader += QString::number( myRect.yMaximum() - ( mapUnitsPerPixel() / 2 ), 'g', 17 ) + "\r\n";
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
  return mMapRenderer->extent();
} // extent

QgsRectangle QgsMapCanvas::fullExtent() const
{
  return mMapRenderer->fullExtent();
} // extent

void QgsMapCanvas::updateFullExtent()
{
  // projection settings have changed

  QgsDebugMsg( "updating full extent" );

  mMapRenderer->updateFullExtent();
  if ( mMapOverview )
  {
    mMapOverview->updateFullExtent( fullExtent() );
    updateOverview();
  }
  refresh();
}

void QgsMapCanvas::setExtent( QgsRectangle const & r )
{
  if ( mDrawing )
  {
    return;
  }

  QgsRectangle current = extent();

  if ( r.isEmpty() )
  {
    QgsDebugMsg( "Empty extent - keeping old extent with new center!" );
    QgsRectangle e( QgsPoint( r.center().x() - current.width() / 2.0, r.center().y() - current.height() / 2.0 ),
                    QgsPoint( r.center().x() + current.width() / 2.0, r.center().y() + current.height() / 2.0 ) );
    mMapRenderer->setExtent( e );
  }
  else
  {
    mMapRenderer->setExtent( r );
  }
  emit extentsChanged();
  updateScale();
  if ( mMapOverview )
    mMapOverview->drawExtentRect();
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
  double scale = mMapRenderer->scale();

  emit scaleChanged( scale );
}


void QgsMapCanvas::clear()
{
  // Indicate to the next paint event that we need to rebuild the canvas contents
  setDirty( true );

} // clear



void QgsMapCanvas::zoomToFullExtent()
{
  if ( mDrawing )
  {
    return;
  }

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
  if ( mDrawing )
  {
    return;
  }

  if ( mLastExtentIndex > 0 )
  {
    mLastExtentIndex--;
    mMapRenderer->setExtent( mLastExtent[mLastExtentIndex] );
    emit extentsChanged();
    updateScale();
    if ( mMapOverview )
      mMapOverview->drawExtentRect();
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
  if ( mDrawing )
  {
    return;
  }
  if ( mLastExtentIndex < mLastExtent.size() - 1 )
  {
    mLastExtentIndex++;
    mMapRenderer->setExtent( mLastExtent[mLastExtentIndex] );
    emit extentsChanged();
    updateScale();
    if ( mMapOverview )
      mMapOverview->drawExtentRect();
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
  return mMapRenderer->hasCrsTransformEnabled();
}

void QgsMapCanvas::mapUnitsChanged()
{
  // We assume that if the map units have changed, the changed value
  // will be accessible from QgsMapRenderer

  // And then force a redraw of the scale number in the status bar
  updateScale();

  // And then redraw the map to force the scale bar to update
  // itself. This is less than ideal as the entire map gets redrawn
  // just to get the scale bar to redraw itself. If we ask the scale
  // bar to redraw itself without redrawing the map, the existing
  // scale bar is not removed, and we end up with two scale bars in
  // the same location. This can perhaps be fixed when/if the scale
  // bar is done as a transparent layer on top of the map canvas.
  refresh();
}

void QgsMapCanvas::zoomToSelected( QgsVectorLayer* layer )
{
  if ( mDrawing )
  {
    return;
  }

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

  QgsRectangle rect = mMapRenderer->layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );

  // no selected features, only one selected point feature
  //or two point features with the same x- or y-coordinates
  if ( rect.isEmpty() )
  {
    // zoom in
    QgsPoint c = rect.center();
    rect = extent();
    rect.expand( 0.25, &c );
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
  if ( mDrawing )
  {
    return;
  }

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

  QgsRectangle rect = mMapRenderer->layerExtentToOutputExtent( layer, layer->boundingBoxOfSelected() );
  setExtent( QgsRectangle( rect.center(), rect.center() ) );
  refresh();
} // panToSelected

void QgsMapCanvas::keyPressEvent( QKeyEvent * e )
{

  if ( mDrawing )
  {
    e->ignore();
  }

  emit keyPressed( e );

  if ( mCanvasProperties->mouseButtonDown || mCanvasProperties->panSelectorDown )
    return;

  QPainter paint;
  QPen     pen( Qt::gray );
  QgsPoint ll, ur;

  if ( ! mCanvasProperties->mouseButtonDown )
  {
    // Don't want to interfer with mouse events

    QgsRectangle currentExtent = mMapRenderer->extent();
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

      default:
        // Pass it on
        if ( mMapTool )
        {
          mMapTool->keyPressEvent( e );
        }
        e->ignore();

        QgsDebugMsg( "Ignoring key: " + QString::number( e->key() ) );

    }
  }
} //keyPressEvent()

void QgsMapCanvas::keyReleaseEvent( QKeyEvent * e )
{
  QgsDebugMsg( "keyRelease event" );

  if ( mDrawing )
  {
    return;
  }

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

      e->ignore();

      QgsDebugMsg( "Ignoring key release: " + QString::number( e->key() ) );
  }

  emit keyReleased( e );

} //keyReleaseEvent()


void QgsMapCanvas::mouseDoubleClickEvent( QMouseEvent * e )
{
  if ( mDrawing )
  {
    return;
  }

  // call handler of current map tool
  if ( mMapTool )
    mMapTool->canvasDoubleClickEvent( e );
} // mouseDoubleClickEvent


void QgsMapCanvas::mousePressEvent( QMouseEvent * e )
{
  if ( mDrawing )
  {
    return;
  }

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
  if ( mDrawing )
  {
    return;
  }

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
  mNewSize = e->size();
}

void QgsMapCanvas::paintEvent( QPaintEvent *e )
{
  if ( mNewSize.isValid() )
  {
    if ( mPainting || mDrawing )
    {
      //cancel current render progress
      if ( mMapRenderer )
      {
        QgsRenderContext* theRenderContext = mMapRenderer->rendererContext();
        if ( theRenderContext )
        {
          theRenderContext->setRenderingStopped( true );
        }
      }
      return;
    }

    mPainting = true;

    while ( mNewSize.isValid() )
    {
      QSize lastSize = mNewSize;
      mNewSize = QSize();

      //set map size before scene size helps keep scene indexes updated properly
      // this was the cause of rubberband artifacts
      mMap->resize( lastSize );
      mScene->setSceneRect( QRectF( 0, 0, lastSize.width(), lastSize.height() ) );

      // notify canvas items of change
      updateCanvasItemPositions();

      updateScale();

      refresh();

      emit extentsChanged();
    }

    mPainting = false;
  }

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

    it++;
  }
}


void QgsMapCanvas::wheelEvent( QWheelEvent *e )
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in

  QgsDebugMsg( "Wheel event delta " + QString::number( e->delta() ) );

  if ( mDrawing )
  {
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

      QgsPoint oldCenter( mMapRenderer->extent().center() );
      QgsPoint mousePos( getCoordinateTransform()->toMapPoint( e->x(), e->y() ) );
      QgsPoint newCenter( mousePos.x() + (( oldCenter.x() - mousePos.x() ) * scaleFactor ),
                          mousePos.y() + (( oldCenter.y() - mousePos.y() ) * scaleFactor ) );

      // same as zoomWithCenter (no coordinate transformations are needed)
      QgsRectangle extent = mMapRenderer->extent();
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
  if ( mDrawing )
  {
    return;
  }

  double scaleFactor = ( zoomIn ? 1 / mWheelZoomFactor : mWheelZoomFactor );

  // transform the mouse pos to map coordinates
  QgsPoint center  = getCoordinateTransform()->toMapPoint( x, y );
  QgsRectangle r = mMapRenderer->extent();
  r.scale( scaleFactor, &center );
  setExtent( r );
  refresh();
}

void QgsMapCanvas::mouseMoveEvent( QMouseEvent * e )
{
  if ( mDrawing )
  {
    return;
  }

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

  // set new map tool and activate it
  mMapTool = tool;
  if ( mMapTool )
  {
    connect( mMapTool, SIGNAL( destroyed() ), this, SLOT( mapToolDestroyed() ) );
    mMapTool->activate();
  }

  emit mapToolSet( mMapTool );
} // setMapTool

void QgsMapCanvas::unsetMapTool( QgsMapTool* tool )
{
  if ( mMapTool && mMapTool == tool )
  {
    mMapTool->deactivate();
    mMapTool = NULL;
    emit mapToolSet( NULL );
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
  mMap->setBackgroundColor( theColor );

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

int QgsMapCanvas::layerCount() const
{
  return mMapRenderer->layerSet().size();
} // layerCount


QList<QgsMapLayer*> QgsMapCanvas::layers() const
{
  QList<QgsMapLayer*> lst;
  foreach ( QString layerID, mMapRenderer->layerSet() )
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


QPixmap& QgsMapCanvas::canvasPixmap()
{
  QPixmap *pixmap = dynamic_cast<QPixmap *>( &canvasPaintDevice() );
  if ( pixmap )
  {
    return *pixmap;
  }

  qWarning( "QgsMapCanvas::canvasPixmap() deprecated - returning static pixmap instance - use QgsMapCanvas::paintDevice()" );

  static QPixmap staticPixmap;

  QImage *image = dynamic_cast<QImage *>( &mMap->paintDevice() );
  if ( image )
  {
    staticPixmap = QPixmap::fromImage( *image );
  }
  else
  {
    staticPixmap = QPixmap( canvasPaintDevice().width(), canvasPaintDevice().height() );
  }

  return staticPixmap;
} // canvasPixmap

QPaintDevice &QgsMapCanvas::canvasPaintDevice()
{
  return mMap->paintDevice();
}

double QgsMapCanvas::mapUnitsPerPixel() const
{
  return mMapRenderer->mapUnitsPerPixel();
} // mapUnitsPerPixel


void QgsMapCanvas::setMapUnits( QGis::UnitType u )
{
  QgsDebugMsg( "Setting map units to " + QString::number( static_cast<int>( u ) ) );
  mMapRenderer->setMapUnits( u );
}


QGis::UnitType QgsMapCanvas::mapUnits() const
{
  return mMapRenderer->mapUnits();
}


void QgsMapCanvas::setRenderFlag( bool theFlag )
{
  mRenderFlag = theFlag;
  if ( mMapRenderer )
  {
    QgsRenderContext* rc = mMapRenderer->rendererContext();
    if ( rc )
    {
      rc->setRenderingStopped( !theFlag );
    }
  }

  if ( mRenderFlag )
  {
    refresh();
  }
}

void QgsMapCanvas::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
  QgsDebugMsg( "QgsMapCanvas connected to " + QString( signal ) );
} //connectNotify



QgsMapTool* QgsMapCanvas::mapTool()
{
  return mMapTool;
}

void QgsMapCanvas::panActionEnd( QPoint releasePoint )
{
  if ( mDrawing )
  {
    return;
  }

  // move map image and other items to standard position
  moveCanvasContents( true ); // true means reset

  // use start and end box points to calculate the extent
  QgsPoint start = getCoordinateTransform()->toMapCoordinates( mCanvasProperties->rubberStartPoint );
  QgsPoint end = getCoordinateTransform()->toMapCoordinates( releasePoint );

  double dx = qAbs( end.x() - start.x() );
  double dy = qAbs( end.y() - start.y() );

  // modify the extent
  QgsRectangle r = mMapRenderer->extent();

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
  refresh();
}

void QgsMapCanvas::panAction( QMouseEvent * e )
{
  Q_UNUSED( e );

  if ( mDrawing )
  {
    return;
  }

  // move all map canvas items
  moveCanvasContents();

  // update canvas
  //updateContents(); // TODO: need to update?
}

void QgsMapCanvas::moveCanvasContents( bool reset )
{
  if ( mDrawing )
  {
    return;
  }

  QPoint pnt( 0, 0 );
  if ( !reset )
    pnt += mCanvasProperties->mouseLastXY - mCanvasProperties->rubberStartPoint;

  mMap->setPanningOffset( pnt );

  QList<QGraphicsItem*> list = mScene->items();
  QList<QGraphicsItem*>::iterator it = list.begin();
  while ( it != list.end() )
  {
    QGraphicsItem* item = *it;

    if ( item != mMap )
    {
      // this tells map canvas item to draw with offset
      QgsMapCanvasItem* canvasItem = dynamic_cast<QgsMapCanvasItem *>( item );
      if ( canvasItem )
        canvasItem->setPanningOffset( pnt );
    }

    it++;
  }

  // show items
  updateCanvasItemPositions();

}

void QgsMapCanvas::showError( QgsMapLayer * mapLayer )
{
#if 0
  QMessageBox::warning(
    this,
    mapLayer->lastErrorTitle(),
    tr( "Could not draw %1 because:\n%2", "COMMENTED OUT" ).arg( mapLayer->name() ).arg( mapLayer->lastError() )
  );
#endif

  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( mapLayer->lastErrorTitle() );
  mv->setMessageAsPlainText( tr( "Could not draw %1 because:\n%2" )
                             .arg( mapLayer->name() ).arg( mapLayer->lastError() ) );
  mv->exec();
  //MH
  //QgsMessageViewer automatically sets delete on close flag
  //so deleting mv would lead to a segfault
}

QPoint QgsMapCanvas::mouseLastXY()
{
  return mCanvasProperties->mouseLastXY;
}

void QgsMapCanvas::readProject( const QDomDocument & doc )
{
  QDomNodeList nodes = doc.elementsByTagName( "mapcanvas" );
  if ( nodes.count() )
  {
    QDomNode node = nodes.item( 0 );
    mMapRenderer->readXML( node );
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
  mMapRenderer->writeXML( mapcanvasNode, doc );
}

void QgsMapCanvas::zoomByFactor( double scaleFactor )
{
  if ( mDrawing )
  {
    return;
  }

  QgsRectangle r = mMapRenderer->extent();
  r.scale( scaleFactor );
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

void QgsMapCanvas::crsTransformEnabled( bool enabled )
{
  if ( enabled )
  {
    QgsDebugMsg( "refreshing after reprojection was enabled" );
    refresh();
    connect( mMapRenderer, SIGNAL( destinationSrsChanged() ), this, SLOT( refresh() ) );
  }
  else
    disconnect( mMapRenderer, SIGNAL( destinationSrsChanged() ), this, SLOT( refresh() ) );
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
  if ( mDrawing )
  {
    return done;
  }
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
