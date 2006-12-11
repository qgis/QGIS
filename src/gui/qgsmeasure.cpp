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
/* $Id$ */

#include "qgsmeasure.h"

#include "qgscontexthelp.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"

#include <QSettings>
#include <iostream>


QgsMeasure::QgsMeasure(bool measureArea, QgsMapCanvas *mc, Qt::WFlags f)
  : QDialog(mc->topLevelWidget(), f), QgsMapTool(mc)
{
    setupUi(this);
#ifdef Q_WS_MAC
    // Mac buttons are larger than X11 and require a larger minimum width to be drawn correctly
    frame4->setMinimumSize(QSize(224, 0));
#endif
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(restart()));
    connect(mCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    mMeasureArea = measureArea;
    mMapCanvas = mc;
    mTotal = 0.;

    mTable->setLeftMargin(0); // hide row labels

    mTable->horizontalHeader()->setLabel( 0, tr("Segments (in meters)") );
    //mTable->horizontalHeader()->setLabel( 1, tr("Total") );
    //mTable->horizontalHeader()->setLabel( 2, tr("Azimuth") );

    mTable->setColumnStretchable ( 0, true );
    //mTable->setColumnStretchable ( 1, true );
    //mTable->setColumnStretchable ( 2, true );

    updateUi();
    
    connect( mMapCanvas, SIGNAL(renderComplete(QPainter*)), this, SLOT(mapCanvasChanged()) );
    
    mCalc = new QgsDistanceArea;

    mRubberBand = new QgsRubberBand(mMapCanvas, mMeasureArea);

    mCanvas->setCursor(Qt::CrossCursor);

    mRightMouseClicked = false;
}

void QgsMeasure::activate()
{
  restorePosition();
  QgsMapTool::activate();
  mRightMouseClicked = false;
}
    
void QgsMeasure::deactivate()
{
  close();
  QgsMapTool::deactivate();
}


void QgsMeasure::setMeasureArea(bool measureArea)
{
  saveWindowLocation();
  mMeasureArea = measureArea;
  restart();
  restorePosition();
}


QgsMeasure::~QgsMeasure()
{
  delete mCalc;
  delete mRubberBand;
}

void QgsMeasure::restart(void )
{
    mPoints.resize(0);
    mTable->setNumRows(0);
    mTotal = 0.;
    
    updateUi();

    mRubberBand->reset(mMeasureArea);

    mRightMouseClicked = false;
}

void QgsMeasure::addPoint(QgsPoint &point)
{
#ifdef QGISDEBUG
    std::cout << "QgsMeasure::addPoint" << point.x() << ", " << point.y() << std::endl;
#endif

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
      editTotal->setText(formatArea(area));
    }
    else if (!mMeasureArea && mPoints.size() > 1)
    {
      int last = mPoints.size()-2;
        
      QgsPoint p1 = mPoints[last], p2 = mPoints[last+1];
      
      double d = mCalc->measureLine(p1,p2);
            
      mTotal += d;
      editTotal->setText(formatDistance(mTotal));
	
    	mTable->setNumRows ( mPoints.size()-1 );

	    int row = mPoints.size()-2;
      mTable->setText(row, 0, QString::number(d, 'f',1));
      //mTable->setText ( row, 1, QString::number(mTotal) );
      
      mTable->ensureCellVisible(row,0);
    }

    mRubberBand->addPoint(point);
}

void QgsMeasure::mousePress(QgsPoint &point)
{
  if (mPoints.size() == 0)
  {
    addPoint(point);
    this->show();
  }
  raise();

  mouseMove(point);
}

void QgsMeasure::mouseMove(QgsPoint &point)
{
#ifdef QGISDEBUG
    //std::cout << "QgsMeasure::mouseMove" << point.x() << ", " << point.y() << std::endl;
#endif

  mRubberBand->movePoint(point);
  
  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  std::vector<QgsPoint> tmpPoints = mPoints;
  tmpPoints.push_back(point);
  if (mMeasureArea && tmpPoints.size() > 2)
  {
    double area = mCalc->measurePolygon(tmpPoints);
    editTotal->setText(formatArea(area));
  }
  else if (!mMeasureArea && tmpPoints.size() > 1)
  {
    int last = tmpPoints.size()-2;
    QgsPoint p1 = tmpPoints[last], p2 = tmpPoints[last+1];

    double d = mCalc->measureLine(p1,p2);
    editTotal->setText(formatDistance(mTotal + d));
  }
}

void QgsMeasure::mapCanvasChanged()
{
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
  int ww = settings.readNumEntry("/Windows/Measure/w", 150);
  int wh;
  if (mMeasureArea)
    wh = settings.readNumEntry("/Windows/Measure/hNoTable", 70);
  else
    wh = settings.readNumEntry("/Windows/Measure/h", 200);    
  int wx = settings.readNumEntry("/Windows/Measure/x", 100);
  int wy = settings.readNumEntry("/Windows/Measure/y", 100);
//  setUpdatesEnabled(false);
  adjustSize();
  resize(ww,wh);
  move(wx,wy);
//  setUpdatesEnabled(true);
  this->show();
}

void QgsMeasure::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/Windows/Measure/x", p.x());
  settings.writeEntry("/Windows/Measure/y", p.y());
  settings.writeEntry("/Windows/Measure/w", s.width());
  if (mMeasureArea)
    settings.writeEntry("/Windows/Measure/hNoTable", s.height());
  else
    settings.writeEntry("/Windows/Measure/h", s.height());
} 

void QgsMeasure::on_btnHelp_clicked()
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
  if (area < 10000)
  {
    txt = QString::number(area,'f',0);
    txt += " m2";
  }
  else
  {
    txt = QString::number(area/1000000,'f',3);
    txt += " km2";
  }
  return txt;
}

void QgsMeasure::updateUi()
{
  if (mMeasureArea)
  {
    mTable->hide();
    editTotal->setText(formatArea(0));
  }
  else
  {
    mTable->show();
    editTotal->setText(formatDistance(0));
  }
  
}

//////////////////////////

void QgsMeasure::canvasPressEvent(QMouseEvent * e)
{
  if (e->button() == Qt::LeftButton)
  {
    if (mRightMouseClicked)
      restart();

    QgsPoint  idPoint = mCanvas->getCoordinateTransform()->toMapCoordinates(e->x(), e->y());
    mousePress(idPoint);
  }
}


void QgsMeasure::canvasMoveEvent(QMouseEvent * e)
{
  if (!mRightMouseClicked)
    {
      QgsPoint point = mCanvas->getCoordinateTransform()->toMapCoordinates(e->pos().x(), e->pos().y());
      mouseMove(point);
    }
}


void QgsMeasure::canvasReleaseEvent(QMouseEvent * e)
{
  QgsPoint point = mCanvas->getCoordinateTransform()->toMapCoordinates(e->x(), e->y());

  if(e->button() == Qt::RightButton && (e->state() & Qt::LeftButton) == 0) // restart
  {
    if (mRightMouseClicked)
      restart();
    else
      mRightMouseClicked = true;
  } 
  else if (e->button() == Qt::LeftButton)
  {
    addPoint(point);
    show();
  }

}
