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
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include "graticulecreator.h"
//standard includes
#include <iostream>

PluginGui::PluginGui() : PluginGuiBase()
{
  
}
PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
   
} 
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  //check input file exists
  //
  GraticuleCreator *  myGraticuleCreator = new GraticuleCreator(leOutputShapeFile->text(),spinLongInterval->value(),spinLatInterval->value());
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  //emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //

  delete myGraticuleCreator;
  emit drawVectorLayer(leOutputShapeFile->text(),QString("Graticule"),QString("ogr"));
  //close the dialog
  done(1);
} 


void PluginGui::pbnSelectOutputFile_clicked()
{
  std::cout << " Gps File Importer Gui::pbnSelectOutputFile_clicked() " << std::endl;
  QString myOutputFileNameQString = QFileDialog::getSaveFileName(
          ".",
          "ESRI Shapefile (*.shp)",
          this,
          "save file dialog"
          "Choose a filename to save under" );
  leOutputShapeFile->setText(myOutputFileNameQString);
  if ( leOutputShapeFile->text()==""  )
  {
    pbnOK->setEnabled(false);
  }
  else
  {
    pbnOK->setEnabled(true);
  }
}


void PluginGui::pbnCancel_clicked()
{
 close(1);
}

