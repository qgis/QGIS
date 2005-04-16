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
#include "imexportplugingui.h"

//qt includes
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtextstream.h>

//standard includes
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <valarray>

#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgsmaptopixel.h>
#include <qgsmaplayerregistry.h>
#include <qgsrasterlayer.h>
#include "imagemapexporter.h"


IMExportPluginGui::IMExportPluginGui() : IMExportPluginGuiBase()
{
  
}

IMExportPluginGui::IMExportPluginGui( QgisIface* iFace, QWidget* parent , const char* name , bool modal , WFlags fl  )
  : IMExportPluginGuiBase( parent, name, modal, fl ), qgisIFace(iFace)
{
  // add the attribute fields to the URL and ALT comboboxes
  layer = (QgsVectorLayer*)(qgisIFace->activeLayer());
  cmbSelectALT->insertItem("- No field selected -");
  for (int i = 0; i < layer->fields().size(); ++i) {
    cmbSelectURL->insertItem(layer->fields()[i].name());
    cmbSelectALT->insertItem(layer->fields()[i].name());
  }
  
  // add the available image formats to the format combobox
  for (int i = 0; i < QImageIO::outputFormats().count(); i++) {
    cmbFormat->insertItem(QImageIO::outputFormats().at(i));
    if (QString(QImageIO::outputFormats().at(i)).compare("PNG") == 0)
      cmbFormat->setCurrentItem(i);
  }
}  


IMExportPluginGui::~IMExportPluginGui()
{
}

void IMExportPluginGui::pbnOK_clicked()
{
  // everything OK?
  if (leSelectHTML->text() == "") {
    QMessageBox::critical(this, "Error", "You must choose an output file.");
    return;
  }
  if (leSelectTemplate->text() == "") {
    QMessageBox::critical(this, "Error", "You must choose a template file.");
    return;
  }
  
  // use extents and resolution from the first raster layer we find
  std::map<QString, QgsMapLayer*>& layers(QgsMapLayerRegistry::instance()->
					  mapLayers());
  std::map<QString, QgsMapLayer*>::const_iterator iter;
  QgsRasterLayer* raster = NULL;
  for (iter = layers.begin(); iter != layers.end(); ++iter) {
    raster = dynamic_cast<QgsRasterLayer*>(iter->second);
    if (raster) {
      std::cerr<<"Will use "<<raster->name()<<std::endl;
      break;
    }
  }

  ImageMapExporter exporter;
  QString templateFile = leSelectTemplate->text();
  QString htmlFile = leSelectHTML->text();
  QString format = cmbFormat->currentText();
  try {
    exporter.generateImageMap(templateFile, htmlFile, sbSelectRadius->value(), 
			      format, cmbSelectURL->currentItem(),
			      cmbSelectALT->currentItem() - 1, layer,
			      qgisIFace->getMapCanvas(), raster);
  } catch (const char* str) {
    QMessageBox::critical(this, "Error", str);
    return;
  }
  done(1);
} 

void IMExportPluginGui::pbnCancel_clicked()
{
 close(1);
}


void IMExportPluginGui::cmbSelectLayer_clicked()
{
  std::cerr<<"cmbSelectLayer_clicked"<<std::endl;
}


void IMExportPluginGui::pbnSelectHTML_clicked()
{
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 "HTML files (*.html)",
				 this,
				 "Save file dialog"
				 "Choose a filename to save under");
  leSelectHTML->setText(filename);
}


void IMExportPluginGui::pbnSelectTemplate_clicked()
{
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 "HTML files (*.html)",
				 this,
				 "Choose template dialog"
				 "Choose a HTML to use as template");
  leSelectTemplate->setText(filename);
}


