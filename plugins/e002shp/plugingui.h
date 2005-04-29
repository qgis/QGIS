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
#ifndef QGSE002SHPPLUGINGUI_H
#define QGSE002SHPPLUGINGUI_H

#include <pluginguibase.uic.h>

/**
@author Tim Sutton
*/
class QgsE002shpPluginGui : public QgsE002shpPluginGuiBase
{
Q_OBJECT
public:
    QgsE002shpPluginGui();
    QgsE002shpPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsE002shpPluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();

private:
    
signals:
   void drawRasterLayer(QString);
   void drawVectorLayer(QString,QString,QString);
};

#endif
