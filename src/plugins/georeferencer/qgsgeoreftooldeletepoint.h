/***************************************************************************
     qgsgeoreftooldeletepoint.h
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

#ifndef QGSGEOREFTOOLDELETEPOINT_H
#define QGSGEOREFTOOLDELETEPOINT_H

#include <QMouseEvent>

#include "qgsmaptoolemitpoint.h"

class QgsPoint;
class QgsMapCanvas;

class QgsGeorefToolDeletePoint : public QgsMapToolEmitPoint
{
    Q_OBJECT

  public:
    QgsGeorefToolDeletePoint( QgsMapCanvas* canvas );

    // Mouse events for overriding
    void canvasPressEvent( QMouseEvent * e ) override;

  signals:
    void deleteDataPoint( const QPoint & );
};

#endif // QGSGEOREFTOOLDELETEPOINT_H
