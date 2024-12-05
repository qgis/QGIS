/***************************************************************************
  qgs3ddebugwidget.h
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DDEBUGWIDGET_H
#define QGS3DDEBUGWIDGET_H

class QStandardItemModel;
class Qgs3DMapCanvas;

#include <ui_3ddebugwidget.h>
#include "qgs3dmapsettings.h"
#include "qgis_app.h"

class APP_EXPORT Qgs3DDebugWidget : public QWidget, Ui::Q3DDebugWidget
{
    Q_OBJECT
  public:
    explicit Qgs3DDebugWidget( Qgs3DMapCanvas *canvas, QWidget *parent = nullptr );

    /**
    * Sets the map settings and necessary connections based on Qgs3DMapSettings.
    * \note We need separate function as Qgs3DMapCanvasWidget, which wraps this widget, also gets it later.
    */
    void setMapSettings( Qgs3DMapSettings *mapSettings );

  public slots:

    /**
    * Function updates the camera info values when the user moves in the scene.
    */
    void updateFromCamera() const;

  private:
    Qgs3DMapSettings *mMap = nullptr;
    Qgs3DMapCanvas *m3DMapCanvas = nullptr;
};

#endif // QGS3DDEBUGWIDGET_H
