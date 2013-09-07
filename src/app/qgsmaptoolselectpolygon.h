/***************************************************************************
qgsmaptoolselectpolygon.h  -  map tool for selecting features by polygon
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPTOOLSELECTPOLYGON_H
#define QGSMAPTOOLSELECTPOLYGON_H

#include "qgsmaptool.h"

class QgsMapCanvas;
class QgsRubberBand;


class APP_EXPORT QgsMapToolSelectPolygon : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectPolygon( QgsMapCanvas* canvas );

    virtual ~QgsMapToolSelectPolygon();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

  private:

    //! used for storing all of the maps point for the polygon
    QgsRubberBand* mRubberBand;
};

#endif
