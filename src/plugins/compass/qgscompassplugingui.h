/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include "ui_qgscompasspluginguibase.h"
#include "qgscontexthelp.h"

#include <QtGui/QDockWidget>
#include <QTimer>
#include <QtSensors/QCompassReading>
QTM_USE_NAMESPACE

class QgisInterface;

/**
 * \class QgsCompassPluginGui
 */
class QgsCompassPluginGui : public QDockWidget, private Ui::QgsCompassPluginGuiBase
{
    Q_OBJECT

  public:
    QgsCompassPluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsCompassPluginGui();

  private:
    QgisInterface * qI;
    QCompassReading mCompass;
    QTimer * mTimer;

  private slots:
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void updateReading();

};

#endif
