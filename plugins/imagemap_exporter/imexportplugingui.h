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
#ifndef IMEXPORTPLUGINGUI_H
#define IMEXPORTPLUGINGUI_H

#ifdef WIN32
#include "imexportpluginguibase.h"
#else
#include "imexportpluginguibase.uic.h"
#endif

#include <qgisiface.h>
#include <qgsvectorlayer.h>


/**
@author Tim Sutton
*/
class IMExportPluginGui : public IMExportPluginGuiBase
{
Q_OBJECT
public:
  IMExportPluginGui();
  IMExportPluginGui( QgisIface* iFace, QWidget* parent = 0, const char* name = 0, 
	     bool modal = FALSE, WFlags fl = 0 );
  ~IMExportPluginGui();
  
public slots:
  
  void pbnOK_clicked();
  void pbnCancel_clicked();
  void cmbSelectLayer_clicked();
  void pbnSelectHTML_clicked();
  void pbnSelectTemplate_clicked();
  
signals:
  void drawRasterLayer(QString);
  void drawVectorLayer(QString,QString,QString);
   
private:
  QgisIface* qgisIFace;
  QgsVectorLayer* layer;
};

#endif
