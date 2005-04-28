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

#include <pluginguibase.uic.h>

/**
@author Tim Sutton
*/
class QgsGeorefPluginGui : public QgsGeorefPluginGuiBase
{
Q_OBJECT
public:
    QgsGeorefPluginGui();
    QgsGeorefPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsGeorefPluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void pbnSelectRaster_clicked();

public slots:
    void openPointDialog();
    void loadLayer(QString);
    
private:
    
signals:
   void drawRasterLayer(QString);
   void drawVectorLayer(QString,QString,QString);
};

#endif
