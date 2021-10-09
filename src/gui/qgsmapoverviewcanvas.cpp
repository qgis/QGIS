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
#include "qgsprojectviewsettings.h"

#include <QPainter>
#include <QPainterPath>
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
  mSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, false );

  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapOverviewCanvas::drawExtentRect );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapOverviewCanvas::destinationCrsChanged );
  connect( mMapCanvas, &QgsMapCanvas::transformContextChanged, this, &QgsMapOverviewCanvas::transformContextChanged );

  connect( QgsProject::instance()->viewSettings(), &QgsProjectViewSettings::presetFullExtentChanged, this, &QgsMapOverviewCanvas::refresh );
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
    const QSize s = mPanningWidget->size();
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
    const QRect rect = mPanningWidget->geometry();

    const QgsPointXY center = cXf.toMapCoordinates( rect.center() );
    mMapCanvas->setCenter( center );
    mMapCanvas->refresh();
  }
}


void QgsMapOverviewCanvas::wheelEvent( QWheelEvent *e )
{
  double zoomFactor = e->angleDelta().y() > 0 ? 1. / mMapCanvas->zoomInFactor() : mMapCanvas->zoomOutFactor();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( e->angleDelta().y() );

  if ( e->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  const double signedWheelFactor = e->angleDelta().y() > 0 ? 1 / zoomFactor : zoomFactor;

  const QgsMapToPixel &cXf = mSettings.mapToPixel();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QgsPointXY center = cXf.toMapCoordinates( e->pos().x(), e->pos().y() );
  updatePanningWidget( QPoint( e->pos().x(), e->pos().y() ) );
#else
  const QgsPointXY center = cXf.toMapCoordinates( e->position().x(), e->position().y() );
  updatePanningWidget( QPoint( e->position().x(), e->position().y() ) );
#endif
  mMapCanvas->zoomByFactor( signedWheelFactor, &center );
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
  const auto oldLayers = mSettings.layers();
  for ( QgsMapLayer *ml : oldLayers )
  {
    disconnect( ml, &QgsMapLayer::repaintRequested, this, &QgsMapOverviewCanvas::layerRepaintRequested );
  }

  mSettings.setLayers( layers );

  const auto newLayers = mSettings.layers();
  for ( QgsMapLayer *ml : newLayers )
  {
    connect( ml, &QgsMapLayer::repaintRequested, this, &QgsMapOverviewCanvas::layerRepaintRequested );
  }

  updateFullExtent();

  refresh();
}

void QgsMapOverviewCanvas::updateFullExtent()
{
  QgsRectangle rect;
  if ( !QgsProject::instance()->viewSettings()->presetFullExtent().isNull() )
  {
    const QgsReferencedRectangle extent = QgsProject::instance()->viewSettings()->fullExtent();
    QgsCoordinateTransform ct( extent.crs(), mSettings.destinationCrs(), QgsProject::instance()->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      rect = ct.transformBoundingBox( extent );
    }
    catch ( QgsCsException & )
    {
    }
  }

  if ( rect.isNull() )
  {
    if ( mSettings.hasValidSettings() )
      rect = mSettings.fullExtent();
    else
      rect = mMapCanvas->projectExtent();
  }

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

  const QRect rect = p.boundingRect() + QMargins( 1, 1, 1, 1 );
  setGeometry( rect );
  update();
}

void QgsPanningWidget::paintEvent( QPaintEvent *pe )
{
  Q_UNUSED( pe )

  QPainter p;

  p.begin( this );
  const QPolygonF t = mPoly.translated( -mPoly.boundingRect().left() + 1, -mPoly.boundingRect().top() + 1 );

  // drawPolygon causes issues on windows - corners of path may be missing resulting in triangles being drawn
  // instead of rectangles! (Same cause as #13343)
  QPainterPath path;
  path.addPolygon( t );

  QPen pen;
  pen.setJoinStyle( Qt::MiterJoin );
  pen.setColor( Qt::white );
  pen.setWidth( 3 );
  p.setPen( pen );
  p.drawPath( path );
  pen.setColor( Qt::red );
  pen.setWidth( 1 );
  p.setPen( pen );
  p.drawPath( path );

  p.end();
}



///@endcond
