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
/* $Id$ */

#include "qgsmapoverviewcanvas.h"
#include "qgsmapcanvasproperties.h"

#include <qpainter.h>

//! widget that serves as rectangle showing current extent in overview
class QgsPanningWidget : public QWidget
{
public:
  QgsPanningWidget(QWidget* parent)
  : QWidget(parent, "panningWidget")
  {
    setMinimumSize(5,5);
    setBackgroundMode(Qt::NoBackground);
  }

  void resizeEvent(QResizeEvent* r)
  {
    QSize s = r->size();
    QRegion reg(0,0, s.width(), s.height());
    QRegion reg2(2,2, s.width()-4, s.height()-4);
    QRegion reg3 = reg.subtract(reg2);
    setMask(reg3);
  }


  void paintEvent(QPaintEvent* pe)
  {
    QRect r(QPoint(0,0), size());
    QPainter p;
    p.begin(this);
    p.setPen(Qt::red);
    p.setBrush(Qt::red);
    p.drawRect(r);
    p.end();
  }

};



QgsMapOverviewCanvas::QgsMapOverviewCanvas(QWidget * parent, QgsMapCanvas* mapCanvas)
  : QgsMapCanvas(parent, "theOverviewCanvas"), mMapCanvas(mapCanvas)
{
  mIsOverviewCanvas = true;
  
  mPanningWidget = new QgsPanningWidget(this);
}


void QgsMapOverviewCanvas::addLayer(QgsMapLayer * lyr)
{
  Q_CHECK_PTR( lyr );

  if ( ! lyr )
  {
    return;
  }

  // regardless of what happens, we need to be in communication with this
  // layer to possibly add it to overview canvas; however, we only want to
  // make this connection once, so we first check to see if we already
  // know about the layer
  if ( mCanvasProperties->layers.end() ==
       mCanvasProperties->layers.find(lyr->getLayerID() )  )
  {
    QObject::connect(lyr, SIGNAL(showInOverview(QgsMapLayer *, bool)),
                     this, SLOT(showInOverview( QgsMapLayer *, bool )));
  }

  if ( ! lyr->showInOverviewStatus() )
  {
#ifdef QGISDEBUG
      qDebug( lyr->name() + " not in overview, so skipping in addLayer()" );
#endif
  }
  else
  {
#ifdef QGISDEBUG
      qDebug( lyr->name() + " in overview, invoking addLayer()" );
#endif
    QgsMapCanvas::addLayer(lyr);
  }
}


void QgsMapOverviewCanvas::showInOverview( QgsMapLayer * maplayer, bool visible )
{
  std::map < QString, QgsMapLayer * >::iterator found =
      mCanvasProperties->layers.find(maplayer->getLayerID());

  // if it's visible, and we already know about it, then do nothing;
  // otherwise, we need to add it if "visible" says so
  if ( found == mCanvasProperties->layers.end() &&
       visible )
  {
    addLayer( maplayer );
  } // if we have it and it's supposed to be removed, remove it
  else if ( found != mCanvasProperties->layers.end() &&
            ! visible )
  {
    remove
        ( maplayer->getLayerID() );
  }

}


