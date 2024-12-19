/***************************************************************************
    qgsmaptoolsplitfeatures.h
    ---------------------
    begin                : August 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSPLITFEATURES_H
#define QGSMAPTOOLSPLITFEATURES_H

#include "qgsmaptoolcapture.h"
#include "qgis_app.h"

//! A map tool that draws a line and splits the features cut by the line
class APP_EXPORT QgsMapToolSplitFeatures: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolSplitFeatures( QgsMapCanvas *canvas );
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
};

#endif
