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
#include "plugingui.h"

//qt includes

//standard includes

QgsE002shpPluginGui::QgsE002shpPluginGui() : QgsE002shpPluginGuiBase()
{
  
}

QgsE002shpPluginGui::QgsE002shpPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: QgsE002shpPluginGuiBase( parent, name, modal, fl )
{
   
}  
QgsE002shpPluginGui::~QgsE002shpPluginGui()
{
}

void QgsE002shpPluginGui::pbnOK_clicked()
{
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  //close the dialog
  done(1);
} 

void QgsE002shpPluginGui::pbnCancel_clicked()
{
 close(1);
}
