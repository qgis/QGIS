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

#include "qgis_app.h"
#include "qgsdockwidget.h"

#include <QDialog>
#include <QVBoxLayout>

#define SIP_NO_FILE

class Qgs3DAnimationWidget;
class Qgs3DMapCanvas;
class Qgs3DMapSettings;
class Qgs3DMapToolMeasureLine;
class QgsMapCanvas;
class Qgs3DMapCanvasWidget;
class QgsDockWidget;

class APP_EXPORT Qgs3DMapCanvasDockWidget : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvasDockWidget( QWidget *parent = nullptr );

    ~Qgs3DMapCanvasDockWidget();

    //! takes ownership
    void setMapSettings( Qgs3DMapSettings *map );

    void setMainCanvas( QgsMapCanvas *canvas );

    Qgs3DMapCanvas *mapCanvas3D();
    Qgs3DAnimationWidget *animationWidget();

    Qgs3DMapToolMeasureLine *measurementLineTool();

    QgsDockWidget *dockWidget() { return mDock; }

    QDialog *dialog() { return mDialog; }

  signals:
    void closed();

  private slots:
    void toggleDockMode( bool docked );

  private:
    void switchToWindowMode();

    void switchToDockMode();

  private:
    bool mIsDocked = true;
    Qgs3DMapCanvasWidget *mCanvasWidget = nullptr;

    QgsDockWidget *mDock = nullptr;

    QDialog *mDialog = nullptr;
    QVBoxLayout *mDialogLayout = nullptr;
};

#endif // QGS3DMAPCANVASDOCKWIDGET_H
