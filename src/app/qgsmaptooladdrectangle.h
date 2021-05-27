/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDRECTANGLE_H
#define QGSMAPTOOLADDRECTANGLE_H

#include "qgsmaptooladdabstract.h"
#include "qgspolygon.h"
#include "qgsquadrilateral.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolAddRectangle: public QgsMapToolAddAbstract
{
    Q_OBJECT

  public:
    QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void deactivate( ) override;
    void clean() override;

  protected:
    explicit QgsMapToolAddRectangle( QgsMapCanvas *canvas ) = delete; //forbidden

    //! Rectangle
    QgsQuadrilateral mRectangle;
};

#endif // QGSMAPTOOLADDRECTANGLE_H
