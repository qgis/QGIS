/***************************************************************************
 *   Copyright (C) 2005 by Lars Luthman
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
#include "qgsleastsquares.h"
#include "qgspointdialog.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"

//qt includes
#include <QFileDialog>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>

//standard includes

QgsGeorefPluginGui::QgsGeorefPluginGui() : QgsGeorefPluginGuiBase()
{
  
}

QgsGeorefPluginGui::QgsGeorefPluginGui(QWidget* parent, Qt::WFlags fl)
: QDialog(parent, fl)
{
  setupUi(this);
  connect(pbnEnterWorldCoords, SIGNAL(clicked()), 
	  this, SLOT(openPointDialog()));
}  


QgsGeorefPluginGui::~QgsGeorefPluginGui()
{
}


void QgsGeorefPluginGui::pbnOK_clicked()
{
  done(1);
} 


void QgsGeorefPluginGui::pbnCancel_clicked()
{
 close(1);
}


void QgsGeorefPluginGui::pbnSelectRaster_clicked() {
  QSettings settings("QuantumGIS", "qgis");
  QString dir = settings.readEntry("/Plugin-GeoReferencer/rasterdirectory");
  if (dir.isEmpty())
    dir = ".";
  QString filename = 
    QFileDialog::getOpenFileName(this,
				 "Choose a raster file",
                 dir,
				 "Raster files (*.*)");
  leSelectRaster->setText(filename);
}


void QgsGeorefPluginGui::openPointDialog() {
  
  // do we think that this is a valid raster?
  if (!QgsRasterLayer::isValidRasterFileName(leSelectRaster->text())) {
    QMessageBox::critical(this, "Error", 
			  "The selected file is not a valid raster file.");
    return;
  }
  
  // remember the directory
  {
    QSettings settings("QuantumGIS", "qgis");
    QFileInfo fileInfo(leSelectRaster->text());
    settings.writeEntry("/Plugin-GeoReferencer/rasterdirectory", 
			fileInfo.dirPath());
  }
  
  // guess the world file name
  QString raster = leSelectRaster->text();
  int point = raster.findRev('.');
  QString worldfile;
  if (point != -1 && point != raster.length() - 1) {
    worldfile = raster.left(point + 1);
    worldfile += raster.at(point + 1);
    worldfile += raster.at(raster.length() - 1);
    worldfile += 'w';
  }
  
  // check if there already is a world file
  if (!worldfile.isEmpty()) {
    if (QFile::exists(worldfile)) {
      QMessageBox::critical(this, "Error",
			    "The selected file already seems to have a "
			    "world file! If you want to replace it with a "
			    "new world file, remove the old one first.");
      return;
    }
  }
  
  // XXX This is horrible, but it works and I'm tired / ll
  {
    QSettings settings("QuantumGIS", "qgis");
    mProjBehaviour = settings.readEntry("/Projections/defaultBehaviour");
    mProjectSRS = QgsProject::instance()->
      readEntry("SpatialRefSys", "/ProjectSRSProj4String");
    mProjectSRSID = QgsProject::instance()->
      readNumEntry("SpatialRefSys", "/ProjectSRSID");
    settings.writeEntry("/Projections/defaultBehaviour", "useProject");
    QgsProject::instance()->
      writeEntry("SpatialRefSys", "/ProjectSRSProj4String", GEOPROJ4);
    QgsProject::instance()->
    writeEntry("SpatialRefSys", "/ProjectSRSID", int(GEOSRS_ID));
  }
  QgsRasterLayer* layer = new QgsRasterLayer(raster, "Raster");
  {
    QSettings settings("QuantumGIS", "qgis");
    settings.writeEntry("/Projections/defaultBehaviour", mProjBehaviour);
    QgsProject::instance()->
      writeEntry("SpatialRefSys", "/ProjectSRSProj4String", mProjectSRS);
    QgsProject::instance()->
      writeEntry("SpatialRefSys", "/ProjectSRSID", mProjectSRSID);
  }
  
  QgsPointDialog* dlg = new QgsPointDialog(layer, this, NULL, true);
  connect(dlg, SIGNAL(loadLayer(QString)), this, SLOT(loadLayer(QString)));
  dlg->show();
}


void QgsGeorefPluginGui::loadLayer(QString str) {
  emit drawRasterLayer(str);
}
