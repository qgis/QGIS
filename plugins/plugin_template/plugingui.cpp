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

[pluginname]Gui::[pluginname]Gui() : [pluginname]GuiBase()
{
  
}

[pluginname]Gui::[pluginname]Gui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: [pluginname]GuiBase( parent, name, modal, fl )
{
   
}  
[pluginname]Gui::~[pluginname]Gui()
{
}

void [pluginname]Gui::pbnOK_clicked()
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

void [pluginname]Gui::pbnCancel_clicked()
{
 close(1);
}
