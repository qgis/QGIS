/***************************************************************************
    qgsmaptooladdcircle.h  -  map tool for adding circle
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

#ifndef QGSMAPTOOLADDCIRCLE_H
#define QGSMAPTOOLADDCIRCLE_H

#include "qgsmaptooladdabstract.h"
#include "qgscircle.h"
#include "qgis_app.h"

struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match &m ) override { return m.hasEdge(); }
};

class APP_EXPORT QgsMapToolAddCircle: public QgsMapToolAddAbstract
{
    Q_OBJECT

  public:
    QgsMapToolAddCircle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void deactivate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddCircle( QgsMapCanvas *canvas ) = delete; //forbidden

    //! Circle
    QgsCircle mCircle;

};

#endif // QGSMAPTOOLADDCIRCLE_H
