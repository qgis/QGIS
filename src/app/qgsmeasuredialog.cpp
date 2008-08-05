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
#include "qgsmaprenderer.h"
#include "qgsspatialrefsys.h"

#include <QCloseEvent>
#include <QLocale>
#include <QSettings>


QgsMeasureDialog::QgsMeasureDialog(QgsMeasureTool* tool, Qt::WFlags f)
  : QDialog(tool->canvas()->topLevelWidget(), f), mTool(tool)
{
    setupUi(this);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(restart()));
    connect(mCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    mMeasureArea = tool->measureArea();
    mTotal = 0.;

    // Set one cell row where to update current distance
    // If measuring area, the table doesn't get shown
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(QString::number(0, 'f', 1)));
    item->setTextAlignment(0, Qt::AlignRight);
    mTable->addTopLevelItem(item);

    //mTable->setHeaderLabels(QStringList() << tr("Segments (in meters)") << tr("Total") << tr("Azimuth") );

    updateUi();
}


void QgsMeasureDialog::restart()
{
    mTool->restart();  
  
    // Set one cell row where to update current distance
    // If measuring area, the table doesn't get shown
    mTable->clear();
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(QString::number(0, 'f', 1)));
    item->setTextAlignment(0, Qt::AlignRight);
    mTable->addTopLevelItem(item);
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
    QTreeWidgetItem *item = mTable->topLevelItem(mTable->topLevelItemCount()-1);
    item->setText(0, QLocale::system().toString(d, 'f', 2));
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

      QTreeWidgetItem *item = mTable->topLevelItem(mTable->topLevelItemCount()-1);
      item->setText(0, QLocale::system().toString(d, 'f', 2));

      item = new QTreeWidgetItem(QStringList(QLocale::system().toString(0.0, 'f', 2)));
      item->setTextAlignment(0, Qt::AlignRight);
      mTable->addTopLevelItem(item);
      mTable->scrollToItem(item);
    }
}


void QgsMeasureDialog::close(void)
{
    restart();
    QDialog::close();
}

void QgsMeasureDialog::closeEvent(QCloseEvent *e)
{
    saveWindowLocation();
    e->accept();
}

void QgsMeasureDialog::restorePosition()
{
  QSettings settings;
  restoreGeometry(settings.value("/Windows/Measure/geometry").toByteArray());
  int wh;
  if (mMeasureArea)
    wh = settings.value("/Windows/Measure/hNoTable", 70).toInt();
  else
    wh = settings.value("/Windows/Measure/h", 200).toInt();    
  resize(width(), wh);
  updateUi();
  this->show();
}

void QgsMeasureDialog::saveWindowLocation()
{
  QSettings settings;
  settings.setValue("/Windows/Measure/geometry", saveGeometry());
  const QString &key = mMeasureArea ? "/Windows/Measure/hNoTable" : "/Windows/Measure/h";
  settings.setValue(key, height());
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
      mTable->setHeaderLabels( QStringList( tr("Segments (in meters)") ) );
      break;
    case QGis::FEET:
      mTable->setHeaderLabels( QStringList( tr("Segments (in feet)") ) );
      break;
    case QGis::DEGREES:
      mTable->setHeaderLabels( QStringList( tr("Segments (in degrees)") ) );
      break;
    case QGis::UNKNOWN:
      mTable->setHeaderLabels( QStringList( tr("Segments") ) );
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

