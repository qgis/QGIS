#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <valarray>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qregexp.h>

#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsrasterlayer.h>
#include "imagemapexporter.h"


bool ImageMapExporter::generateImageMap(QString& templateFile, 
					QString& outputFile,
					int radius, QString& format,
					int urlIndex, int altIndex,
					QgsVectorLayer* layer,
					QgsMapCanvas* canvas,
					QgsRasterLayer* raster) {

  // load the template
  QFile tmpl(templateFile);
  if (!tmpl.open(IO_ReadOnly))
    throw "Could not load the template file!";
  QTextStream tmplStream(&tmpl);
  QString tmplString = tmplStream.read();
  QRegExp regexp("\\[QIM-map\\]", false);
  int mapStart = regexp.search(tmplString);
  if (mapStart == -1)
    throw "The template file does not contain [QIM-map]!";
  int mapEnd = mapStart + regexp.matchedLength();
  
  // open the HTML file
  QFile f(outputFile);
  if (!f.open(IO_WriteOnly)) {
    throw "Could not write to the selected output file. "
      "Please select another file.";
  }
  QTextStream file(&f);
  
  // write the image file
  QFileInfo fi(f);
  QString dir = fi.dir().path() + "/";
  QString basename = fi.fileName();
  basename.replace('.', "_");
  QString imageFile = basename + "." + format;
  QgsRect oldExtent = canvas->extent();
  if (!raster) {
    canvas->saveAsImage(dir + imageFile, 0, format);
  }
  else {
    QPixmap pix(raster->getRasterXDim(), raster->getRasterYDim());
    pix.fill();
    canvas->setExtent(raster->extent());
    canvas->saveAsImage(dir + imageFile, &pix, format);
  }
  
  // write the HTML code
  QgsMapToPixel transform;
  if (raster) {
    transform.setParameters(1, canvas->extent().xMin(),
			    canvas->extent().yMin(), raster->getRasterYDim());
  }
  else {
    transform.setParameters(canvas->mupp(), canvas->extent().xMin(),
			    canvas->extent().yMin(), canvas->height());
  }
  
  file<<tmplString.left(mapStart)

      <<"<SCRIPT type=\"text/javascript\">"<<endl
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
      <<"\" usemap=\"#qgismap\" border=\"0\""
      <<" alt=\"Clickable map\">"<<endl;
  QgsRect extentRect((canvas->extent()));
  layer->select(&extentRect, false);

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
	  //QMessageBox::critical(this, "Error", 
	  //			QString("Could not write the index file "
	  //				"%1!").arg(dir + ciName));
	  throw "Could not write to an index file!";
	}
	QTextStream cIdx(&cif);
	cIdx<<"<HTML>\n  <BODY>"<<endl;
	writeClusterIndex(urls[i], alts[i], cIdx, i, false);
	cIdx<<"  </BODY>\n</HTML>"<<endl;
      }
    }

    for (int i = 0; i < n; ++i) {
      if (i == features[i].second && urls[features[i].second].size() > 1) {
	writeClusterIndex(urls[i], alts[i], file, i, true);
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
      QString alttext;
      if (alts[features[i].second].size() > 1)
	alttext = QString("%1 links").arg(alts[features[i].second].size());
      else if (altIndex != -1)
	alttext = (alts[features[i].second][0].isNull() ? "" : 
		   alts[features[i].second][0].
		   replace(QChar('&'), "&amp;").
		   replace(QChar('"'), "&quot;").
		   replace(QChar('<'), "&lt;").
		   replace(QChar('>'), "&gt;"));
      file<<"\" title=\""<<alttext<<"\"";
      file<<" alt=\""<<alttext<<"\">";
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
  
  layer->removeSelection();
  canvas->setExtent(oldExtent);
  canvas->refresh();
  
  file<<"    </MAP>"<<endl    
      <<"    </DIV>"<<endl
      <<tmplString.mid(mapEnd);
  
  return true;
}


bool ImageMapExporter::polygonIsHole(const double* points, int nPoints)
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


void ImageMapExporter::writeClusterIndex(const std::vector<QString>& urls,
					 const std::vector<QString>& alts,
					 QTextStream& stream, int clusterID,
					 bool popup) {
  stream<<"<DIV style=\"visibility: "<<(popup == true ? "hidden" : "visible")
	<<"; position:absolute;\" class=\"QIM_clusterindex\" "
	<<"id=\"QIM_cidx_"<<clusterID<<"\">"<<endl
	<<"  <DIV class=\"QIM_clusterindexheader\" align=\"right\">"<<endl;
  if (popup == true) {
    stream<<"    <A href=\"javascript:QIM_hideObj('QIM_cidx_"<<clusterID
	  <<"')\">&nbsp;X&nbsp;</A>"<<endl;
  }
  stream<<"  </DIV>"<<endl;
  for (int j = 0; j < urls.size(); ++j) {
    stream<<"  <DIV class=\"QIM_clusterindexentry\">"<<endl
	  <<"    <A href=\""<<(urls[j].isNull() ? "" : urls[j])
	  <<"\" onClick=\"QIM_hideObj('QIM_cidx_"<<clusterID<<"')\">"
	  <<(alts[j].isEmpty() ? urls[j] : alts[j])<<"</A>"<<endl
	  <<"  </DIV>"<<endl;
  }
  stream<<"</DIV>"<<endl;
}
  
