/***************************************************************************
    qgsmaptoolmeasureangle.h
    ------------------------
    begin                : December 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLMEASUREANGLE_H
#define QGSMAPTOOLMEASUREANGLE_H

#include "qgsmaptool.h"
#include "qgsmapcanvassnapper.h"
#include "qgspoint.h"

class QgsDisplayAngle;
class QgsRubberBand;

/**Map tool to measure angle between two segments*/
class QgsMapToolMeasureAngle: public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolMeasureAngle( QgsMapCanvas* canvas );
    ~QgsMapToolMeasureAngle();

    //! Mouse move event for overriding
    void canvasMoveEvent( QMouseEvent * e );

    //! Mouse release event for overriding
    void canvasReleaseEvent( QMouseEvent * e );

    //! called when set as currently active map tool
    void activate();

    //! called when map tool is being deactivated
    void deactivate();

  private:
    /**Points defining the angle (three for measuring)*/
    QList<QgsPoint> mAnglePoints;
    QgsRubberBand* mRubberBand;
    QgsDisplayAngle* mResultDisplay;
    QgsMapCanvasSnapper mSnapper;

    /**Creates a new rubber band and deletes the old one*/
    void createRubberBand();
    /**Snaps point to background layers*/
    QgsPoint snapPoint( const QPoint& p );

  private slots:
    /**Deletes the rubber band and the dialog*/
    void stopMeasuring();

};

#endif // QGSMAPTOOLMEASUREANGLE_H
