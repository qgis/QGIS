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

#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"

#include "qgscontexthelp.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsmaprender.h"
#include "qgsspatialrefsys.h"

#include <QSettings>
#include <QLocale>


QgsMeasureDialog::QgsMeasureDialog(QgsMeasureTool* tool, Qt::WFlags f)
  : QDialog(tool->canvas()->topLevelWidget(), f), mTool(tool)
{
    setupUi(this);
#ifdef Q_WS_MAC
    // Mac buttons are larger than X11 and require a larger minimum width to be drawn correctly
    frame4->setMinimumSize(QSize(224, 0));
#endif
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(restart()));
    connect(mCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    mMeasureArea = tool->measureArea();
    mTotal = 0.;

    mTable->setLeftMargin(0); // hide row labels

    // Set one cell row where to update current distance
    // If measuring area, the table doesn't get shown
    mTable->setNumRows(1);
    mTable->setText(0, 0, QString::number(0, 'f',1));

    //mTable->horizontalHeader()->setLabel( 0, tr("Segments (in meters)") );
    //mTable->horizontalHeader()->setLabel( 1, tr("Total") );
    //mTable->horizontalHeader()->setLabel( 2, tr("Azimuth") );

    mTable->setColumnStretchable ( 0, true );
    //mTable->setColumnStretchable ( 1, true );
    //mTable->setColumnStretchable ( 2, true );

    updateUi();
}


void QgsMeasureDialog::restart()
{
    mTool->restart();  
  
    // Set one cell row where to update current distance
    // If measuring area, the table doesn't get shown
    mTable->setNumRows(1);
    mTable->setText(0, 0, QString::number(0, 'f',1));
    mTotal = 0.;
    
    updateUi();
}


void QgsMeasureDialog::mousePress(QgsPoint &point)
{
  if (mTool->points().size() == 0)
  {
    addPoint(point);
    this->show();
  }
  raise();

  mouseMove(point);
}

void QgsMeasureDialog::mouseMove(QgsPoint &point)
{
  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  QList<QgsPoint> tmpPoints = mTool->points();
  tmpPoints.append(point);
  if (mMeasureArea && tmpPoints.size() > 2)
  {
    double area = mTool->canvas()->mapRender()->distArea()->measurePolygon(tmpPoints);
    editTotal->setText(formatArea(area));
  }
  else if (!mMeasureArea && tmpPoints.size() > 1)
  {
    int last = tmpPoints.size()-2;
    QgsPoint p1 = tmpPoints[last], p2 = tmpPoints[last+1];

    double d = mTool->canvas()->mapRender()->distArea()->measureLine(p1,p2);
    //mTable->setText(last, 0, QString::number(d, 'f',1));
    mTable->setText(last, 0, QLocale::system().toString(d, 'f', 2));
    editTotal->setText(formatDistance(mTotal + d));
  }
}

void QgsMeasureDialog::addPoint(QgsPoint &point)
{
    int numPoints = mTool->points().size();
    if (mMeasureArea && numPoints > 2)
    {
      double area = mTool->canvas()->mapRender()->distArea()->measurePolygon(mTool->points());
      editTotal->setText(formatArea(area));
    }
    else if (!mMeasureArea && numPoints > 1)
    {
      int last = numPoints-2;
        
      QgsPoint p1 = mTool->points()[last], p2 = mTool->points()[last+1];
      
      double d = mTool->canvas()->mapRender()->distArea()->measureLine(p1,p2);
            
      mTotal += d;
      editTotal->setText(formatDistance(mTotal));
	

      int row = numPoints-2;
      mTable->setText(row, 0, QLocale::system().toString(d, 'f', 2));
      mTable->setNumRows ( numPoints );
      
      mTable->setText(row + 1, 0, QLocale::system().toString(0.0, 'f', 2));
      mTable->ensureCellVisible(row + 1,0);
    }
}


void QgsMeasureDialog::close(void)
{
    restart();
    saveWindowLocation();
    hide();
}

void QgsMeasureDialog::closeEvent(QCloseEvent *e)
{
    saveWindowLocation();
    e->accept();
}

void QgsMeasureDialog::restorePosition()
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
  updateUi();
  this->show();
}

void QgsMeasureDialog::saveWindowLocation()
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

void QgsMeasureDialog::on_btnHelp_clicked()
{
  QgsContextHelp::run(context_id);
}


QString QgsMeasureDialog::formatDistance(double distance)
{
  QString txt;
  QString unitLabel;

  QGis::units myMapUnits = mTool->canvas()->mapUnits();
  return QgsDistanceArea::textUnit(distance, 2, myMapUnits, false);
}

QString QgsMeasureDialog::formatArea(double area)
{
  QGis::units myMapUnits = mTool->canvas()->mapUnits();
  return QgsDistanceArea::textUnit(area, 2, myMapUnits, true);
}

void QgsMeasureDialog::updateUi()
{
  
  QGis::units myMapUnits = mTool->canvas()->mapUnits();
  switch (myMapUnits)
  {
    case QGis::METERS: 
      mTable->horizontalHeader()->setLabel( 0, tr("Segments (in meters)") );
      break;
    case QGis::FEET:
      mTable->horizontalHeader()->setLabel( 0, tr("Segments (in feet)") );
      break;
    case QGis::DEGREES:
      mTable->horizontalHeader()->setLabel( 0, tr("Segments (in degrees)") );
      break;
    case QGis::UNKNOWN:
      mTable->horizontalHeader()->setLabel( 0, tr("Segments") );
  };

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

