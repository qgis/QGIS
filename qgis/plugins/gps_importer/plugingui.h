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
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include "../../src/qgsvectorlayer.h"
#include "pluginguibase.h"
#include "qgsbabelformat.h"

#include <vector>

#include <qstring.h>


/**
@author Tim Sutton
*/
class PluginGui : public PluginGuiBase
{
  Q_OBJECT
public:
  PluginGui(const BabelMap& importers, BabelMap& devices, 
	    std::vector<QgsVectorLayer*> gpxMapLayers, QWidget* parent, 
	    const char* name , bool modal , WFlags);
  ~PluginGui();

public slots:

  void openDeviceEditor();
  void slotDevicesUpdated();
  
private:
  
  void pbnSelectInputFile_clicked();
  void pbnSelectOutputFile_clicked();
  
  void pbnGPXSelectFile_clicked();
  
  void pbnIMPInput_clicked();
  void pbnIMPOutput_clicked();
  
  void pbnDLOutput_clicked();
  
  void enableRelevantControls();
  void pbnCancel_clicked();
  void pbnOK_clicked();
  
  void populateDeviceComboBox();
  void populateULLayerComboBox();
  void populateIMPBabelFormats();
  void populatePortComboBoxes();
  
signals:
  void drawRasterLayer(QString);
  void drawVectorLayer(QString,QString,QString);
  void loadGPXFile(QString filename, bool showWaypoints, bool showRoutes, 
		   bool showTracks);
  void importGPSFile(QString inputFilename, QgsBabelFormat* importer,
		     bool importWaypoints, bool importRoutes, 
		     bool importTracks, QString outputFilename, 
		     QString layerName);
  void downloadFromGPS(QString device, QString port, bool downloadWaypoints, 
		       bool downloadRoutes, bool downloadTracks, 
		       QString outputFilename, QString layerName);
  void uploadToGPS(QgsVectorLayer* gpxLayer, QString device, QString port);
  
private:
  
  std::vector<QgsVectorLayer*> gpxLayers;
  const BabelMap& mImporters;
  BabelMap& mDevices;
  QString babelFilter;
  QString impFormat;
};

#endif
