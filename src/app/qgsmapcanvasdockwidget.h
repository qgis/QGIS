/***************************************************************************
    qgsmapcanvasdockwidget.h
    ------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPCANVASDOCKWIDGET_H
#define QGSMAPCANVASDOCKWIDGET_H

#include <ui_qgsmapcanvasdockwidgetbase.h>

#include "qgsdockwidget.h"
#include "qgis_app.h"

class QgsMapCanvas;

class APP_EXPORT QgsMapCanvasDockWidget : public QgsDockWidget, private Ui::QgsMapCanvasDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMapCanvasDockWidget( const QString &name, QWidget *parent = nullptr );

    /**
     * Returns the map canvas contained in the dock widget.
     */
    QgsMapCanvas *mapCanvas();

  private slots:

    void setMapCrs();

  private:

    QgsMapCanvas *mMapCanvas = nullptr;


};


#endif // QGSMAPCANVASDOCKWIDGET_H
