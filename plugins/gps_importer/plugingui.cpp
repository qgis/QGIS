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
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsdataprovider.h"

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
#include <cassert>
#include <cstdlib>
#include <iostream>


PluginGui::PluginGui() : PluginGuiBase()
{
  populateDeviceComboBox();
  populateULLayerComboBox();
  populateIMPBabelFormats();
  tabWidget->removePage(tabWidget->page(2));
}
PluginGui::PluginGui( std::vector<QgsVectorLayer*> gpxMapLayers, 
		      QWidget* parent , const char* name , bool modal , 
		      WFlags fl  )
  : PluginGuiBase( parent, name, modal, fl ), gpxLayers(gpxMapLayers)
{
  populateDeviceComboBox();
  populateULLayerComboBox();
  populateIMPBabelFormats();
  tabWidget->removePage(tabWidget->page(2));
} 
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  
  // what should we do?
  switch (tabWidget->currentPageIndex()) {
  // add a GPX/LOC layer?
  case 0:
    emit loadGPXFile(leGPXFile->text(), cbGPXWaypoints->isChecked(), 
		     cbGPXRoutes->isChecked(), cbGPXTracks->isChecked());
    break;
  
    // or import a download file?
    /*
      case 666:
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
      break;
    */
    
    // or import other file?
  case 1: {
    const QString& typeString(cmbDLFeatureType->currentText());
    emit importGPSFile(leIMPInput->text(), 
		       babelFormats.find(impFormat)->second.formatName,
		       typeString == "Waypoints", typeString == "Routes",
		       typeString == "Tracks", leIMPOutput->text(),
		       leIMPLayer->text());
    break;
  }
  
  // or download GPS data from a device?
  case 2: {
    int featureType = cmbDLFeatureType->currentItem();
    emit downloadFromGPS(cmbDLProtocol->currentText().lower(),
			 cmbDLDevice->currentText(), featureType == 0,
			 featureType == 1, featureType == 2, 
			 leDLOutput->text(), leDLBasename->text());
    break;
  }
  
  // or upload GPS data to a device?
  case 3:
    emit uploadToGPS(gpxLayers[cmbULLayer->currentItem()], 
		     cmbULProtocol->currentText().lower(),
		     cmbULDevice->currentText());
    break;
  }
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
  else if (tabWidget->currentPageIndex() == 666) {
    if ( (leOutputShapeFile->text()=="") || (leInputFile->text()=="") )
    {
      pbnOK->setEnabled(false);
    }
    else
    {
      pbnOK->setEnabled(true);
    }
  }
  
  // import other file
  else if (tabWidget->currentPageIndex() == 1) {
    
    if ((leIMPInput->text() == "") || (leIMPOutput->text() == "") ||
	(leIMPLayer->text() == ""))
      pbnOK->setEnabled(false);
    else
      pbnOK->setEnabled(true);
  }
  
  // download from device
  else if (tabWidget->currentPageIndex() == 2) {
    if (cmbDLDevice->currentText() == "" || leDLBasename->text() == "" ||
	leDLOutput->text() == "")
      pbnOK->setEnabled(false);
    else
      pbnOK->setEnabled(true);
  }

  // upload from device
  else if (tabWidget->currentPageIndex() == 3) {
    if (cmbULDevice->currentText() == "" || cmbULLayer->currentText() == "")
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


void PluginGui::pbnIMPInput_clicked() {
  QString myFileType;
  QString myFileName = QFileDialog::getOpenFileName(
          "." , //initial dir
	  babelFilter,
          this , //parent dialog
          "OpenFileDialog" , //QFileDialog qt object name
          "Select file and format to import" , //caption
          &myFileType //the pointer to store selected filter
          );
  impFormat = myFileType.left(myFileType.length() - 6);
  std::map<QString, BabelFormatInfo>::const_iterator iter;
  iter = babelFormats.find(impFormat);
  if (iter == babelFormats.end()) {
    std::cerr<<"Unknown file format selected: "
	     <<myFileType.left(myFileType.length() - 6)<<std::endl;
  }
  else {
    std::cerr<<iter->first<<" selected"<<std::endl;
    leIMPInput->setText(myFileName);
    cmbIMPFeature->clear();
    if (iter->second.hasWaypoints)
      cmbIMPFeature->insertItem("Waypoints");
    if (iter->second.hasRoutes)
      cmbIMPFeature->insertItem("Routes");    
    if (iter->second.hasTracks)
      cmbIMPFeature->insertItem("Tracks");
  }
}


void PluginGui::pbnIMPOutput_clicked() {
  QString myFileNameQString = 
    QFileDialog::getSaveFileName("." , //initial dir
				 "GPS eXchange format (*.gpx)",
				 this , //parent dialog
				 "Select GPX output",
				 "Choose a filename to save under" );
  leIMPOutput->setText(myFileNameQString);
}


void PluginGui::populateDeviceComboBox() {
  // look for linux serial devices
#ifdef linux
  QString linuxDev("/dev/ttyS%1");
  for (int i = 0; i < 10; ++i) {
    if (QFileInfo(linuxDev.arg(i)).exists()) {
      cmbDLDevice->insertItem(linuxDev.arg(i));
      cmbULDevice->insertItem(linuxDev.arg(i));
    }
    else
      break;
  }
#endif

  // and freebsd devices (untested)
#ifdef freebsd
  QString freebsdDev("/dev/cuaa%1");
  for (int i = 0; i < 10; ++i) {
    if (QFileInfo(freebsdDev.arg(i)).exists()) {
      cmbDLDevice->insertItem(freebsdDev.arg(i));
      cmbULDevice->insertItem(freebsdDev.arg(i));
    }
    else
      break;
  }
#endif
  
  // and solaris devices (also untested)
#ifdef sparc
  QString solarisDev("/dev/cua/%1");
  for (int i = 'a'; i < 'k'; ++i) {
    if (QFileInfo(solarisDev.arg(char(i))).exists()) {
      cmbDLDevice->insertItem(solarisDev.arg(char(i)));
      cmbULDevice->insertItem(solarisDev.arg(char(i)));
    }
    else
      break;
  }
#endif

#ifdef WIN32
  cmbULDevice->insertItem("com1");
  cmbULDevice->insertItem("com2");
#endif
  // OSX, OpenBSD, NetBSD etc? Anyone?

}


void PluginGui::populateULLayerComboBox() {
  for (int i = 0; i < gpxLayers.size(); ++i) {
    cmbULLayer->insertItem(gpxLayers[i]->name());
    std::cerr<<gpxLayers[i]->name()<<std::endl;
  }
}


void PluginGui::populateIMPBabelFormats() {
  babelFormats["Magellan Mapsend"] = 
    BabelFormatInfo("mapsend", true, true, true);
  babelFormats["Garmin PCX5"] = 
    BabelFormatInfo("pcx", true, false, true);
  babelFormats["Garmin Mapsource"] = 
    BabelFormatInfo("mapsource", true, true, true);
  babelFormats["GPSUtil"] = 
    BabelFormatInfo("gpsutil", true, false, false);
  babelFormats["PocketStreets 2002/2003 Pushpin"] = 
    BabelFormatInfo("psp", true, false, false);
  babelFormats["CoPilot Flight Planner"] = 
    BabelFormatInfo("copilot", true, false, false);
  babelFormats["Magellan Navigator Companion"] = 
    BabelFormatInfo("magnav", true, false, false);
  babelFormats["Holux"] = 
    BabelFormatInfo("holux", true, false, false);
  babelFormats["Topo by National Geographic"] = 
    BabelFormatInfo("tpg", true, false, false);
  babelFormats["TopoMapPro"] = 
    BabelFormatInfo("tmpro", true, false, false);
  babelFormats["GeocachingDB"] = 
    BabelFormatInfo("gcdb", true, false, false);
  babelFormats["Tiger"] = 
    BabelFormatInfo("tiger", true, false, false);
  babelFormats["EasyGPS Binary Format"] = 
    BabelFormatInfo("easygps", true, false, false);
  babelFormats["Delorme Routes"] = 
    BabelFormatInfo("saroute", false, false, true);
  babelFormats["Navicache"] = 
    BabelFormatInfo("navicache", true, false, false);
  babelFormats["PSITrex"] = 
    BabelFormatInfo("psitrex", true, true, true);
  babelFormats["Delorme GPS Log"] = 
    BabelFormatInfo("gpl", false, false, true);
  babelFormats["OziExplorer"] = 
    BabelFormatInfo("ozi", true, false, false);
  babelFormats["NMEA Sentences"] = 
    BabelFormatInfo("nmea", true, false, true);
  babelFormats["Delorme Street Atlas 2004 Plus"] = 
    BabelFormatInfo("saplus", true, false, false);
  babelFormats["Microsoft Streets and Trips"] = 
    BabelFormatInfo("s_and_t", true, false, false);
  babelFormats["NIMA/GNIS Geographic Names"] = 
    BabelFormatInfo("nima", true, false, false);
  babelFormats["Maptech"] = 
    BabelFormatInfo("mxf", true, false, false);
  babelFormats["Mapopolis.com Mapconverter Application"] = 
    BabelFormatInfo("mapconverter", true, false, false);
  babelFormats["GPSman"] = 
    BabelFormatInfo("gpsman", true, false, false);
  babelFormats["GPSDrive"] = 
    BabelFormatInfo("gpsdrive", true, false, false);
  babelFormats["Fugawi"] = 
    BabelFormatInfo("fugawi", true, false, false);
  babelFormats["DNA"] = 
    BabelFormatInfo("dna", true, false, false);
  std::map<QString, BabelFormatInfo>::const_iterator iter;
  for (iter = babelFormats.begin(); iter != babelFormats.end(); ++iter)
    babelFilter.append((const char*)iter->first).append(" (*.*);;");
}
