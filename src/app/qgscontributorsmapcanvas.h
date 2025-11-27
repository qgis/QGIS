/***************************************************************************
                          qgscontributorsmapcanvas.h
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
#ifndef QGSCONTRIBUTORSMAPCANVAS_H
#define QGSCONTRIBUTORSMAPCANVAS_H

#include "qgis_app.h"
#include "qgsfloatingwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolpan.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include <QLabel>


class APP_EXPORT QgsContributorsMapFloatingPanel : public QgsFloatingWidget
{
    Q_OBJECT
  public:
    QgsContributorsMapFloatingPanel( QWidget *parent = nullptr );

    void setText( const QString &text );

  private:
    QLabel *mLabel = nullptr;
};


class APP_EXPORT QgsContributorsMapTool : public QgsMapToolPan
{
    Q_OBJECT
  public:
    QgsContributorsMapTool( QgsMapCanvas *canvas, QgsVectorLayer *layer );
    ~QgsContributorsMapTool() override = default;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

  private:
    QgsRectangle filterRectForMouseEvent( QgsMapMouseEvent *e );

    QgsVectorLayer *mContributorsMapLayer = nullptr;
    QgsContributorsMapFloatingPanel *mContributorsMapFloatingPanel = nullptr;
};


class APP_EXPORT QgsContributorsMapCanvas : public QgsMapCanvas
{
    Q_OBJECT
  public:
    QgsContributorsMapCanvas( QWidget *parent = nullptr );
    ~QgsContributorsMapCanvas() override = default;

  private:
    std::unique_ptr<QgsRasterLayer> mContributorsMapBaseLayer;
    std::unique_ptr<QgsVectorLayer> mContributorsMapLayer;
    std::unique_ptr<QgsContributorsMapTool> mContributorsMapTool;
};

#endif // QGSCONTRIBUTORSMAPCANVAS_H
