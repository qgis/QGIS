/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
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

#include <qgslauncherpluginguibase.uic.h>

/**
@author Gary Sherman
*/
class QProcess;
class QgsLauncherPluginGui : public QgsLauncherPluginGuiBase
{
Q_OBJECT
public:
    QgsLauncherPluginGui();
    QgsLauncherPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsLauncherPluginGui();
public slots:
  void runProgram();
  void chooseProgram();
  void readFromStdout();
  void readFromStderr();


private:
  QProcess *proc;    
signals:
   void drawRasterLayer(QString);
   void drawVectorrLayer(QString,QString,QString);
};

#endif
