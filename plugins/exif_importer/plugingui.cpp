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
#include "plugingui.h"
#include "exif2gpx.h"

//qt includes
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

//standard includes
#include <cstdio>


QgsExifImporterPluginGui::QgsExifImporterPluginGui() : QgsExifImporterPluginGuiBase()
{
  
}

QgsExifImporterPluginGui::QgsExifImporterPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: QgsExifImporterPluginGuiBase( parent, name, modal, fl )
{
   
}  
QgsExifImporterPluginGui::~QgsExifImporterPluginGui()
{
}

void QgsExifImporterPluginGui::pbnOK_clicked()
{
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to 
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  
  // first check the format of the time offset
  int hour, minute, second;
  if (std::sscanf(leOffset->text().latin1(), "%d:%d:%d", 
		  &hour, &minute, &second) != 3) {
    leOffset->setFocus();
    leOffset->selectAll();
    QMessageBox::warning(this, "Wrong time offset format!",
			 "The time offset is in the wrong format. It should "
			 "be written as HH:MM:SS, e.g. 02:30:00.",
			 QMessageBox::Ok, 0);
    return;
  }
  
  Exif2GPX e2g;
  if (!e2g.loadGPX(leGPXIn->text(), cbTracks->isChecked(),
		   cbWaypoints->isChecked())) {
    QMessageBox::warning(this, "No data found",
			 "Could not find any timestamped points in the "
			 "GPX input file.", QMessageBox::Ok, 0);
    return;
  }
  if (!e2g.writeGPX(pictures, leGPXOut->text(), cbInterpolate->isChecked(), 
		    ((hour < 0 ? -1 : 1) * 
		     (second + 60 * minute + 3600 * std::abs(hour))), 
		    lePrefix->text())) {
    QMessageBox::warning(this, "Could not write GPX",
			 "Could not open the GPX output file for writing.",
			 QMessageBox::Ok, 0);
    return;
  }
  
  emit drawVectorLayer(leGPXOut->text() + QString("?type=waypoint"), 
		       "exif", "gpx");
  
  //close the dialog
  done(1);
} 

void QgsExifImporterPluginGui::pbnCancel_clicked()
{
 close(1);
}


void QgsExifImporterPluginGui::pbnGPXInput_clicked() {
  QString filename = 
    QFileDialog::getOpenFileName(".",
				 "GPS eXchange files (*.gpx *.GPX)",
				 this,
				 "Open file dialog",
				 "Choose a file to open");
  if (filename != "")
    leGPXIn->setText(filename);
}


void QgsExifImporterPluginGui::pbnGPXOutput_clicked() {
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 "GPS eXchange files (*.gpx *.GPX)",
				 this,
				 "Save file dialog"
				 "Choose a filename to save under");
  if (filename != "")
    leGPXOut->setText(filename);
}


void QgsExifImporterPluginGui::pbnPictures_clicked() {
  QStringList tmpPics =
    QFileDialog::getOpenFileNames("EXIF JPEG files (*.jpg *.JPG)",
				  ".",
				  this,
				  "Select pictures",
				  "Select one or more pictures to import");
  if (tmpPics.size() != 0) {
    pictures = tmpPics;
    QFile file(pictures[0]);
    QFileInfo fi(file);
    lePrefix->setText(QString("file://") + fi.dir().absPath() + "/");
  }
  
  lePictures->setText((pictures.size() ? 
		       QString("%1").arg(pictures.size()) : QString("No")) + 
		      " pictures selected");
}


void QgsExifImporterPluginGui::enableRelevantControls() {
  if (pictures.size() == 0 || leGPXIn->text() == "" || 
      leGPXOut->text() == "" || 
      !(cbTracks->isChecked() || cbWaypoints->isChecked())) {
    pbnOK->setEnabled(false);
  }
  else
    pbnOK->setEnabled(true);
}
