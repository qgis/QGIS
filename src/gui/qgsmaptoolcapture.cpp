/***************************************************************************
    qgsmaptoolcapture.cpp  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsattributedialog.h"
#include "qgsmaptoolcapture.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsfeature.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscursors.h"
#include <QCursor>
#include <QPixmap>
#include <QMessageBox>


QgsMapToolCapture::QgsMapToolCapture(QgsMapCanvas* canvas, enum CaptureTool tool)
  : QgsMapTool(canvas), mTool(tool), mRubberBand(0)
{
  mCapturing = FALSE;
  
  QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
  mCursor = QCursor(mySelectQPixmap, 8, 8);
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  delete mRubberBand;
  mRubberBand = 0;
}


QgsPoint QgsMapToolCapture::maybeInversePoint(QgsPoint point, const char whenmsg[])
{
  QgsVectorLayer *vlayer = dynamic_cast <QgsVectorLayer*>(mCanvas->currentLayer());
  QgsPoint transformedPoint;

  if( mCanvas->projectionsEnabled() )
  {
    // Do reverse transformation before saving. If possible!
    try
    {
      transformedPoint = vlayer->coordinateTransform()->transform(point, QgsCoordinateTransform::INVERSE);
    }
    catch(QgsCsException &cse)
    {
      //#ifdef QGISDEBUG
      std::cout << "Caught transform error when " << whenmsg <<"." 
          << "Setting untransformed values." << std::endl;
      //#endif  
      // Transformation failed,. Bail out with original rectangle.
      return point;
    }
    return transformedPoint;
  }
  return point;
}


void QgsMapToolCapture::canvasReleaseEvent(QMouseEvent * e)
{
  QgsVectorLayer *vlayer = dynamic_cast <QgsVectorLayer*>(mCanvas->currentLayer());
  
  if (!vlayer)
  {
    QMessageBox::information(0,"Not a vector layer","The current layer is not a vector layer",QMessageBox::Ok);
    return;
  }
  
  if (!vlayer->isEditable())
  {
    QMessageBox::information(0,"Layer not editable",
                             "Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check 'Allow Editing'.",
                             QMessageBox::Ok);
    return;
  }

  double tolerance  = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
  
  // POINT CAPTURING
  if (mTool == CapturePoint)
  {
    //check we only use this tool for point/multipoint layers
    if(vlayer->vectorType() != QGis::Point)
      {
	QMessageBox::information(0,"Wrong editing tool", "Cannot apply the 'capture point' tool on this vector layer",\
QMessageBox::Ok);
	return;
      }
    QgsPoint idPoint = toMapCoords(e->pos());
    
    // emit signal - QgisApp can catch it and save point position to clipboard
    // FIXME: is this still actual or something old that's not used anymore?
    //emit xyClickCoordinates(idPoint);

    //only do the rest for provider with feature addition support
    //note that for the grass provider, this will return false since
    //grass provider has its own mechanism of feature addition
    if(vlayer->getDataProvider()->capabilities()&QgsVectorDataProvider::AddFeatures)
    {
      QgsFeature* f = new QgsFeature(0,"WKBPoint");
      QgsPoint savePoint = maybeInversePoint(idPoint, "adding point");
      // snap point to points within the vector layer snapping tolerance
      // project to layer's SRS
      vlayer->snapPoint(savePoint, tolerance);
      
      int size;
      char end=vlayer->endian();
      unsigned char *wkb;
      int wkbtype;
      double x = savePoint.x();
      double y = savePoint.y();

      if(vlayer->getGeometryType() == QGis::WKBPoint)
	{
	  size=1+sizeof(int)+2*sizeof(double);
	  wkb = new unsigned char[size];
	  wkbtype=QGis::WKBPoint;
	  memcpy(&wkb[0],&end,1);
	  memcpy(&wkb[1],&wkbtype, sizeof(int));
	  memcpy(&wkb[5], &x, sizeof(double));
	  memcpy(&wkb[5]+sizeof(double), &y, sizeof(double));
	}
      else if(vlayer->getGeometryType() == QGis::WKBMultiPoint)
	{
	  size = 2+3*sizeof(int)+2*sizeof(double);
	  wkb = new unsigned char[size];
	  wkbtype=QGis::WKBMultiPoint;
	  int position = 0;
	  memcpy(&wkb[position], &end, 1);
	  position += 1;
	  memcpy(&wkb[position], &wkbtype, sizeof(int));
	  position += sizeof(int);
	  int npoint = 1;
	  memcpy(&wkb[position], &npoint, sizeof(int));
	  position += sizeof(int);
	  memcpy(&wkb[position], &end, 1);
	  position += 1;
	  int pointtype = QGis::WKBPoint;
	  memcpy(&wkb[position],&pointtype, sizeof(int));
	  position += sizeof(int);
	  memcpy(&wkb[position], &x, sizeof(double));
	  position += sizeof(double);
	  memcpy(&wkb[position], &y, sizeof(double));
	}

      f->setGeometryAndOwnership(&wkb[0],size);
      // add the fields to the QgsFeature
      std::vector<QgsField> fields=vlayer->fields();
      for(std::vector<QgsField>::iterator it=fields.begin();it!=fields.end();++it)
      {
        f->addAttribute((*it).name(), vlayer->getDefaultValue(it->name(),f));
      }

      // show the dialog to enter attribute values
      if (QgsAttributeDialog::queryAttributes(*f))
        vlayer->addFeature(f);
      else
        delete f;
      
      mCanvas->refresh();
    }

  }  
  else if (mTool == CaptureLine || mTool == CapturePolygon)
  {
    //check we only use the line tool for line/multiline layers
    if(mTool == CaptureLine && vlayer->vectorType() != QGis::Line)
      {
	QMessageBox::information(0,"Wrong editing tool", "Cannot apply the 'capture line' tool on this vector layer",\
QMessageBox::Ok);
	return;
      }

    //check we only use the polygon tool for polygon/multipolygon layers
    if(mTool == CapturePolygon && vlayer->vectorType() != QGis::Polygon)
      {
	QMessageBox::information(0,"Wrong editing tool", "Cannot apply the 'capture polygon' tool on this vector layer",\
QMessageBox::Ok);
	return;
      }

    if (mCaptureList.size() == 0)
    {
      mRubberBand = new QgsRubberBand(mCanvas, mTool == CapturePolygon);
      QgsProject* project = QgsProject::instance();
      QColor color(
          project->readNumEntry("Digitizing", "/LineColorRedPart", 255),
          project->readNumEntry("Digitizing", "/LineColorGreenPart", 0),
          project->readNumEntry("Digitizing", "/LineColorBluePart", 0));
      mRubberBand->setColor(color);
      mRubberBand->setWidth(project->readNumEntry("Digitizing", "/LineWidth", 1));
      mRubberBand->show();
    }
  
    QgsPoint digitisedPoint = toMapCoords(e->pos());
    vlayer->snapPoint(digitisedPoint, tolerance);
  
    if (e->button() == Qt::LeftButton)
    {
      mCaptureList.push_back(digitisedPoint);
      mRubberBand->addPoint(digitisedPoint);
      mCapturing = TRUE;
    }
    else if (e->button() == Qt::RightButton)
    {
      // End of string
  
      mCapturing = FALSE;
      
      delete mRubberBand;
      mRubberBand = NULL;
  
      //create QgsFeature with wkb representation
      QgsFeature* f = new QgsFeature(0,"WKBLineString");
      unsigned char* wkb;
      int size;
      char end=vlayer->endian();
      if(mTool == CaptureLine)
      {
	if(vlayer->getGeometryType() == QGis::WKBLineString)
	  {
	    size=1+2*sizeof(int)+2*mCaptureList.size()*sizeof(double);
	    wkb= new unsigned char[size];
	    int wkbtype=QGis::WKBLineString;
	    int length=mCaptureList.size();
	    memcpy(&wkb[0],&end,1);
	    memcpy(&wkb[1],&wkbtype, sizeof(int));
	    memcpy(&wkb[1+sizeof(int)],&length, sizeof(int));
	    int position=1+2*sizeof(int);
	    double x,y;
	    for(std::list<QgsPoint>::iterator it=mCaptureList.begin();it!=mCaptureList.end();++it)
	      {
		QgsPoint savePoint = maybeInversePoint(*it, "adding line");
		x = savePoint.x();
		y = savePoint.y();
		
		memcpy(&wkb[position],&x,sizeof(double));
		position+=sizeof(double);
		
		memcpy(&wkb[position],&y,sizeof(double));
		position+=sizeof(double);
	      }
	  }
	else if(vlayer->getGeometryType() == QGis::WKBMultiLineString)
	  {
	    size = 1+2*sizeof(int)+1+2*sizeof(int)+2*mCaptureList.size()*sizeof(double);
	    wkb= new unsigned char[size];
	    int position = 0;
	    int wkbtype=QGis::WKBMultiLineString;
	    memcpy(&wkb[position], &end, 1);
	    position += 1;
	    memcpy(&wkb[position], &wkbtype, sizeof(int));
	    position += sizeof(int);
	    int nlines = 1;
	    memcpy(&wkb[position], &nlines, sizeof(int));
	    position += sizeof(int);
	    memcpy(&wkb[position], &end, 1);
	    position += 1;
	    int linewkbtype = QGis::WKBLineString;
	    memcpy(&wkb[position], &linewkbtype, sizeof(int));
	    position += sizeof(int);
	    int length=mCaptureList.size();
	    memcpy(&wkb[position], &length, sizeof(int));
	    position += sizeof(int);
	    double x,y;
	    for(std::list<QgsPoint>::iterator it=mCaptureList.begin();it!=mCaptureList.end();++it)
	      {
		QgsPoint savePoint = maybeInversePoint(*it, "adding line");
		x = savePoint.x();
		y = savePoint.y();
		
		memcpy(&wkb[position],&x,sizeof(double));
		position+=sizeof(double);
		
		memcpy(&wkb[position],&y,sizeof(double));
		position+=sizeof(double);
	      }
	  }
      }
      else // polygon
      {
	if(vlayer->getGeometryType() == QGis::WKBPolygon)
	  {
	    size=1+3*sizeof(int)+2*(mCaptureList.size()+1)*sizeof(double);
	    wkb= new unsigned char[size];
	    int wkbtype=QGis::WKBPolygon;
	    int length=mCaptureList.size()+1;//+1 because the first point is needed twice
	    int numrings=1;
	    memcpy(&wkb[0],&end,1);
	    memcpy(&wkb[1],&wkbtype, sizeof(int));
	    memcpy(&wkb[1+sizeof(int)],&numrings,sizeof(int));
	    memcpy(&wkb[1+2*sizeof(int)],&length, sizeof(int));
	    int position=1+3*sizeof(int);
	    double x,y;
	    std::list<QgsPoint>::iterator it;
	    for(it=mCaptureList.begin();it!=mCaptureList.end();++it)
	      {
		QgsPoint savePoint = maybeInversePoint(*it, "adding poylgon");
		x = savePoint.x();
		y = savePoint.y();
		
		memcpy(&wkb[position],&x,sizeof(double));
		position+=sizeof(double);
		
		memcpy(&wkb[position],&y,sizeof(double));
		position+=sizeof(double);
	      }
	    // close the polygon
	    it=mCaptureList.begin();
	    QgsPoint savePoint = maybeInversePoint(*it, "closing polygon");
	    x = savePoint.x();
	    y = savePoint.y();
  
	    memcpy(&wkb[position],&x,sizeof(double));
	    position+=sizeof(double);
	    
	    memcpy(&wkb[position],&y,sizeof(double));
	  }
	else if(vlayer->getGeometryType() == QGis::WKBMultiPolygon)
	  {
	    size = 2+5*sizeof(int)+2*(mCaptureList.size()+1)*sizeof(double);
	    wkb = new unsigned char[size];
	    int wkbtype = QGis::WKBMultiPolygon;
	    int polygontype = QGis::WKBPolygon;
	    int length = mCaptureList.size()+1;//+1 because the first point is needed twice
	    int numrings = 1;
	    int numpolygons = 1;
	    int position = 0; //pointer position relative to &wkb[0]
	    memcpy(&wkb[position],&end,1);
	    position += 1;
	    memcpy(&wkb[position],&wkbtype, sizeof(int));
	    position += sizeof(int);
	    memcpy(&wkb[position], &numpolygons, sizeof(int));
	    position += sizeof(int);
	    memcpy(&wkb[position], &end, 1);
	    position += 1;
	    memcpy(&wkb[position], &polygontype, sizeof(int));
	    position += sizeof(int);
	    memcpy(&wkb[position], &numrings, sizeof(int));
	    position += sizeof(int);
	    memcpy(&wkb[position], &length, sizeof(int));
	    position += sizeof(int);
	    double x,y;
	    std::list<QgsPoint>::iterator it;
	    for(it=mCaptureList.begin();it!=mCaptureList.end();++it)//add the captured points to the polygon
	      {
		QgsPoint savePoint = maybeInversePoint(*it, "adding poylgon");
		x = savePoint.x();
		y = savePoint.y();
		
		memcpy(&wkb[position],&x,sizeof(double));
		position+=sizeof(double);
		
		memcpy(&wkb[position],&y,sizeof(double));
		position+=sizeof(double);
	      }
	    // close the polygon
	    it=mCaptureList.begin();
	    QgsPoint savePoint = maybeInversePoint(*it, "closing polygon");
	    x = savePoint.x();
	    y = savePoint.y();
	    memcpy(&wkb[position],&x,sizeof(double));
	    position+=sizeof(double);
	    memcpy(&wkb[position],&y,sizeof(double));
	  }
      }
      f->setGeometryAndOwnership(&wkb[0],size);
  
      // add the fields to the QgsFeature
      std::vector<QgsField> fields=vlayer->fields();
      for(std::vector<QgsField>::iterator it=fields.begin();it!=fields.end();++it)
      {
        f->addAttribute((*it).name(),vlayer->getDefaultValue(it->name(), f));
      }
  
      if (QgsAttributeDialog::queryAttributes(*f))
        vlayer->addFeature(f);
      else
        delete f;
      
      // delete the elements of mCaptureList
      mCaptureList.clear();
      mCanvas->refresh();
    }
    
  }
  
} // mouseReleaseEvent



void QgsMapToolCapture::canvasMoveEvent(QMouseEvent * e)
{

  if (mCapturing)
  {
    // show the rubber-band from the last click
    QgsVectorLayer *vlayer = dynamic_cast <QgsVectorLayer*>(mCanvas->currentLayer());
    double tolerance  = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
    QgsPoint rbpoint = toMapCoords(e->pos());
    vlayer->snapPoint(rbpoint, tolerance); //show snapping during dragging
    mRubberBand->movePoint(rbpoint);
  }

} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent(QMouseEvent * e)
{
  // nothing to be done
}


void QgsMapToolCapture::renderComplete()
{
}

void QgsMapToolCapture::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;
}
