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

#include "qgspoint.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmeasure.h"
#include "qgscontexthelp.h"

QgsMeasure::QgsMeasure( QgsMapCanvas *mc, QWidget *parent, const char * name, WFlags f)
           :QgsMeasureBase( parent, name, f)
{
    mMapCanvas = mc;
    mDynamic = false;
    mPixmap = mMapCanvas->canvasPixmap();
    mTotal = 0.;

    mTable->setLeftMargin(0); // hide row labels

    mTable->horizontalHeader()->setLabel( 0, tr("Increment") );
    mTable->horizontalHeader()->setLabel( 1, tr("Total") );
    //mTable->horizontalHeader()->setLabel( 2, tr("Azimuth") );

    mTable->setColumnStretchable ( 0, true );
    mTable->setColumnStretchable ( 1, true );
    //mTable->setColumnStretchable ( 2, true );

    connect ( mMapCanvas, SIGNAL(renderComplete(QPainter*)), this, SLOT(draw(QPainter*)) );
    restorePosition();
}

QgsMeasure::~QgsMeasure()
{
}

void QgsMeasure::restart(void )
{
    // Delete old line
    drawLine();
    
    mPoints.resize(0);
    mTable->setNumRows(0);
    mTotal = 0.;

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
    drawLine();

    QgsPoint pnt(point);
    mPoints.push_back(pnt);

    if ( mPoints.size() > 1 ) {
        int last = mPoints.size()-2;
	double dx = point.x() - mPoints[last].x();
	double dy = point.y() - mPoints[last].y();
	double d = std::sqrt( dx*dx + dy*dy );
        mTotal += d;
	
	mTable->setNumRows ( mPoints.size()-1 );

	int row = mPoints.size()-2;
        mTable->setText ( row, 0, QString::number(d) );
        mTable->setText ( row, 1, QString::number(mTotal) );
	
    }

    // Draw new line
    drawLine();
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

void QgsMeasure::drawLine(void)
{
#ifdef QGISDEBUG
    std::cout << "QgsMeasure::drawLine" << std::endl;
#endif

// TODO: Qt4 uses "QRubberBand"s - need to refactor.
#if QT_VERSION < 0x040000
    QPainter p;
    p.begin(mPixmap);
    QPen pen(Qt::gray);
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
    QPen pen(Qt::gray);
    p.setPen(pen);
    p.setRasterOp(Qt::XorROP);

    QgsMapToPixel *trans = mMapCanvas->getCoordinateTransform();
    QgsPoint ppnt = trans->transform(mDynamicPoints[0]);
    p.moveTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
    ppnt = trans->transform(mDynamicPoints[1]);
    p.lineTo(static_cast<int>(ppnt.x()), static_cast<int>(ppnt.y()));
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
  int ww = settings.readNumEntry("/qgis/Windows/Measure/w", 250);
  int wh = settings.readNumEntry("/qgis/Windows/Measure/h", 300);
  int wx = settings.readNumEntry("/qgis/Windows/Measure/x", 100);
  int wy = settings.readNumEntry("/qgis/Windows/Measure/y", 100);
  resize(ww,wh);
  move(wx,wy);
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
  settings.writeEntry("/qgis/Windows/Measure/h", s.height());
} 

void QgsMeasure::showHelp()
{
  QgsContextHelp::run(context_id);
}

