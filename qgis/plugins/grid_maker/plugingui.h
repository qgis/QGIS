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

#include <pluginguibase.h>

/**
@author Tim Sutton
*/
class QgsGridMakerPluginGui : public QgsGridMakerPluginGuiBase
{
  Q_OBJECT
    public:
      QgsGridMakerPluginGui();
      QgsGridMakerPluginGui( QWidget* parent , const char* name , 
			     bool modal , WFlags );
      ~QgsGridMakerPluginGui();

    private:

      void pbnSelectOutputFile_clicked();
      void pbnCancel_clicked();
      void pbnOK_clicked();

signals:
      void drawRasterLayer(QString);
      void drawVectorLayer(QString,QString,QString);
};

#endif
