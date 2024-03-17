/***************************************************************************
    qgsappcanvasfiltering.h
    -------------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPCANVASFILTERING_H
#define QGSAPPCANVASFILTERING_H

#include "qgis.h"
#include <QObject>
#include <QHash>
#include <QDialog>

class QAction;
class QgsMapCanvas;
class QgsElevationControllerWidget;

class QgsAppCanvasFiltering : public QObject
{
    Q_OBJECT

  public:

    QgsAppCanvasFiltering( QObject *parent );

    void setupElevationControllerAction( QAction *action, QgsMapCanvas *canvas );

  private:

    QHash< QgsMapCanvas *, QgsElevationControllerWidget * > mCanvasElevationControllerMap;

};

#endif // QGSAPPCANVASFILTERING_H
