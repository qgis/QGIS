/***************************************************************************
                                 qgsmeasure.h
                               ------------------
        begin                : March 2005
        copyright            : (C) 2005 by Radim Blazek
        email                : blazek@itc.it 
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include <qtable.h>
#include <qsettings.h>
#include <qevent.h>
#include <qsize.h>
#include <qpoint.h>
#include <qpen.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qglobal.h>
#include <qlabel.h>

#include "qgspoint.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmeasure.h"
#include "qgscontexthelp.h"

#include "qgsdistancearea.h"



QgsMeasure::QgsMeasure(bool measureArea, QgsMapCanvas *mc, QWidget *parent, const char * name, WFlags f)
           :QgsMeasureBase( parent, name, f)
{
    mMeasureArea = measureArea;
    mMapCanvas = mc;
    mDynamic = false;
    mPixmap = mMapCanvas->canvasPixmap();
    mTotal = 0.;

    mTable->setLeftMargin(0); // hide row labels

    mTable->horizontalHeader()->setLabel( 0, tr("Segments (in meters)") );
    //mTable->horizontalHeader()->setLabel( 1, tr("Total") );
    //mTable->horizontalHeader()->setLabel( 2, tr("Azimuth") );

    mTable->setColumnStretchable ( 0, true );
    //mTable->setColumnStretchable ( 1, true );
    //mTable->setColumnStretchable ( 2, true );
    
    updateUi();
    
    connect ( mMapCanvas, SIGNAL(renderComplete(QPainter*)), this, SLOT(draw(QPainter*)) );
    restorePosition();
    
    mCalc = new QgsDistanceArea;
    
}


void QgsMeasure::setMeasureArea(bool measureArea)
{
  saveWindowLocation();
  restart();
  mMeasureArea = measureArea;
  updateUi();
  restorePosition();
}


QgsMeasure::~QgsMeasure()
{
  delete mCalc;
}

void QgsMeasure::restart(void )
{
    // Delete old line
    drawLine();
    
    mPoints.resize(0);
    mTable->setNumRows(0);
    mTotal = 0.;
    
    updateUi();
        
    if ( mDynamic ) { 
	drawDynamicLine();
        mDynamic = false;
    }
    
}

void QgsMeasure::addPoint(QgsPoint &point)
{
#ifdef QGISDEBUG
    std::cout << "QgsMeasure::addPoint" << point.x() << ", " << point.y() << std::endl;
#endif

    // Delete dynamic
    if ( mDynamic ) { 
	drawDynamicLine();
        mDynamic = false;
    }

    // Delete old line
    drawLine(true);

    // don't add points with the same coordinates
    if (mPoints.size() > 0 && point == mPoints[0])
      return;
    
    QgsPoint pnt(point);
    mPoints.push_back(pnt);
    
    if (mPoints.size() == 1)
    {
      // ensure that we have correct settings
      mCalc->setDefaultEllipsoid();
      mCalc->setProjectAsSourceSRS();
    }

    if (mMeasureArea && mPoints.size() > 2)
    {
      double area = mCalc->measurePolygon(mPoints);
      lblTotal->setText(formatArea(area));
    }
    else if (!mMeasureArea && mPoints.size() > 1)
    {
      int last = mPoints.size()-2;
        
      QgsPoint p1 = mPoints[last], p2 = mPoints[last+1];
      
      double d = mCalc->measureLine(p1,p2);
            
      mTotal += d;
      lblTotal->setText(formatDistance(mTotal));
	
    	mTable->setNumRows ( mPoints.size()-1 );

	    int row = mPoints.size()-2;
      mTable->setText(row, 0, QString::number(d, 'f',1));
      //mTable->setText ( row, 1, QString::number(mTotal) );
      
      mTable->ensureCellVisible(row,0);
    }

    // Draw new line
    drawLine();
}

void QgsMeasure::mousePress(QgsPoint &point)
{
  if (mPoints.size() == 0)
  {
    addPoint(point);
    show();
  }
  
  mouseMove(point);

  if (mMeasureArea && mPoints.size() > 2)
  {
    // delete old line which connect 1. and last point
    
 // TODO: Qt4 uses "QRubberBand"s - need to refactor.
#if QT_VERSION < 0x040000
    QPainter p;
    p.begin(mPixmap);
    QPen pen(Qt::gray, 2);
    p.setPen(pen);
    p.setRasterOp(Qt::XorROP);

    QgsMapToPixel *trans = mMapCanvas->getCoordinateTransform();
    QgsPoint ppnt = trans->transform(mPoints[mPoints.size()-1]);
    p.moveTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    ppnt = trans->transform(mPoints[0]);
    p.lineTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    p.end();
#endif
    mMapCanvas->repaint(false);
  }
  
}

void QgsMeasure::mouseMove(QgsPoint &point)
{
#ifdef QGISDEBUG
    //std::cout << "QgsMeasure::mouseMove" << point.x() << ", " << point.y() << std::endl;
#endif

    if ( mDynamic ) { 
	drawDynamicLine(); // delete old
    }
    
    if ( mPoints.size() > 0 ) {
	mDynamicPoints[0] = mPoints[mPoints.size()-1];
	mDynamicPoints[1] = point;
	drawDynamicLine();
	mDynamic = true;
    }
}

void QgsMeasure::draw(QPainter *p)
{
#ifdef QGISDEBUG
    std::cout << "QgsMeasure::draw" << std::endl;
#endif

    drawLine();
    mDynamic = false;
}

void QgsMeasure::drawLine(bool erase)
{
#ifdef QGISDEBUG
    std::cout << "QgsMeasure::drawLine" << std::endl;
#endif

// TODO: Qt4 uses "QRubberBand"s - need to refactor.
#if QT_VERSION < 0x040000
    QPainter p;
    p.begin(mPixmap);
    QPen pen(Qt::gray, 2);
    p.setPen(pen);
    p.setRasterOp(Qt::XorROP);

    QgsMapToPixel *trans = mMapCanvas->getCoordinateTransform();
    for ( int i = 0; i < mPoints.size(); i++ ) {
        QgsPoint ppnt = trans->transform(mPoints[i]);
	if ( i == 0 ) {
	    p.moveTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
	} else {
	    p.lineTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
	}
    }
    
    if (!erase && mMeasureArea && mPoints.size() > 2) // draw the last point of the polygon
    {
      QgsPoint ppnt = trans->transform(mPoints[0]);
      p.lineTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    }
    
    p.end();
#endif
    mMapCanvas->repaint(false);
}

void QgsMeasure::drawDynamicLine( void )
{
#ifdef QGISDEBUG
    //std::cout << "QgsMeasure::drawDynamicLine" << std::endl;
#endif

// TODO: Qt4 uses "QRubberBand"s and "QPainterPath"s - need to refactor.
#if QT_VERSION < 0x040000
    QPainter p;
    p.begin(mPixmap);
    QPen pen(Qt::gray, 2);
    p.setPen(pen);
    p.setRasterOp(Qt::XorROP);

    QgsMapToPixel *trans = mMapCanvas->getCoordinateTransform();
    QgsPoint ppnt = trans->transform(mDynamicPoints[0]);
    p.moveTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    QgsPoint ppnt2 = trans->transform(mDynamicPoints[1]);
    p.lineTo(static_cast<int>(ppnt2.x()), static_cast<int>(ppnt2.y()));
    
    if (mMeasureArea && mPoints.size() >= 2)
    {
      ppnt = trans->transform(mPoints[0]);
      p.lineTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    }
    
    p.end();
#endif
    mMapCanvas->repaint(false);
}

void QgsMeasure::close(void)
{
    restart();
    saveWindowLocation();
    hide();
}

void QgsMeasure::closeEvent(QCloseEvent *e)
{
    saveWindowLocation();
    e->accept();
}

void QgsMeasure::restorePosition()
{
  QSettings settings;
  int ww = settings.readNumEntry("/qgis/Windows/Measure/w", 150);
  int wh;
  if (mMeasureArea)
    wh = settings.readNumEntry("/qgis/Windows/Measure/hNoTable", 70);
  else
    wh = settings.readNumEntry("/qgis/Windows/Measure/h", 200);    
  int wx = settings.readNumEntry("/qgis/Windows/Measure/x", 100);
  int wy = settings.readNumEntry("/qgis/Windows/Measure/y", 100);
//  setUpdatesEnabled(false);
  adjustSize();
  resize(ww,wh);
  move(wx,wy);
//  setUpdatesEnabled(true);
  QgsMeasureBase::show();
}

void QgsMeasure::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/qgis/Windows/Measure/x", p.x());
  settings.writeEntry("/qgis/Windows/Measure/y", p.y());
  settings.writeEntry("/qgis/Windows/Measure/w", s.width());
  if (mMeasureArea)
    settings.writeEntry("/qgis/Windows/Measure/hNoTable", s.height());
  else
    settings.writeEntry("/qgis/Windows/Measure/h", s.height());
} 

void QgsMeasure::showHelp()
{
  QgsContextHelp::run(context_id);
}


QString QgsMeasure::formatDistance(double distance)
{
  QString txt;
  if (distance < 1000)
  {
    txt = QString::number(distance,'f',0);
    txt += " m";
  }
  else
  {
    txt = QString::number(distance/1000,'f',1);
    txt += " km";
  }
  return txt;
}

QString QgsMeasure::formatArea(double area)
{
  QString txt;
  if (area < 1000)
  {
    txt = QString::number(area,'f',0);
    txt += " m<sup>2</sup>";
  }
  else
  {
    txt = QString::number(area/1000000,'f',3);
    txt += " km<sup>2</sup>";
  }
  return txt;
}

void QgsMeasure::updateUi()
{
  if (mMeasureArea)
  {
    mTable->hide();
    lblTotal->setText(formatArea(0));
  }
  else
  {
    mTable->show();
    lblTotal->setText(formatDistance(0));
  }
  
}
