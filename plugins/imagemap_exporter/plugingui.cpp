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
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qspinbox.h>

//standard includes
#include <iostream>
#include <fstream>

#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgscoordinatetransform.h>


PluginGui::PluginGui() : PluginGuiBase()
{
  
}

PluginGui::PluginGui( QgisIface* iFace, QWidget* parent , const char* name , bool modal , WFlags fl  )
  : PluginGuiBase( parent, name, modal, fl ), qgisIFace(iFace)
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


PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  // everything OK?
  if (leSelectHTML->text() == "") {
    QMessageBox::critical(this, "Error", "You must choose an output file.");
    return;
  }
  std::ofstream file(leSelectHTML->text());
  if (!file) {
    QMessageBox::critical(this, "Error", 
			  QString("Could not write to %1. "
				  "Please select another file.")
			  .arg(leSelectHTML->text()));
    return;
  }
  
  
  // write the image file
  QString imageFile = leSelectHTML->text();
  imageFile.replace('.', "_");
  imageFile = imageFile + "." + cmbFormat->currentText().lower();
  qgisIFace->getMapCanvas()->saveAsImage(imageFile, 0, 
					 cmbFormat->currentText());
  
  // write the HTML code
  QgsCoordinateTransform transform;
  transform.setParameters(qgisIFace->getMapCanvas()->mupp(),
			  qgisIFace->getMapCanvas()->extent().xMin(),
			  qgisIFace->getMapCanvas()->extent().yMin(),
			  qgisIFace->getMapCanvas()->height());
  file<<"<HTML>"<<std::endl
      <<"  <BODY>"<<std::endl
      <<"    <IMG src=\""<<imageFile
      <<"\" usemap=\"#qgismap\" border=0>"<<std::endl
      <<"    <MAP name=\"qgismap\">"<<std::endl;
  QgsRect extentRect((qgisIFace->getMapCanvas()->extent()));
  layer->select(&extentRect, false);
  QgsFeature* feature = layer->getFirstFeature(true);
  int urlIndex = cmbSelectURL->currentItem();
  int altIndex = cmbSelectALT->currentItem() - 1;
  while (feature != 0) {
    unsigned char* geometry = feature->getGeometry();
    if (geometry[1] == 1) {
      double x = *((double*)(geometry + 5));
      double y = *((double*)(geometry + 13));
      QgsPoint point = transform.transform(x, y);
      file<<"      <AREA shape=\"circle\" coords=\""
	  <<int(point.x())<<","<<int(point.y())<<","
	  <<sbSelectRadius->value()<<"\" "
	  <<"href=\""<<feature->attributeMap()[urlIndex].fieldValue()<<"\"";
      if (altIndex != -1)
	file<<" alt=\""<<feature->attributeMap()[altIndex].fieldValue()<<"\"";
      file<<">"<<std::endl;
    }
    feature = layer->getNextFeature(true);
  }
  file<<"    </MAP>"<<std::endl
      <<"  </BODY>"<<std::endl
      <<"</HTML>"<<std::endl;
  file.close();
  
  done(1);
} 

void PluginGui::pbnCancel_clicked()
{
 close(1);
}


void PluginGui::cmbSelectLayer_clicked()
{
  std::cerr<<"cmbSelectLayer_clicked"<<std::endl;
}


void PluginGui::pbnSelectHTML_clicked()
{
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 "HTML files (*.html)",
				 this,
				 "Save file dialog"
				 "Choose a filename to save under");
  leSelectHTML->setText(filename);
}
