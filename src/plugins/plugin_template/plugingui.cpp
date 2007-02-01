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
#include "[pluginlcasename]gui.h"
#include "qgscontexthelp.h"

//qt includes

//standard includes

[pluginname]Gui::[pluginname]Gui( QWidget* parent, Qt::WFlags fl )
: QDialog ( parent, fl )
{
  setupUi(this);
}  

[pluginname]Gui::~[pluginname]Gui()
{
}

void [pluginname]Gui::on_buttonBox_accepted()
{
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  //close the dialog
  accept();
} 

void [pluginname]Gui::on_buttonBox_rejected()
{
  reject();
}

void [pluginname]Gui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run(context_id);
}
