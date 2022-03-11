/***************************************************************************
    qgsmaptoolsplitparts.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSPLITPARTS_H
#define QGSMAPTOOLSPLITPARTS_H

#include "qgsmaptoolcapture.h"

//! A map tool that draws a line and splits the parts cut by the line
class APP_EXPORT QgsMapToolSplitParts: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolSplitParts( QgsMapCanvas *canvas );
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
};

#endif
