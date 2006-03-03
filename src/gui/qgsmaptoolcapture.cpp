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
  : QgsMapTool(canvas), mTool(tool)
{
  mCapturing = FALSE;
  
  QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
  mCursor = QCursor(mySelectQPixmap, 8, 8);
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
                             "Cannot edit the vector layer. Use 'Start editing' in the legend item menu",
                             QMessageBox::Ok);
    return;
  }

  double tolerance  = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
  
  // POINT CAPTURING
  if (mTool == CapturePoint)
  {

    QgsPoint idPoint = toMapCoords(e->pos());
    
    // why emit a signal? [MD]
    //emit xyClickCoordinates(idPoint);

    //only do the rest for provider with feature addition support
    //note that for the grass provider, this will return false since
    //grass provider has its own mechanism of feature addition
    if(vlayer->getDataProvider()->capabilities()&QgsVectorDataProvider::AddFeatures)
    {
      QgsFeature* f = new QgsFeature(0,"WKBPoint");
      
      // snap point to points within the vector layer snapping tolerance
      vlayer->snapPoint(idPoint, tolerance);
      // project to layer's SRS
      QgsPoint savePoint = maybeInversePoint(idPoint, "adding point");
      
      // create geos geometry and attach it to feature      
      int size=5+2*sizeof(double);
      unsigned char *wkb = new unsigned char[size];
      int wkbtype=QGis::WKBPoint;
      double x = savePoint.x();
      double y = savePoint.y();
      memcpy(&wkb[1],&wkbtype, sizeof(int));
      memcpy(&wkb[5], &x, sizeof(double));
      memcpy(&wkb[5]+sizeof(double), &y, sizeof(double));
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
    // LINE & POLYGON CAPTURING
  
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
    mCaptureList.push_back(digitisedPoint);
  
    mRubberBand->addPoint(digitisedPoint);
  
    if (e->button() == Qt::LeftButton)
    {
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
      if(mTool == CaptureLine)
      {
        size=1+2*sizeof(int)+2*mCaptureList.size()*sizeof(double);
        wkb= new unsigned char[size];
        int wkbtype=QGis::WKBLineString;
        int length=mCaptureList.size();
        memcpy(&wkb[1],&wkbtype, sizeof(int));
        memcpy(&wkb[5],&length, sizeof(int));
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
      else // polygon
      {
        size=1+3*sizeof(int)+2*(mCaptureList.size()+1)*sizeof(double);
        wkb= new unsigned char[size];
        int wkbtype=QGis::WKBPolygon;
        int length=mCaptureList.size()+1;//+1 because the first point is needed twice
        int numrings=1;
        memcpy(&wkb[1],&wkbtype, sizeof(int));
        memcpy(&wkb[5],&numrings,sizeof(int));
        memcpy(&wkb[9],&length, sizeof(int));
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
    mRubberBand->movePoint(toMapCoords(e->pos()));
  }

} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent(QMouseEvent * e)
{
  // nothing to be done
}


void QgsMapToolCapture::renderComplete()
{
}
