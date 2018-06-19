/***************************************************************************
                          qgscompassplugingui.h
 Functions:
                             -------------------
    begin                : Jan 28, 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco@bernawebdesign.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include "ui_qgscompasspluginguibase.h"

#include <QtGui/QDockWidget>
#include "compass.h"


class QgisInterface;

/**
 * \class QgsCompassPluginGui
 */
class QgsCompassPluginGui : public QWidget, private Ui::QgsCompassPluginGuiBase
{
    Q_OBJECT

  public:
    QgsCompassPluginGui( QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr );
    ~QgsCompassPluginGui();

  private:
    QgisInterface *qI = nullptr;
    Compass *compass = nullptr;

  private slots:
    void handleVisibilityChanged( bool visible );
    void handleAzimuth( const QVariant &azimuth, const QVariant &calibrationLevel );
    void rotatePixmap( QLabel *pixmapLabel, QString myFileNameQString, int rotationInt );
};

#endif
