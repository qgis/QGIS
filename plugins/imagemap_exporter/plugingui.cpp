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
#include <qregexp.h>
#include <qspinbox.h>
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
  if (leSelectTemplate->text() == "") {
    QMessageBox::critical(this, "Error", "You must choose a template file.");
    return;
  }
  
  
  // load the template
  QFile tmpl(leSelectTemplate->text());
  if (!tmpl.open(IO_ReadOnly)) {
    QMessageBox::critical(this, "Error", 
			  "Could not load the template file!");
    return;
  }
  QTextStream tmplStream(&tmpl);
  QString tmplString = tmplStream.read();
  QRegExp regexp("\\[QIM-map\\]", false);
  int mapStart = regexp.search(tmplString);
  if (mapStart == -1) {
    QMessageBox::critical(this, "Error", 
			  "The template file does not contain [QIM-map]!");
    return;
  }
  int mapEnd = mapStart + regexp.matchedLength();
  
  // open the HTML file
  QFile f(leSelectHTML->text());
  if (!f.open(IO_WriteOnly)) {
    QMessageBox::critical(this, "Error", 
			  QString("Could not write to %1. "
				  "Please select another file.")
			  .arg(leSelectHTML->text()));
    return;
  }
  QTextStream file(&f);
  
  // write the image file
  QFileInfo fi(f);
  QString dir = fi.dir().path() + "/";
  QString basename = fi.fileName();
  basename.replace('.', "_");
  QString imageFile = basename + "." + cmbFormat->currentText().lower();
  qgisIFace->getMapCanvas()->saveAsImage(dir + imageFile, 0, 
					 cmbFormat->currentText());
  
  // write the HTML code
  QgsMapToPixel transform;
  transform.setParameters(qgisIFace->getMapCanvas()->mupp(),
			  qgisIFace->getMapCanvas()->extent().xMin(),
			  qgisIFace->getMapCanvas()->extent().yMin(),
			  qgisIFace->getMapCanvas()->height());
  
  file<<tmplString.left(mapStart)

      <<"<SCRIPT>"<<endl
      <<"  function QIM_hideObj(objID) {"<<endl
      <<"    document.getElementById(objID).style.visibility = 'hidden';"<<endl
      <<"  }"<<endl
      <<""<<endl
      <<"  function QIM_getPos(el){"<<endl
      <<"    for (var lx=0, ly=0; el != null; "<<endl
      <<"         lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent);"<<endl
      <<"    return { x : lx, y : ly}"<<endl
      <<"  }"<<endl
      <<""<<endl
      <<"  function QIM_showClusterIndex(indexID, xPos, yPos) {"<<endl
      <<"    var obj = document.getElementById('QIM_cidx_' + indexID);"<<endl
      <<"    var img = document.getElementById('QIM_img');"<<endl
      <<"    obj.style.position = 'absolute';"<<endl
      <<"    imgPos = QIM_getPos(img);"<<endl
      <<"    obj.style.left = xPos + imgPos.x;"<<endl
      <<"    obj.style.top = yPos + imgPos.y;"<<endl
      <<"    obj.style.visibility = 'visible';"<<endl
      <<"  }"<<endl
      <<"</SCRIPT>"<<endl
    
      <<"    <DIV class=\"QIM_imgdiv\">"<<endl
      <<"    <IMG id=\"QIM_img\" src=\""<<imageFile
      <<"\" usemap=\"#qgismap\" border=\"0\">"<<endl;
  QgsRect extentRect((qgisIFace->getMapCanvas()->extent()));
  layer->select(&extentRect, false);

  int urlIndex = cmbSelectURL->currentItem();
  int altIndex = cmbSelectALT->currentItem() - 1;
  int radius = sbSelectRadius->value();
  
  // do different things for different geometry types
  switch (layer->vectorType()) {
  case QGis::Point: {

    // build a vector of all the features in the current view
    QgsFeature* feature = layer->getFirstFeature(true);
    std::vector<std::pair<QgsFeature*, int> > features;
    while (feature != 0) {
      features.push_back(std::pair<QgsFeature*, int>(feature, -1));
      feature = layer->getNextFeature(true);
    }
    int n = features.size();
    
    // calculate the neighbour matrix
    std::valarray<bool> col(n);
    std::valarray<std::valarray<bool> > neighbour(col, n);
    for (int i = 0; i < n; ++i) {
      neighbour[i][i] = false;
      for (int j = 0; j < i; ++j) {
	unsigned char* geo1 = features[i].first->getGeometry();
	double x1 = *((double*)(geo1 + 5));
	double y1 = *((double*)(geo1 + 13));
	QgsPoint p1 = transform.transform(x1, y1);
	unsigned char* geo2 = features[j].first->getGeometry();
	double x2 = *((double*)(geo2 + 5));
	double y2 = *((double*)(geo2 + 13));
	QgsPoint p2 = transform.transform(x2, y2);
	double d = std::sqrt(std::pow(p2.x() - p1.x(), 2) + 
			     std::pow(p2.y() - p1.y(), 2));
	neighbour[i][j] = std::sqrt(std::pow(p2.x() - p1.x(), 2) + 
				    std::pow(p2.y() - p1.y(), 2)) < radius;
	neighbour[j][i] = neighbour[i][j];
      }
    }
    
    // find clusters
    std::queue<int> q;
    for (int i = 0; i < n; ++i) {
      if (features[i].second == -1) {
	features[i].second = i;
	q.push(i);
      }
      while (q.size() > 0) {
	int k = q.front();
	q.pop();
	for (int j = 0; j < n; ++j) {
	  if (neighbour[k][j] && features[j].second == -1) {
	    features[j].second = features[k].second;
	    q.push(j);
	  }
	}
      }
    }

    // get urls
    std::map<int, std::vector<QString> > urls, alts;
    for (int i = 0; i < n; ++i) {
      urls[features[i].second].
	push_back(features[i].first->attributeMap()[urlIndex].fieldValue());
      if (altIndex != -1) {
	alts[features[i].second].
	  push_back(features[i].first->attributeMap()[altIndex].fieldValue());
      }
      else {
	alts[features[i].second].push_back("");
      }
    }
    
    // create cluster indices
    for (int i = 0; i < n; ++i) {
      if (i == features[i].second && urls[features[i].second].size() > 1) {

	QString ciName = basename + "_" + QString::number(features[i].second) +
	  ".html";
	QFile cif(dir + ciName);
	if (!cif.open(IO_WriteOnly)) {
	  QMessageBox::critical(this, "Error", 
				QString("Could not write the index file "
					"%1!").arg(dir + ciName));
	  return;
	}
	QTextStream cIdx(&cif);
	cIdx<<"<HTML>\n  <BODY>"<<endl;
	writeClusterIndex(urls[i], alts[i], cIdx, i);
	cIdx<<"  </BODY>\n</HTML>"<<endl;
      }
    }

    for (int i = 0; i < n; ++i) {
      if (i == features[i].second && urls[features[i].second].size() > 1) {
	writeClusterIndex(urls[i], alts[i], file, i);
      }
    }
      
    // print map areas
    file<<"    <MAP name=\"qgismap\">"<<endl;
    for (int i = 0; i < n; ++i) {
      unsigned char* geo = features[i].first->getGeometry();
      double x = *((double*)(geo + 5));
      double y = *((double*)(geo + 13));
      QgsPoint p = transform.transform(x, y);
      file<<"      <AREA shape=\"circle\" coords=\""
	  <<int(p.x())<<","<<int(p.y())<<","<<radius<<"\" "
	  <<"href=\"";
      
      // if this is part of a cluster, link to the cluster index
      if (urls[features[i].second].size() > 1) {
	QString ciName = basename + "_" + QString::number(features[i].second) +
	  ".html\"";
	file<<ciName<<" onClick=\"QIM_showClusterIndex("<<features[i].second<<
	  ","<<p.x()<<","<<p.y()<<"); return false;";
      }
      
      // else, just link to the URL
      else
	file<<(urls[features[i].second][0].isNull() ? "" : 
	       urls[features[i].second][0]);
      
      // add an ALT text
      file<<"\" alt=\"";
      if (alts[features[i].second].size() > 1)
	file<<alts[features[i].second].size()<<" links";
      else if (altIndex != -1)
	file<<(alts[features[i].second][0].isNull() ? "" : 
	       alts[features[i].second][0]);
      file<<"\">"<<endl;
    }
    
    break;
  }
    
  case QGis::Line:
    break;
    
  case QGis::Polygon: {
    file<<"    <MAP name=\"qgismap\">"<<endl;
    QgsFeature* feature = layer->getFirstFeature(true);
    while (feature != 0) {
      unsigned char* geometry = feature->getGeometry();
      QString url = feature->attributeMap()[urlIndex].fieldValue();
      url = (url.isNull() ? "" : url);
      QString alt;
      if (altIndex != -1)
	alt = feature->attributeMap()[altIndex].fieldValue();
      alt = (alt.isNull() ? "" : alt);
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
	  file<<" alt=\""<<alt<<"\">"<<endl;
	  ringBegin += 4 + numPoints * 16;
	}
      }
      feature = layer->getNextFeature(true);
    }
    break;
  }

  }
  
  file<<"    </MAP>"<<endl    
      <<"    </DIV>"<<endl
      <<tmplString.mid(mapEnd);
  
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


