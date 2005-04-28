/***************************************************************************
 *   Copyright (C) 2004 by Lars Luthman
 *   larsl@users.sourceforge.net
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSEXITIMPORTERPLUGINGUI_H
#define QGSEXITIMPORTERPLUGINGUI_H

#include <qstringlist.h>

#ifdef WIN32
#include <pluginguibase.h>
#else
#include <pluginguibase.uic.h>
#endif

/**
@author Tim Sutton
*/
class QgsExifImporterPluginGui : public QgsExifImporterPluginGuiBase
{
Q_OBJECT
public:
    QgsExifImporterPluginGui();
    QgsExifImporterPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsExifImporterPluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void pbnGPXInput_clicked();
    void pbnGPXOutput_clicked();
    void pbnPictures_clicked();
    void enableRelevantControls();
    
private:
    
    QStringList pictures;
    
signals:
   void drawRasterLayer(QString);
   void drawVectorLayer(QString,QString,QString);
};

#endif
