/***************************************************************************
  qgs3dcameracontrolswidget.h
  --------------------------------------
  Date                 : February 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DCAMERACONTROLSWIDGET_H
#define QGS3DCAMERACONTROLSWIDGET_H

class QStandardItemModel;
class Qgs3DMapCanvas;

#include "ui_qgs3dcameracontrolswidget.h"

#include "qgis_app.h"
#include "qgs3dmapsettings.h"

class APP_EXPORT Qgs3DCameraControlsWidget : public QWidget, Ui::Qgs3DCameraControlsWidget
{
    Q_OBJECT
  public:
    explicit Qgs3DCameraControlsWidget( Qgs3DMapCanvas *canvas, QWidget *parent = nullptr );

  public slots:
    void updateFromCamera() const;

  private:
    void setCRSInfo() const;
    void updateCameraLookingAt();

    Qgs3DMapCanvas *m3DMapCanvas = nullptr;
};

#endif // QGS3DCAMERACONTROLSWIDGET_H
