/***************************************************************************
     qgsgeoreftoolmovepoint.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOREFTOOLMOVEPOINT_H
#define QGSGEOREFTOOLMOVEPOINT_H

#include "qgsmaptool.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"

class QgsGeorefToolMovePoint : public QgsMapTool
{
    Q_OBJECT

  public:
    explicit QgsGeorefToolMovePoint( QgsMapCanvas *canvas );

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    bool isCanvas( QgsMapCanvas * );

    QgsPointXY startPoint() const { return mStartPointMapCoords; }
    void setStartPoint( const QgsPointXY &startPoint ) { mStartPointMapCoords = startPoint; }

  signals:
    void pointBeginMove( const QgsPointXY &p );
    void pointMoving( const QgsPointXY &p );
    void pointEndMove( const QgsPointXY &p );
    void pointCancelMove( const QgsPointXY &p );

  private:
    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
};

#endif // QGSGEOREFTOOLMOVEPOINT_H