void QgsMapOverviewCanvas::reflectChangedExtent()
{
  const QgsRect& extent = mMapCanvas->extent();
  
  // show only when valid extent is set
  if (extent.isEmpty())
  {
#ifdef QGISDEBUG
    std::cout << "panning: empty extent" << std::endl;
#endif
    mPanningWidget->hide();
    return;
  }
  
  QgsMapToPixel* cXf = getCoordinateTransform();
  QgsPoint ll(extent.xMin(), extent.yMin());
  QgsPoint ur(extent.xMax(), extent.yMax());
  if(cXf)
  {
    // transform the points before drawing
    cXf->transform(&ll);
    cXf->transform(&ur);
  }
  /*  
  // test whether panning widget should be drawn  
  bool show = false;
  if (ur.x() >= 0 && ur.x() < width())  show = true;
  if (ll.x() >= 0 && ll.x() < width())  show = true;
  if (ur.y() >= 0 && ur.y() < height()) show = true;
  if (ll.y() >= 0 && ll.y() < height()) show = true;
  if (!show)
  {
#ifdef QGISDEBUG
    std::cout << "panning: extent out of overview area" << std::endl;
#endif
    mPanningWidget->hide();
    return;
}*/
  
  // round values
  int x1 = static_cast<int>(ur.x()+0.5), x2 = static_cast<int>(ll.x()+0.5);
  int y1 = static_cast<int>(ur.y()+0.5), y2 = static_cast<int>(ll.y()+0.5);
  
  if (x1 > x2)
    std::swap(x1, x2);  
  if (y1 > y2)
    std::swap(y1, y2);
  
  QRect r(x1, y1, x2-x1+1, y2-y1+1);
  
#ifdef QGISDEBUG
  std::cout << "panning: extent to widget: [" << x1 << "," << y1 <<
                "] [" << r.width() << "x" << r.height() << "]" << std::endl;
#endif
    
  mPanningWidget->setGeometry(r);
  mPanningWidget->show(); // show if hidden 
}


void QgsMapOverviewCanvas::mousePressEvent(QMouseEvent * e)
{
//  if (mPanningWidget->isHidden())
//    return;

  // set offset in panning widget if inside it
  // for better experience with panning :)
  if (mPanningWidget->geometry().contains(e->pos()))
  {
    mPanningCursorOffset = e->pos() - mPanningWidget->pos();
  }
  else
  {
    // use center of the panning widget if outside
    QSize s = mPanningWidget->size();
    mPanningCursorOffset = QPoint(s.width()/2, s.height()/2);
  }
  updatePanningWidget(e->pos());
}


void QgsMapOverviewCanvas::mouseReleaseEvent(QMouseEvent * e)
{
//  if (mPanningWidget->isHidden())
//    return;

  if ((e->state() && Qt::LeftButton) == Qt::LeftButton)
  {
    // set new extent
    QgsMapToPixel* cXf = getCoordinateTransform();
    QRect rect = mPanningWidget->geometry();
    
    QgsPoint center = cXf->toMapCoordinates(rect.center());
    QgsRect oldExtent = mMapCanvas->extent();
    QgsRect ext;
    ext.setXmin(center.x() - oldExtent.width()/2);
    ext.setXmax(center.x() + oldExtent.width()/2);
    ext.setYmin(center.y() - oldExtent.height()/2);
    ext.setYmax(center.y() + oldExtent.height()/2);
    
#ifdef QGISDEBUG
  std::cout << "panning: new position: [" << rect.left() << "," << rect.top() <<
                "] [" << rect.width() << "x" << rect.height() << "]" << std::endl;
#endif

    mMapCanvas->setExtent(ext);
    mMapCanvas->clear();
    mMapCanvas->render();
  }
}


void QgsMapOverviewCanvas::mouseMoveEvent(QMouseEvent * e)
{
  // move with panning widget if tracking cursor
  if ((e->state() && Qt::LeftButton) == Qt::LeftButton)
  {
    updatePanningWidget(e->pos());
  }
}


void QgsMapOverviewCanvas::wheelEvent(QWheelEvent * e)
{
}


void QgsMapOverviewCanvas::updatePanningWidget(const QPoint& pos)
{
//  if (mPanningWidget->isHidden())
//    return;
  QSize size = mPanningWidget->size();
  mPanningWidget->move(pos.x() - mPanningCursorOffset.x(), pos.y() - mPanningCursorOffset.y());
}


void QgsMapOverviewCanvas::render(QPaintDevice * theQPaintDevice)
{
  QgsMapCanvas::render(theQPaintDevice);
  reflectChangedExtent();
}
