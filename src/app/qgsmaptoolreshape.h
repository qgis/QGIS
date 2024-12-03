/***************************************************************************
    qgsmaptoolreshape.h
    ---------------------
    begin                : Juli 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco.hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLRESHAPE_H
#define QGSMAPTOOLRESHAPE_H

#include "qgsmaptoolcapture.h"
#include "qgis_app.h"

//! A map tool that draws a line and splits the features cut by the line
class APP_EXPORT QgsMapToolReshape : public QgsMapToolCapture
{
    Q_OBJECT

  public:
    QgsMapToolReshape( QgsMapCanvas *canvas );
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;

  private:
    void reshape( QgsVectorLayer *vlayer );

    bool isBindingLine( QgsVectorLayer *vlayer, const QgsRectangle &bbox ) const;

    friend class TestQgsMapToolReshape;
};

#endif
