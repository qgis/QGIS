/***************************************************************************
  qgs3dmapcanvasdockwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVASDOCKWIDGET_H
#define QGS3DMAPCANVASDOCKWIDGET_H

#include "qgsdockwidget.h"

class Qgs3DMapCanvas;
class QgsMapCanvas;

class Qgs3DMapSettings;


class Qgs3DMapCanvasDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvasDockWidget( QWidget *parent = nullptr );

    //! takes ownership
    void setMapSettings( Qgs3DMapSettings *map );

    void setMainCanvas( QgsMapCanvas *canvas );

    Qgs3DMapCanvas *mapCanvas3D() { return mCanvas; }

  private slots:
    void resetView();
    void configure();

    void onMainCanvasLayersChanged();
    void onMainCanvasColorChanged();

  private:
    Qgs3DMapCanvas *mCanvas = nullptr;
    QgsMapCanvas *mMainCanvas = nullptr;
};

#endif // QGS3DMAPCANVASDOCKWIDGET_H
