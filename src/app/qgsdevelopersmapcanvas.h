/***************************************************************************
                          qgsdevelopersmapcanvas.h
                             -------------------
    begin                : November 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDEVELOPERSMAPCANVAS_H
#define QGSDEVELOPERSMAPCANVAS_H

#include "qgis_app.h"
#include "qgsfloatingwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolpan.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include <QLabel>


class APP_EXPORT QgsDevelopersMapFloatingPanel : public QgsFloatingWidget
{
    Q_OBJECT
  public:
    QgsDevelopersMapFloatingPanel( QWidget *parent = nullptr );

    void setText( const QString &text );

  private:
    QLabel *mLabel = nullptr;
};


class APP_EXPORT QgsDevelopersMapTool : public QgsMapToolPan
{
    Q_OBJECT
  public:
    QgsDevelopersMapTool( QgsMapCanvas *canvas, QgsVectorLayer *layer );
    ~QgsDevelopersMapTool() override = default;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

  private:
    QgsRectangle filterRectForMouseEvent( QgsMapMouseEvent *e );

    QgsVectorLayer *mDevelopersMapLayer = nullptr;
    QgsDevelopersMapFloatingPanel *mDevelopersMapFloatingPanel = nullptr;
};


class APP_EXPORT QgsDevelopersMapCanvas : public QgsMapCanvas
{
    Q_OBJECT
  public:
    QgsDevelopersMapCanvas( QWidget *parent = nullptr );
    ~QgsDevelopersMapCanvas() override = default;

  private:
    std::unique_ptr<QgsRasterLayer> mDevelopersMapBaseLayer;
    std::unique_ptr<QgsVectorLayer> mDevelopersMapLayer;
    std::unique_ptr<QgsDevelopersMapTool> mDevelopersMapTool;
};

#endif // QGSDEVELOPERSMAPCANVAS_H
