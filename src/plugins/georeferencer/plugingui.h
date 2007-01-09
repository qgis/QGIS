/***************************************************************************
 *   Copyright (C) 2005 by Lars Luthman
 *   larsl@users.sourceforge.net
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

#include <ui_pluginguibase.h>
#include <QDialog>

class QgisInterface;

/**
@author Tim Sutton
*/
class QgsGeorefPluginGui : public QDialog, private Ui::QgsGeorefPluginGuiBase
{
Q_OBJECT
public:
    QgsGeorefPluginGui();
    QgsGeorefPluginGui(QgisInterface* theQgisInterface, QWidget* parent = 0, Qt::WFlags fl = 0);
    ~QgsGeorefPluginGui();
    
public slots:
    void on_pbnClose_clicked();
    void on_pbnEnterWorldCoords_clicked();
    void on_pbnSelectRaster_clicked();
    
private:
    
   QString mProjBehaviour, mProjectSRS;
   int mProjectSRSID;
   QgisInterface* mIface;

};

#endif
