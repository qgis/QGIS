/***************************************************************************
                          qgsmaptoolchangelabelproperties.h
                          ---------------------------------
    begin                : 2010-11-11
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCHANGELABELPROPERTIES_H
#define QGSMAPTOOLCHANGELABELPROPERTIES_H

#include "qgsmaptoollabel.h"

class QgsMapToolChangeLabelProperties: public QgsMapToolLabel
{
  public:
    QgsMapToolChangeLabelProperties( QgsMapCanvas* canvas );
    ~QgsMapToolChangeLabelProperties();

    virtual void canvasPressEvent( QMouseEvent * e );
    virtual void canvasReleaseEvent( QMouseEvent * e );

};

#endif // QGSMAPTOOLCHANGELABEL_H
