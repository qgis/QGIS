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
#ifndef QGSGRIDMAKERPLUGINGUI_H
#define QGSGRIDMAKERPLUGINGUI_H

#include <ui_pluginguibase.h>
#include <QDialog>

/**
@author Tim Sutton
*/
class QgsGridMakerPluginGui : public QDialog, private Ui::QgsGridMakerPluginGuiBase
{
  Q_OBJECT
public:
  QgsGridMakerPluginGui();
  QgsGridMakerPluginGui(QWidget* parent, Qt::WFlags);
  ~QgsGridMakerPluginGui();

public slots:
  
  void on_pbnSelectOutputFile_clicked();
  void on_pbnCancel_clicked();
  void on_pbnOK_clicked();

signals:
  void drawRasterLayer(QString);
  void drawVectorLayer(QString,QString,QString);
};

#endif
