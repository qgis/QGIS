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
#include <cmath>
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
    QString url = feature->attributeMap()[urlIndex].fieldValue();
    QString alt = "";
    if (altIndex != -1)
      alt = feature->attributeMap()[altIndex].fieldValue();
    std::cerr<<"geo: "<<(*(int*)(geometry + 1))<<", url: "<<url<<std::endl;
    
    // point - use a circle area
    if (geometry[1] == 1) {
      double x = *((double*)(geometry + 5));
      double y = *((double*)(geometry + 13));
      QgsPoint point = transform.transform(x, y);
      file<<"      <AREA shape=\"circle\" coords=\""
	  <<int(point.x())<<","<<int(point.y())<<","
	  <<sbSelectRadius->value()<<"\" "<<"href=\""<<url<<"\""
	  <<" alt=\""<<alt<<"\">"<<std::endl;
    }
    
    // linestring - use a polygon area that extends RADIUS units from the line
    else if (geometry[1] == 2) {
      
    }
    
    // polygon - just use polygon areas, ignore the radius
    else if (geometry[1] == 3) {
      int numRings = *((int*)(geometry + 5));
      if (numRings != 0) {
	unsigned char* ringBegin = geometry + 9;
	for (int r = 0; r < numRings; ++r) {
	  int numPoints = *((int*)(ringBegin));
	  file<<"      <AREA shape=\"poly\" coords=\"";
	  for (int i = 0; i < numPoints; ++i) {
	    double x = *((double*)(ringBegin + 4 + i*16)); 
	    double y = *((double*)(ringBegin + 12 + i*16)); 
	    QgsPoint point = transform.transform(x, y);
	    file<<int(point.x())<<","<<int(point.y());
	    if (i != numPoints - 1)
	      file<<",";
	  }
	  file<<"\" ";
	  // not sure if this is correct - I want to check if the ring is solid
	  // or a hole, polygonIsHole() calculates the winding
	  if (polygonIsHole((double*)(ringBegin + 4), numPoints))
	    file<<"nohref";
	  else
	    file<<"href=\""<<url<<"\"";
	  file<<" alt=\""<<alt<<"\">"<<std::endl;
	  ringBegin += 4 + numPoints * 16;
	}
      }
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


bool PluginGui::polygonIsHole(const double* points, int nPoints)
{
  /* This is how it works: Find the leftmost point, point[i]. Check if the ray
     from point[i] that goes through point[i+1] is above the ray from point[i]
     that goes through point[i-1] - if it is the polygon is defined clockwise,
     otherwise it's counterclockwise. Counterclockwise polygons are holes.
     NOTE: point is an array of doubles, not points, but you get the idea. */
  
  // find the leftmost point
  int leftmost = 0;
  for (int i = 1; i < nPoints - 1; ++i) {
    if (points[i*2] < points[leftmost*2])
      leftmost = i;
  }
  
  // calculate the angles for the two rays and compare
  int h = (leftmost == 0 ? nPoints - 2 : leftmost - 1);
  int j = (leftmost == nPoints - 2 ? 0 : leftmost + 1);
  double a1 = std::atan((points[j*2+1] - points[leftmost*2+1]) /
			(points[j*2] - points[leftmost*2]));
  double a2 = std::atan((points[h*2+1] - points[leftmost*2+1]) /
			(points[h*2] - points[leftmost*2]));
  if (a1 > a2)
    return false;
  if (a1 < a2)
    return true;
  // angles are equal - probably two vertical lines - check points instead
  else
    return (points[h*2] < points[j*2]);
}
