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
#include "waypointtoshape.h"
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
  if (!QFile::exists ( leInputFile->text() ))
  {
   QMessageBox::warning( this, "GPS Importer",
               "Unable to find the input file.\n"
                   "Please reselect a valid file." );
   return;
  }
  WayPointToShape *  myWayPointToShape = new  WayPointToShape(leOutputShapeFile->text(),leInputFile->text());
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  delete myWayPointToShape;
  emit drawVectorLayer(leOutputShapeFile->text(),QString("Waypoints"),QString("ogr"));
  //close the dialog
  done(1);
} 


void PluginGui::pbnSelectInputFile_clicked()
{
  std::cout << " Gps File Importer::pbnSelectInputFile_clicked() " << std::endl;
  QString myFileTypeQString;
  QString myFilterString="Text File (*.txt)";
  QString myFileNameQString = QFileDialog::getOpenFileName(
          "." , //initial dir
          myFilterString,  //filters to select
          this , //parent dialog
          "OpenFileDialog" , //QFileDialog qt object name
          "Select GPS dump text file" , //caption
          &myFileTypeQString //the pointer to store selected filter
          );
  std::cout << "Selected filetype filter is : " << myFileTypeQString << std::endl;
  leInputFile->setText(myFileNameQString);
  if ( (leOutputShapeFile->text()=="") || (leInputFile->text()=="") )
  {
    pbnOK->setEnabled(false);
  }
  else
  {
    pbnOK->setEnabled(true);
  }
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
  if ( leOutputShapeFile->text()=="" || leInputFile->text()=="" )
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

