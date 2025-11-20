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
#include "qgsmapcanvas.h"
#include "qgsmaptoolpan.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

class APP_EXPORT QgsDevelopersMapCanvas : public QgsMapCanvas
{
    Q_OBJECT
  public:
    QgsDevelopersMapCanvas( QWidget *parent = nullptr );
    ~QgsDevelopersMapCanvas() override {}

  private:
    std::unique_ptr<QgsRasterLayer> mDevelopersMapBaseLayer;
    std::unique_ptr<QgsVectorLayer> mDevelopersMapLayer;
    std::unique_ptr<QgsMapToolPan> mDevelopersMapTool;
};

#endif // QGSDEVELOPERSMAPCANVAS_H
