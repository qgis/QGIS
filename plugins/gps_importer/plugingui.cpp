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
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include <qprocess.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include "waypointtoshape.h"

//standard includes
#include <cstdlib>
#include <iostream>


PluginGui::PluginGui() : PluginGuiBase()
{
  populateDeviceComboBox();
}
PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
  populateDeviceComboBox();
} 
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  
  // what should we do?
  // add a GPX/LOC layer?
  if (tabWidget->currentPageIndex() == 0) {

    //check if input file is readable
    QFileInfo fileInfo(leGPXFile->text());
    if (!fileInfo.isReadable())
      {
	QMessageBox::warning( this, "GPX/LOC Loader",
			      "Unable to read the selected file.\n"
			      "Please reselect a valid file." );
	return;
      }
    
    // add the requested layers
    if (cbGPXTracks->isChecked())
      emit drawVectorLayer(leGPXFile->text() + "?type=track", 
			   fileInfo.baseName() + ", tracks", "gpx");
    if (cbGPXRoutes->isChecked())
      emit drawVectorLayer(leGPXFile->text() + "?type=route", 
			   fileInfo.baseName() + ", routes", "gpx");
    if (cbGPXWaypoints->isChecked())
      emit drawVectorLayer(leGPXFile->text() + "?type=waypoint", 
			   fileInfo.baseName() + ", waypoints", "gpx");
  }
  
  // or import a download file?
  else if (tabWidget->currentPageIndex() == 1) {
    
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
  }
  
  // or start downloading GPS data from a device?
  else {
    
    // try to start the gpsbabel process
    QStringList babelArgs;
    babelArgs<<"gpsbabel"<<"-t"<<"-i"<<"garmin"<<"-o"<<"gpx"
	     <<cmbDLDevice->currentText()<<leDLOutput->text();
    QProcess babelProcess(babelArgs);
    if (!babelProcess.start()) {
      QMessageBox::warning(this, "Could not start process",
			   "Could not start GPSBabel!");
      return;
    }
    
    // wait for gpsbabel to finish (or the user to cancel)
    QProgressDialog progressDialog("Downloading data...", "Cancel", 0,
				   this, 0, true);
    progressDialog.show();
    for (int i = 0; babelProcess.isRunning(); ++i) {
      QApplication::eventLoop()->processEvents(0);
      progressDialog.setProgress(i/64);
      if (progressDialog.wasCancelled())
	return;
    }
    
    // did we get any data?
    if (babelProcess.exitStatus() != 0) {
      QString babelError(babelProcess.readStderr());
      QString errorMsg("Could not download data from GPS!\n\n");
      errorMsg += babelError;
      QMessageBox::warning(this, "Error downloading data", errorMsg);
      return;
    }
    
    // add the layer
    emit drawVectorLayer(leDLOutput->text() + "?type=track", 
			 leDLBasename->text() + ", tracks", "gpx");
  }
  
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
}


void PluginGui::pbnDLOutput_clicked()
{
  QString myFileNameQString = QFileDialog::getSaveFileName(
          "." , //initial dir
	  "GPS eXchange format (*.gpx)",
          this , //parent dialog
	  "Select GPX output",
	  "Choose a filename to save under" );
  leDLOutput->setText(myFileNameQString);
}


void PluginGui::enableRelevantControls() 
{
  // load GPX/LOC
  if (tabWidget->currentPageIndex() == 0) {
    if ((leGPXFile->text()==""))
    {
      pbnOK->setEnabled(false);
      cbGPXWaypoints->setEnabled(false);
      cbGPXRoutes->setEnabled(false);
      cbGPXTracks->setEnabled(false);
      cbGPXWaypoints->setChecked(false);
      cbGPXRoutes->setChecked(false);
      cbGPXTracks->setChecked(false);
    }
    else
    {
      pbnOK->setEnabled(true);
      cbGPXWaypoints->setEnabled(true);
      cbGPXWaypoints->setChecked(true);
      if (leGPXFile->text().right(4).lower() != ".loc") {
	cbGPXRoutes->setEnabled(true);
	cbGPXTracks->setEnabled(true);
	cbGPXRoutes->setChecked(true);
	cbGPXTracks->setChecked(true);
      }
      else {
	cbGPXRoutes->setEnabled(false);
	cbGPXTracks->setEnabled(false);
	cbGPXRoutes->setChecked(false);
	cbGPXTracks->setChecked(false);
      }
    }
  }
  
  // import download file
  else if (tabWidget->currentPageIndex() == 1) {
    if ( (leOutputShapeFile->text()=="") || (leInputFile->text()=="") )
    {
      pbnOK->setEnabled(false);
    }
    else
    {
      pbnOK->setEnabled(true);
    }
  }
  
  // download from device
  else {
    if (cmbDLDevice->currentText() == "" || leDLBasename->text() == "" ||
	leDLOutput->text() == "")
      pbnOK->setEnabled(false);
    else
      pbnOK->setEnabled(true);
  }
}


void PluginGui::pbnCancel_clicked()
{
 close(1);
}


void PluginGui::pbnGPXSelectFile_clicked()
{
  std::cout << " Gps File Importer::pbnGPXSelectFile_clicked() " << std::endl;
  QString myFileTypeQString;
  QString myFilterString="GPS eXchange format (*.gpx);;"
    "Geocaching waypoints (*.loc)";
  QString myFileNameQString = QFileDialog::getOpenFileName(
          "." , //initial dir
          myFilterString,  //filters to select
          this , //parent dialog
          "OpenFileDialog" , //QFileDialog qt object name
          "Select GPX or LOC file" , //caption
          &myFileTypeQString //the pointer to store selected filter
          );
  std::cout << "Selected filetype filter is : " << myFileTypeQString << std::endl;
  leGPXFile->setText(myFileNameQString);
}


void PluginGui::populateDeviceComboBox() {
  // look for linux serial devices
#ifdef linux
  QString linuxDev("/dev/ttyS%1");
  for (int i = 0; i < 10; ++i) {
    if (QFileInfo(linuxDev.arg(i)).exists())
      cmbDLDevice->insertItem(linuxDev.arg(i));
    else
      break;
  }
#endif

  // and freebsd devices (untested)
#ifdef freebsd
  QString freebsdDev("/dev/cuaa%1");
  for (int i = 0; i < 10; ++i) {
    if (QFileInfo(freebsdDev.arg(i)).exists())
      cmbDLDevice->insertItem(freebsdDev.arg(i));
    else
      break;
  }
#endif
  
  // and solaris devices (also untested)
#ifdef sparc
  QString solarisDev("/dev/cua/%1");
  for (int i = 'a'; i < 'k'; ++i) {
    if (QFileInfo(solarisDev.arg(char(i))).exists())
      cmbDLDevice->insertItem(solarisDev.arg(char(i)));
    else
      break;
  }
#endif

  // OSX, OpenBSD, NetBSD etc? Anyone?

}
