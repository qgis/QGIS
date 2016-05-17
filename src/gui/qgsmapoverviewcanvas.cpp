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


QgsMapOverviewCanvas::QgsMapOverviewCanvas( QWidget * parent, QgsMapCanvas* mapCanvas )
    : QWidget( parent )
    , mMapCanvas( mapCanvas )
    , mJob( nullptr )
{
  setAutoFillBackground( true );
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

void QgsMapOverviewCanvas::showEvent( QShowEvent* e )
{
  refresh();
  QWidget::showEvent( e );
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

  const QPolygonF& vPoly = mMapCanvas->mapSettings().visiblePolygon();
  const QgsMapToPixel& cXf = mSettings.mapToPixel();
  QVector< QPoint > pts;
  pts.push_back( cXf.transform( QgsPoint( vPoly[0] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPoint( vPoly[1] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPoint( vPoly[2] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPoint( vPoly[3] ) ).toQPointF().toPoint() );
  mPanningWidget->setPolygon( QPolygon( pts ) );
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


void QgsMapOverviewCanvas::updatePanningWidget( QPoint pos )
{
//  if (mPanningWidget->isHidden())
//    return;
  mPanningWidget->move( pos.x() - mPanningCursorOffset.x(), pos.y() - mPanningCursorOffset.y() );
}

void QgsMapOverviewCanvas::refresh()
{
  if ( !isVisible() )
    return;

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
  mJob = nullptr;

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

  Q_FOREACH ( const QString& layerID, mSettings.layers() )
  {
    if ( QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerID ) )
      disconnect( ml, SIGNAL( repaintRequested() ), this, SLOT( layerRepaintRequested() ) );
  }

  mSettings.setLayers( layerSet );

  Q_FOREACH ( const QString& layerID, mSettings.layers() )
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


/// @cond PRIVATE

QgsPanningWidget::QgsPanningWidget( QWidget* parent )
    : QWidget( parent )
{
  setObjectName( "panningWidget" );
  setMinimumSize( 5, 5 );
  setAttribute( Qt::WA_NoSystemBackground );
}

void QgsPanningWidget::setPolygon( const QPolygon& p )
{
  if ( p == mPoly ) return;
  mPoly = p;
  setGeometry( p.boundingRect() );
  update();
}

void QgsPanningWidget::paintEvent( QPaintEvent* pe )
{
  Q_UNUSED( pe );

  QPainter p;
  p.begin( this );
  p.setPen( Qt::red );
  QPolygonF t = mPoly.translated( -mPoly.boundingRect().left(), -mPoly.boundingRect().top() );
  p.drawConvexPolygon( t );
  p.end();
}



///@endcond