void PluginGui::pbnSelectTemplate_clicked()
{
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 "HTML files (*.html)",
				 this,
				 "Choose template dialog"
				 "Choose a HTML to use as template");
  leSelectTemplate->setText(filename);
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


void PluginGui::writeClusterIndex(const std::vector<QString>& urls,
				  const std::vector<QString>& alts,
				  QTextStream& stream, int clusterID) {
  stream<<"<DIV style=\"visibility: hidden; position:absolute;\" class=\"QIM_clusterindex\" "
	<<"id=\"QIM_cidx_"<<clusterID<<"\">"<<endl
	<<"  <DIV class=\"QIM_clusterindexheader\" align=\"right\">"<<endl
	<<"    <A href=\"javascript:QIM_hideObj('QIM_cidx_"<<clusterID<<"')\""
	<<">&nbsp;X&nbsp;</A>"<<endl
	<<"  </DIV>"<<endl;
  for (int j = 0; j < urls.size(); ++j) {
    stream<<"  <DIV class=\"QIM_clusterindexentry\">"<<endl
	  <<"    <A href=\""<<(urls[j].isNull() ? "" : urls[j])
	  <<"\" onClick=\"QIM_hideObj('QIM_cidx_"<<clusterID<<"')\">"
	  <<(alts[j].isEmpty() ? urls[j] : alts[j])<<"</A>"<<endl
	  <<"  </DIV>"<<endl;
  }
  stream<<"</DIV>"<<endl;
}
  
