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
#include "qgsproject.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmaptopixel.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include "qgslogger.h"
#include <limits>


QgsMapOverviewCanvas::QgsMapOverviewCanvas( QWidget *parent, QgsMapCanvas *mapCanvas )
  : QWidget( parent )
  , mMapCanvas( mapCanvas )

{
  setAutoFillBackground( true );
  setObjectName( QStringLiteral( "theOverviewCanvas" ) );
  mPanningWidget = new QgsPanningWidget( this );

  mSettings.setTransformContext( mMapCanvas->mapSettings().transformContext() );
  mSettings.setFlag( QgsMapSettings::DrawLabeling, false );

  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapOverviewCanvas::drawExtentRect );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapOverviewCanvas::destinationCrsChanged );
  connect( mMapCanvas, &QgsMapCanvas::transformContextChanged, this, &QgsMapOverviewCanvas::transformContextChanged );
}

void QgsMapOverviewCanvas::resizeEvent( QResizeEvent *e )
{
  mPixmap = QPixmap();

  mSettings.setOutputSize( e->size() );

  updateFullExtent();

  refresh();

  QWidget::resizeEvent( e );
}

void QgsMapOverviewCanvas::showEvent( QShowEvent *e )
{
  refresh();
  QWidget::showEvent( e );
}

void QgsMapOverviewCanvas::paintEvent( QPaintEvent *pe )
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

  const QgsRectangle &extent = mMapCanvas->extent();

  // show only when valid extent is set
  if ( extent.isEmpty() || mSettings.visibleExtent().isEmpty() )
  {
    mPanningWidget->hide();
    return;
  }

  const QPolygonF &vPoly = mMapCanvas->mapSettings().visiblePolygon();
  const QgsMapToPixel &cXf = mSettings.mapToPixel();
  QVector< QPoint > pts;
  pts.push_back( cXf.transform( QgsPointXY( vPoly[0] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPointXY( vPoly[1] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPointXY( vPoly[2] ) ).toQPointF().toPoint() );
  pts.push_back( cXf.transform( QgsPointXY( vPoly[3] ) ).toQPointF().toPoint() );
  mPanningWidget->setPolygon( QPolygon( pts ) );
  mPanningWidget->show(); // show if hidden
}


void QgsMapOverviewCanvas::mousePressEvent( QMouseEvent *e )
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


void QgsMapOverviewCanvas::mouseReleaseEvent( QMouseEvent *e )
{
//  if (mPanningWidget->isHidden())
//    return;

  if ( e->button() == Qt::LeftButton )
  {
    // set new extent
    const QgsMapToPixel &cXf = mSettings.mapToPixel();
    QRect rect = mPanningWidget->geometry();

    QgsPointXY center = cXf.toMapCoordinates( rect.center() );
    mMapCanvas->setCenter( center );
    mMapCanvas->refresh();
  }
}


void QgsMapOverviewCanvas::mouseMoveEvent( QMouseEvent *e )
{
  // move with panning widget if tracking cursor
  if ( ( e->buttons() & Qt::LeftButton ) == Qt::LeftButton )
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
    QgsDebugMsg( QStringLiteral( "oveview - canceling old" ) );
    mJob->cancel();
    QgsDebugMsg( QStringLiteral( "oveview - deleting old" ) );
    delete mJob; // get rid of previous job (if any)
  }

  QgsDebugMsg( QStringLiteral( "oveview - starting new" ) );

  // TODO: setup overview mode
  mJob = new QgsMapRendererSequentialJob( mSettings );
  connect( mJob, &QgsMapRendererJob::finished, this, &QgsMapOverviewCanvas::mapRenderingFinished );
  mJob->start();

  setBackgroundColor( mMapCanvas->mapSettings().backgroundColor() );

  // schedule repaint
  update();

  // update panning widget
  drawExtentRect();
}

void QgsMapOverviewCanvas::mapRenderingFinished()
{
  QgsDebugMsg( QStringLiteral( "overview - finished" ) );
  mPixmap = QPixmap::fromImage( mJob->renderedImage() );

  delete mJob;
  mJob = nullptr;

  // schedule repaint
  update();
}

void QgsMapOverviewCanvas::layerRepaintRequested( bool deferred )
{
  if ( !deferred )
    refresh();
}


void QgsMapOverviewCanvas::setBackgroundColor( const QColor &color )
{
  mSettings.setBackgroundColor( color );

  // set erase color
  QPalette palette;
  palette.setColor( backgroundRole(), color );
  setPalette( palette );
}

void QgsMapOverviewCanvas::setLayers( const QList<QgsMapLayer *> &layers )
{
  Q_FOREACH ( QgsMapLayer *ml, mSettings.layers() )
  {
    disconnect( ml, &QgsMapLayer::repaintRequested, this, &QgsMapOverviewCanvas::layerRepaintRequested );
  }

  mSettings.setLayers( layers );

  Q_FOREACH ( QgsMapLayer *ml, mSettings.layers() )
  {
    connect( ml, &QgsMapLayer::repaintRequested, this, &QgsMapOverviewCanvas::layerRepaintRequested );
  }

  updateFullExtent();

  refresh();
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

void QgsMapOverviewCanvas::destinationCrsChanged()
{
  mSettings.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
}

void QgsMapOverviewCanvas::transformContextChanged()
{
  mSettings.setTransformContext( mMapCanvas->mapSettings().transformContext() );
}

QList<QgsMapLayer *> QgsMapOverviewCanvas::layers() const
{
  return mSettings.layers();
}


/// @cond PRIVATE

QgsPanningWidget::QgsPanningWidget( QWidget *parent )
  : QWidget( parent )
{
  setObjectName( QStringLiteral( "panningWidget" ) );
  setMinimumSize( 5, 5 );
  setAttribute( Qt::WA_NoSystemBackground );
}

void QgsPanningWidget::setPolygon( const QPolygon &p )
{
  if ( p == mPoly ) return;
  mPoly = p;

  //ensure polygon is closed
  if ( mPoly.at( 0 ) != mPoly.at( mPoly.length() - 1 ) )
    mPoly.append( mPoly.at( 0 ) );

  setGeometry( p.boundingRect() );
  update();
}

void QgsPanningWidget::paintEvent( QPaintEvent *pe )
{
  Q_UNUSED( pe );

  QPainter p;
  p.begin( this );
  p.setPen( Qt::red );
  QPolygonF t = mPoly.translated( -mPoly.boundingRect().left(), -mPoly.boundingRect().top() );

  // drawPolygon causes issues on windows - corners of path may be missing resulting in triangles being drawn
  // instead of rectangles! (Same cause as #13343)
  QPainterPath path;
  path.addPolygon( t );
  p.drawPath( path );

  p.end();
}



///@endcond
