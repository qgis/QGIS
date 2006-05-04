/***************************************************************************
    qgsmaptoolvertexedit.cpp  - tool for adding, moving, deleting vertices
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

#include "qgsmaptoolvertexedit.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgscursors.h"
#include <QMessageBox>
#include <QPixmap>
#include <QCursor>


QgsMapToolVertexEdit::QgsMapToolVertexEdit(QgsMapCanvas* canvas, enum Tool tool)
  : QgsMapTool(canvas), mTool(tool), mRubberBand(0)
{
  // TODO - select a real cursor
  QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
  mCursor = QCursor(mySelectQPixmap, 8, 8);
}

QgsMapToolVertexEdit::~QgsMapToolVertexEdit()
{
  delete mRubberBand;
  mRubberBand = 0;
}


void QgsMapToolVertexEdit::canvasMoveEvent(QMouseEvent * e)
{
  if (e->buttons() == Qt::LeftButton && (mTool == AddVertex || mTool == MoveVertex))
  {
      int index = (mStartPointValid ? 1 : 0);
      //snap to nearest vertex of vectorlayer
      QgsPoint rbpoint = toMapCoords(e->pos());
      if(mTool == AddVertex)
	{
	  snapVertex(rbpoint, mSnappedAtFeatureId, mSnappedBeforeVertex.back());
	  mRubberBand->movePoint(index, rbpoint);
	}
      else if(mTool == MoveVertex)
	{
	  snapVertex(rbpoint, mSnappedAtFeatureId, mSnappedAtVertex.back());
	  mRubberBand->movePoint(index, rbpoint);
	}
  }

}


void QgsMapToolVertexEdit::canvasPressEvent(QMouseEvent * e)
{
  QgsPoint point = toMapCoords(e->pos());
  
  double x1, y1;
  double x2, y2;
  QgsGeometryVertexIndex index;
  
  if (mTool == AddVertex)
  {
    // Find the closest line segment to the mouse position
    // Then set up the rubber band to its endpoints

#ifdef QGISDEBUG
        std::cout << "QgsMapCanvas::mousePressEvent: QGis::AddVertex." << std::endl;
#endif

    //Find nearest segment of the selected line, move that node to the mouse location
    if (!snapSegmentWithContext(point))
      {
	QMessageBox::warning(0, "Error", "Could not snap vertex. Have you set the tolerance?",
                         QMessageBox::Ok, Qt::NoButton);
	return;
      }
      
    index = mSnappedBeforeVertex;

    // Get the endpoint of the snapped-to segment
    mSnappedAtGeometry.vertexAt(x2, y2, index);

    // Get the startpoint of the snapped-to segment
    index.decrement_back();
    mStartPointValid = mSnappedAtGeometry.vertexAt(x1, y1, index);
    
    createRubberBand();

    if (mStartPointValid)
      {
	mRubberBand->addPoint(QgsPoint(x1,y1));
      }
    mRubberBand->addPoint(toMapCoords(e->pos()));
    mRubberBand->addPoint(QgsPoint(x2,y2));

  }
  else if (mTool == MoveVertex)
  {
#ifdef QGISDEBUG
    std::cout << "QgsMapCanvas::mousePressEvent: QGis::MoveVertex." << std::endl;
#endif

    // Find the closest line segment to the mouse position
    // Then set up the rubber band to its endpoints

    //Find nearest segment of the selected line, move that node to the mouse location    
    if (!snapVertexWithContext(point))
      {
	QMessageBox::warning(0, "Error", "Could not snap vertex. Have you set the tolerance?",
                         QMessageBox::Ok, Qt::NoButton);
	return;
      }

#ifdef QGISDEBUG
    qWarning("Creating rubber band for moveVertex");
#endif

    index = mSnappedAtVertex;

    // Get the startpoint of the rubber band, as the previous vertex to the snapped-to one.
    index.decrement_back();
    mStartPointValid = mSnappedAtGeometry.vertexAt(x1, y1, index);

    // Get the endpoint of the rubber band, as the following vertex to the snapped-to one.
    index.increment_back();
    index.increment_back();
    mStopPointValid = mSnappedAtGeometry.vertexAt(x2, y2, index);
    
    createRubberBand();
    
    if (mStartPointValid)
      {
	mRubberBand->addPoint(QgsPoint(x1,y1));
      }
    mRubberBand->addPoint(toMapCoords(e->pos()));
    if (mStopPointValid)
      {
	mRubberBand->addPoint(QgsPoint(x2,y2));
      }
          
  }
  else if (mTool == DeleteVertex)
  {
#ifdef QGISDEBUG
    std::cout << "QgsMapCanvas::mousePressEvent: QGis::DeleteVertex." << std::endl;
#endif

    // TODO: Find nearest node of the selected line, show a big X symbol
  
    // TODO: Find nearest segment of the selected line, move that node to the mouse location
    if (!snapVertexWithContext(point))
      {
	QMessageBox::warning(0, "Error", "Could not snap vertex. Have you set the tolerance?",
                         QMessageBox::Ok, Qt::NoButton);
	return;
      }
      
    // Get the point of the snapped-to vertex
    mSnappedAtGeometry.vertexAt(x1, y1, mSnappedAtVertex);
    
    mCross = new QgsVertexMarker(mCanvas);
    mCross->setIconType(QgsVertexMarker::ICON_X);
    mCross->setCenter(QgsPoint(x1,y1));
  }
  
}

double QgsMapToolVertexEdit::tolerance()
{
  // TODO: tolerance should be in screen pixels
  // here it can be computed for map coordinates depending on zoom
  return QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
}


bool QgsMapToolVertexEdit::snapSegmentWithContext(QgsPoint& point)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
          
  QgsGeometryVertexIndex beforeVertex;
  int atFeatureId;
  QgsGeometry atGeometry;
  
  if (!vlayer)
    return FALSE;
  
  // TODO: Find nearest segment of the selected line, move that node to the mouse location
  if (!vlayer->snapSegmentWithContext(point, beforeVertex, atFeatureId, atGeometry, tolerance()))
  {
    mSnappedAtFeatureId = -1;
    QMessageBox::warning(0, "Error", "Could not snap segment. Have you set the tolerance?",
                         QMessageBox::Ok, Qt::NoButton);
    return FALSE;
  }
  else
  {
#ifdef QGISDEBUG
      std::cout << "QgsMapToolVertexEdit: Snapped to segment fid " << atFeatureId << "." << std::endl;
#endif
    
    // Save where we snapped to
    mSnappedBeforeVertex = beforeVertex;
    mSnappedAtFeatureId  = atFeatureId;
    mSnappedAtGeometry   = atGeometry;
    return TRUE;
  }

}


bool QgsMapToolVertexEdit::snapVertexWithContext(QgsPoint& point)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  
  QgsGeometryVertexIndex atVertex;
  int atFeatureId;
  QgsGeometry atGeometry;
  
  if (!vlayer)
    return FALSE;
  
  if (!vlayer->snapVertexWithContext(point, atVertex, atFeatureId, atGeometry, tolerance()))
  {
    mSnappedAtFeatureId = -1;
    return FALSE;
  }
  else
  {
#ifdef QGISDEBUG
      std::cout << "QgsMapToolVertexEdit: Snapped to segment fid " << atFeatureId << "." << std::endl;
#endif
    
    // Save where we snapped to
    mSnappedAtVertex     = atVertex;
    mSnappedAtFeatureId  = atFeatureId;
    mSnappedAtGeometry   = atGeometry;
    return TRUE;
  }
}

bool QgsMapToolVertexEdit::snapVertex(QgsPoint& point, int exclFeatureId, int exclVertexNr)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  if(vlayer)
    {
      QgsGeometryVertexIndex vIndex;
      int snappedFeatureId;
      QgsGeometry snappedGeometry;
      //do the snapping to a copy point first
      QgsPoint cpyPoint = point;
      vlayer->snapVertexWithContext(cpyPoint, vIndex, snappedFeatureId, snappedGeometry, tolerance());
      if(snappedFeatureId != exclFeatureId || vIndex.back() != exclVertexNr)
	{
	  point = cpyPoint;
	}
    }
}


void QgsMapToolVertexEdit::canvasReleaseEvent(QMouseEvent * e)
{
  if (mSnappedAtFeatureId == -1)
    {
      return;
    }
  
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  
  if (!vlayer)
  {
    QMessageBox::information(0,"Not a vector layer","The current layer is not a vector layer",QMessageBox::Ok);
    return;
  }
  
  if (!(vlayer->getDataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries))
  {
    QMessageBox::information(0,"Change geometry",
                             "Data provider of the current layer doesn't allow changing geometries",
                             QMessageBox::Ok);
    return;
  }
  
  if (!vlayer->isEditable())
  {
    QMessageBox::information(0,"Layer not editable",
                              "Cannot edit the vector layer. Use 'Start editing' in the legend item menu",
                              QMessageBox::Ok);
    return;
  }
  
  QgsPoint point = toMapCoords(e->pos());
  
  if (mTool == AddVertex)
  {

    //snap to nearest vertex of vectorlayer
    snapVertex(point, mSnappedAtFeatureId, mSnappedBeforeVertex.back());

#ifdef QGISDEBUG
    std::cout << "QgsMapToolVertexEdit::canvasReleaseEvent: AddVertex." << std::endl;
#endif

    delete mRubberBand;
    mRubberBand = 0;
  
    // Add the new vertex
    vlayer->insertVertexBefore(point.x(), point.y(), mSnappedAtFeatureId, mSnappedBeforeVertex);
    mCanvas->refresh();
  }
  else if (mTool == MoveVertex)
  {
    //snap to nearest vertex of vectorlayer
    snapVertex(point, mSnappedAtFeatureId, mSnappedAtVertex.back());
#ifdef QGISDEBUG
    std::cout << "QgsMapToolVertexEdit::canvasReleaseEvent: MoveVertex." << std::endl;
#endif

    delete mRubberBand;
    mRubberBand = 0;
    vlayer->moveVertexAt(point.x(), point.y(), mSnappedAtFeatureId, mSnappedAtVertex);
    mCanvas->refresh();
  }
  else if (mTool == DeleteVertex)
  {
#ifdef QGISDEBUG
    std::cout << "QgsMapToolVertexEdit::canvasReleaseEvent: DeleteVertex." << std::endl;
#endif

    delete mCross;
    mCross = 0;

    // delete vertex
    vlayer->deleteVertexAt(mSnappedAtFeatureId, mSnappedAtVertex);
    mCanvas->refresh();
  }
}

void QgsMapToolVertexEdit::createRubberBand()
{
  mRubberBand = new QgsRubberBand(mCanvas, FALSE);
  QgsProject* project = QgsProject::instance();
  QColor color( project->readNumEntry("Digitizing", "/LineColorRedPart", 255),
                project->readNumEntry("Digitizing", "/LineColorGreenPart", 0),
                project->readNumEntry("Digitizing", "/LineColorBluePart", 0));
  mRubberBand->setColor(color);
  mRubberBand->setWidth(project->readNumEntry("Digitizing", "/LineWidth", 1));
}

void QgsMapToolVertexEdit::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;
}

QgsPoint QgsMapToolVertexEdit::maybeInversePoint(QgsPoint point, const char whenmsg[])
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
