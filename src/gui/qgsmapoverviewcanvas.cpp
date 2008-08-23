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

#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaptopixel.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

//! widget that serves as rectangle showing current extent in overview
class QgsPanningWidget : public QWidget
{
public:
  QgsPanningWidget(QWidget* parent)
  : QWidget(parent)
  {
    setObjectName("panningWidget");
    setMinimumSize(5,5);
    setAttribute(Qt::WA_NoSystemBackground);
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
  : QWidget(parent), mMapCanvas(mapCanvas)
{
  setObjectName("theOverviewCanvas");
  mPanningWidget = new QgsPanningWidget(this);
  
  mMapRenderer = new QgsMapRenderer;
  mMapRenderer->setOverview();
  
  setbgColor(palette().window().color());
}

QgsMapOverviewCanvas::~QgsMapOverviewCanvas()
{
  delete mMapRenderer;
}

void QgsMapOverviewCanvas::resizeEvent(QResizeEvent* e)
{
  mPixmap = QPixmap(e->size());
  mMapRenderer->setOutputSize(e->size(), mPixmap.logicalDpiX());
  refresh();
}


void QgsMapOverviewCanvas::reflectChangedExtent()
{
  const QgsRect& extent = mMapCanvas->extent();
  
  // show only when valid extent is set
  if (extent.isEmpty())
  {
    mPanningWidget->hide();
    return;
  }
  
  const QgsMapToPixel* cXf = mMapRenderer->coordinateTransform();
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
  
#ifdef Q_WS_MAC
  // setGeometry (Qt 4.2) is causing Mac window corruption (decorations
  // are drawn at odd locations) if both coords are at limit. This may
  // have something to do with Qt calculating dimensions as x2 - x1 + 1.
  // (INT_MAX - INT_MIN + 1 is UINT_MAX + 1)
  if (x1 == INT_MIN && x2 == INT_MAX) x1 += 1;  // x2 -= 1 works too
  if (y1 == INT_MIN && y2 == INT_MAX) y1 += 1;
#endif

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

  if (e->button() == Qt::LeftButton)
  {
    // set new extent
    const QgsMapToPixel* cXf = mMapRenderer->coordinateTransform();
    QRect rect = mPanningWidget->geometry();
    
    QgsPoint center = cXf->toMapCoordinates(rect.center());
    QgsRect oldExtent = mMapCanvas->extent();
    QgsRect ext;
    ext.setXMinimum(center.x() - oldExtent.width()/2);
    ext.setXMaximum(center.x() + oldExtent.width()/2);
    ext.setYmin(center.y() - oldExtent.height()/2);
    ext.setYmax(center.y() + oldExtent.height()/2);
    
#ifdef QGISDEBUG
  std::cout << "panning: new position: [" << rect.left() << "," << rect.top() <<
                "] [" << rect.width() << "x" << rect.height() << "]" << std::endl;
#endif

    mMapCanvas->setExtent(ext);
    mMapCanvas->refresh();
  }
}


void QgsMapOverviewCanvas::mouseMoveEvent(QMouseEvent * e)
{
  // move with panning widget if tracking cursor
  if ((e->buttons() & Qt::LeftButton) == Qt::LeftButton)
  {
    updatePanningWidget(e->pos());
  }
}


void QgsMapOverviewCanvas::updatePanningWidget(const QPoint& pos)
{
//  if (mPanningWidget->isHidden())
//    return;
  QSize size = mPanningWidget->size();
  mPanningWidget->move(pos.x() - mPanningCursorOffset.x(), pos.y() - mPanningCursorOffset.y());
}


void QgsMapOverviewCanvas::paintEvent(QPaintEvent * pe)
{
  QPainter paint(this);
  paint.drawPixmap(pe->rect().topLeft(), mPixmap, pe->rect());
}


void QgsMapOverviewCanvas::refresh()
{
  mPixmap.fill(mBgColor); //palette().color(backgroundRole());

  QPainter painter;
  painter.begin(&mPixmap);
  
  // antialiasing
  if (mAntiAliasing)
    painter.setRenderHint(QPainter::Antialiasing);
  
  // render image
  mMapRenderer->render(&painter);
  
  painter.end();
  
  // schedule repaint
  update();
  
  // update panning widget
  reflectChangedExtent();
}


void QgsMapOverviewCanvas::setbgColor(const QColor& color)
{
  mBgColor = color;
  
  // set erase color
  QPalette palette;
  palette.setColor(backgroundRole(), color);
  setPalette(palette);
}

void QgsMapOverviewCanvas::setLayerSet(const QStringList& layerSet)
{
  mMapRenderer->setLayerSet(layerSet);
}

void QgsMapOverviewCanvas::updateFullExtent(const QgsRect& rect)
{
  mMapRenderer->setExtent(rect);
  reflectChangedExtent();
}

void QgsMapOverviewCanvas::projectionsEnabled(bool flag)
{
  mMapRenderer->setProjectionsEnabled(flag);
}

void QgsMapOverviewCanvas::destinationSrsChanged()
{
  const QgsCoordinateReferenceSystem& srs = mMapCanvas->mapRenderer()->destinationSrs();
  mMapRenderer->setDestinationSrs(srs);
}

QStringList& QgsMapOverviewCanvas::layerSet()
{
  return mMapRenderer->layerSet();
}
