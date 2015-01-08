/***************************************************************************
                           qgsmapoverviewcanvas.cpp
                      Map canvas subclassed for overview
                              -------------------
    begin                : 09/14/2005
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmaptopixel.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include "qgslogger.h"
#include <limits.h>

//! widget that serves as rectangle showing current extent in overview
class QgsPanningWidget : public QWidget
{
  public:
    QgsPanningWidget( QWidget* parent )
        : QWidget( parent )
    {
      setObjectName( "panningWidget" );
      setMinimumSize( 5, 5 );
      setAttribute( Qt::WA_NoSystemBackground );
    }

    void resizeEvent( QResizeEvent* r )
    {
      QSize s = r->size();
      QRegion reg( 0, 0, s.width(), s.height() );
      QRegion reg2( 2, 2, s.width() - 4, s.height() - 4 );
      QRegion reg3 = reg.subtracted( reg2 );
      setMask( reg3 );
    }


    void paintEvent( QPaintEvent* pe )
    {
      Q_UNUSED( pe );

      QRect r( QPoint( 0, 0 ), size() );
      QPainter p;
      p.begin( this );
      p.setPen( Qt::red );
      p.setBrush( Qt::red );
      p.drawRect( r );
      p.end();
    }

};



QgsMapOverviewCanvas::QgsMapOverviewCanvas( QWidget * parent, QgsMapCanvas* mapCanvas )
    : QWidget( parent )
    , mMapCanvas( mapCanvas )
    , mJob( 0 )
{
  setObjectName( "theOverviewCanvas" );
  mPanningWidget = new QgsPanningWidget( this );

  mSettings.setFlag( QgsMapSettings::DrawLabeling, false );

  connect( mMapCanvas, SIGNAL( extentsChanged() ), this, SLOT( drawExtentRect() ) );
}

QgsMapOverviewCanvas::~QgsMapOverviewCanvas()
{
}

void QgsMapOverviewCanvas::resizeEvent( QResizeEvent* e )
{
  mPixmap = QPixmap();

  mSettings.setOutputSize( e->size() );

  updateFullExtent();

  refresh();

  QWidget::resizeEvent( e );
}

void QgsMapOverviewCanvas::paintEvent( QPaintEvent* pe )
{
  if ( !mPixmap.isNull() )
  {
    QPainter paint( this );
    paint.drawPixmap( pe->rect().topLeft(), mPixmap, pe->rect() );
  }
}


void QgsMapOverviewCanvas::drawExtentRect()
{
  if ( !mMapCanvas ) return;

  const QgsRectangle& extent = mMapCanvas->extent();

  // show only when valid extent is set
  if ( extent.isEmpty() || mSettings.visibleExtent().isEmpty() )
  {
    mPanningWidget->hide();
    return;
  }

  const QgsMapToPixel& cXf = mSettings.mapToPixel();
  QgsPoint ll( extent.xMinimum(), extent.yMinimum() );
  QgsPoint ur( extent.xMaximum(), extent.yMaximum() );

  // transform the points before drawing
  cXf.transform( &ll );
  cXf.transform( &ur );

#if 0
  // test whether panning widget should be drawn
  bool show = false;
  if ( ur.x() >= 0 && ur.x() < width() )
    show = true;
  if ( ll.x() >= 0 && ll.x() < width() )
    show = true;
  if ( ur.y() >= 0 && ur.y() < height() )
    show = true;
  if ( ll.y() >= 0 && ll.y() < height() )
    show = true;
  if ( !show )
  {
    QgsDebugMsg( "panning: extent out of overview area" );
    mPanningWidget->hide();
    return;
  }
#endif

  // round values
  int x1 = static_cast<int>( ur.x() + 0.5 ), x2 = static_cast<int>( ll.x() + 0.5 );
  int y1 = static_cast<int>( ur.y() + 0.5 ), y2 = static_cast<int>( ll.y() + 0.5 );

  if ( x1 > x2 )
    std::swap( x1, x2 );
  if ( y1 > y2 )
    std::swap( y1, y2 );

#ifdef Q_OS_MAC
  // setGeometry (Qt 4.2) is causing Mac window corruption (decorations
  // are drawn at odd locations) if both coords are at limit. This may
  // have something to do with Qt calculating dimensions as x2 - x1 + 1.
  // (INT_MAX - INT_MIN + 1 is UINT_MAX + 1)
  if ( x1 == INT_MIN && x2 == INT_MAX )
    x1 += 1;  // x2 -= 1 works too
  if ( y1 == INT_MIN && y2 == INT_MAX )
    y1 += 1;
#endif

  QRect r( x1, y1, x2 - x1 + 1, y2 - y1 + 1 );

  // allow for 5 pixel minimum widget size
  if ( r.width() < 5 && x1 > INT_MIN + 2 ) // make sure no underflow occurs (2 is largest adjustment)
  {
    r.setX( r.x() - (( 5 - r.width() ) / 2 ) );  // adjust x  by 1/2 the difference of calculated and min. width
    r.setWidth( 5 );
  }
  if ( r.height() < 5 && y1 > INT_MIN + 2 )
  {
    r.setY( r.y() - (( 5 - r.height() ) / 2 ) );  // adjust y
    r.setHeight( 5 );
  }

  QgsDebugMsg( QString( "panning: extent to widget: [%1,%2] [%3x%4]" ).arg( x1 ).arg( y1 ).arg( r.width() ).arg( r.height() ) );

  mPanningWidget->setGeometry( r );
  mPanningWidget->show(); // show if hidden
}


void QgsMapOverviewCanvas::mousePressEvent( QMouseEvent * e )
{
//  if (mPanningWidget->isHidden())
//    return;

  // set offset in panning widget if inside it
  // for better experience with panning :)
  if ( mPanningWidget->geometry().contains( e->pos() ) )
  {
    mPanningCursorOffset = e->pos() - mPanningWidget->pos();
  }
  else
  {
    // use center of the panning widget if outside
    QSize s = mPanningWidget->size();
    mPanningCursorOffset = QPoint( s.width() / 2, s.height() / 2 );
  }
  updatePanningWidget( e->pos() );
}


void QgsMapOverviewCanvas::mouseReleaseEvent( QMouseEvent * e )
{
//  if (mPanningWidget->isHidden())
//    return;

  if ( e->button() == Qt::LeftButton )
  {
    // set new extent
    const QgsMapToPixel& cXf = mSettings.mapToPixel();
    QRect rect = mPanningWidget->geometry();

    QgsPoint center = cXf.toMapCoordinates( rect.center() );
    mMapCanvas->setCenter( center );
    mMapCanvas->refresh();
  }
}


void QgsMapOverviewCanvas::mouseMoveEvent( QMouseEvent * e )
{
  // move with panning widget if tracking cursor
  if (( e->buttons() & Qt::LeftButton ) == Qt::LeftButton )
  {
    updatePanningWidget( e->pos() );
  }
}


void QgsMapOverviewCanvas::updatePanningWidget( const QPoint& pos )
{
//  if (mPanningWidget->isHidden())
//    return;
  mPanningWidget->move( pos.x() - mPanningCursorOffset.x(), pos.y() - mPanningCursorOffset.y() );
}

void QgsMapOverviewCanvas::refresh()
{
  updateFullExtent();

  if ( !mSettings.hasValidSettings() )
  {
    mPixmap = QPixmap();
    update();
    return; // makes no sense to render anything
  }

  if ( mJob )
  {
    QgsDebugMsg( "oveview - cancelling old" );
    mJob->cancel();
    QgsDebugMsg( "oveview - deleting old" );
    delete mJob; // get rid of previous job (if any)
  }

  QgsDebugMsg( "oveview - starting new" );

  // TODO: setup overview mode
  mJob = new QgsMapRendererSequentialJob( mSettings );
  connect( mJob, SIGNAL( finished() ), this, SLOT( mapRenderingFinished() ) );
  mJob->start();

  setBackgroundColor( mMapCanvas->mapSettings().backgroundColor() );

  // schedule repaint
  update();

  // update panning widget
  drawExtentRect();
}

void QgsMapOverviewCanvas::mapRenderingFinished()
{
  QgsDebugMsg( "overview - finished" );
  mPixmap = QPixmap::fromImage( mJob->renderedImage() );

  delete mJob;
  mJob = 0;

  // schedule repaint
  update();
}

void QgsMapOverviewCanvas::layerRepaintRequested()
{
  refresh();
}


void QgsMapOverviewCanvas::setBackgroundColor( const QColor& color )
{
  mSettings.setBackgroundColor( color );

  // set erase color
  QPalette palette;
  palette.setColor( backgroundRole(), color );
  setPalette( palette );
}

void QgsMapOverviewCanvas::setLayerSet( const QStringList& layerSet )
{
  QgsDebugMsg( "layerSet: " + layerSet.join( ", " ) );

  foreach ( const QString& layerID, mSettings.layers() )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      disconnect( ml, SIGNAL( repaintRequested() ), this, SLOT( layerRepaintRequested() ) );
  }

  mSettings.setLayers( layerSet );

  foreach ( const QString& layerID, mSettings.layers() )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      connect( ml, SIGNAL( repaintRequested() ), this, SLOT( layerRepaintRequested() ) );
  }

  updateFullExtent();
}

void QgsMapOverviewCanvas::updateFullExtent()
{
  QgsRectangle rect;
  if ( mSettings.hasValidSettings() )
    rect = mSettings.fullExtent();
  else
    rect = mMapCanvas->fullExtent();

  // expand a bit to keep features on margin
  rect.scale( 1.1 );

  mSettings.setExtent( rect );
  drawExtentRect();
}

void QgsMapOverviewCanvas::hasCrsTransformEnabled( bool flag )
{
  mSettings.setCrsTransformEnabled( flag );
}

void QgsMapOverviewCanvas::destinationSrsChanged()
{
  mSettings.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
}

QStringList QgsMapOverviewCanvas::layerSet() const
{
  return mSettings.layers();
}
