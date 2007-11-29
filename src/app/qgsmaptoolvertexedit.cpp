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

#include "qgslogger.h"
#include "qgsmaptoolvertexedit.h"
#include "qgsmapcanvas.h"
#include "qgsmaprender.h"
#include "qgsvertexmarker.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgscursors.h"
#include "qgsmessageviewer.h"
#include <QMessageBox>
#include <QPixmap>
#include <QCursor>
#include <QSettings>

QgsMapToolVertexEdit::QgsMapToolVertexEdit(QgsMapCanvas* canvas, enum Tool tool)
  : QgsMapTool(canvas), mTool(tool), mRubberBandIndex1(-1), mRubberBandIndex2(-1), mRubberBand(0)
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
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  if(!vlayer)
  {
    return;
  }

  if (e->buttons() == Qt::LeftButton && (mTool == AddVertex || mTool == MoveVertex) && mRubberBand)
  {
    //int index = (mStartPointValid ? 1 : 0);
    int index;

    if(mTool == MoveVertex)
    {
      if(mRubberBandIndex1 == -1)
      {
        index = 0;
      }
      else
      {
        index = 1;
      }
    }
    else
    {
      index = 1;
    }

    QgsPoint layerPoint = toLayerCoords(vlayer, e->pos());

    //snap to nearest vertex of vectorlayer
    if (mTool == AddVertex)
    {
      snapVertex(layerPoint, mSnappedAtFeatureId, mSnappedBeforeVertex.back());
    }
    else if (mTool == MoveVertex)
    {
      snapVertex(layerPoint, mSnappedAtFeatureId, mSnappedAtVertex.back());
    }

    //transform snapped point back into map coordinates for the rubberband
    QgsPoint mapPoint = toMapCoords(vlayer, layerPoint);

    mRubberBand->movePoint(index, mapPoint);
  }
}


void QgsMapToolVertexEdit::canvasPressEvent(QMouseEvent * e)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  if(!vlayer)
  {
    return;
  }

  QgsPoint layerPoint = toLayerCoords(vlayer, e->pos());

  QgsGeometryVertexIndex index, rb1Index, rb2Index; //rb1Index/rb2Index is for rubberbanding

  if (mTool == AddVertex)
  {
    // Find the closest line segment to the mouse position
    // Then set up the rubber band to its endpoints

    QgsDebugMsg("QGis::AddVertex.");

    //Find nearest segment of the selected line, move that node to the mouse location
    if ( !snapSegmentWithContext(layerPoint) )
    {
      displaySnapToleranceWarning();
      return;
    }

    index = mSnappedBeforeVertex;
    // Get the endpoint of the snapped-to segment
    QgsPoint layerPnt2 = mSnappedAtGeometry.vertexAt(index);

    // Get the startpoint of the snapped-to segment
    index.decrement_back();
    QgsPoint layerPnt1 = mSnappedAtGeometry.vertexAt(index);

    createRubberBand();

    if (layerPnt1 != QgsPoint(0,0))
    {
      QgsPoint mapPnt1 = toMapCoords(vlayer, layerPnt1);
      mRubberBand->addPoint(mapPnt1);
    }
    QgsPoint mapPoint = toMapCoords(vlayer, layerPoint);
    mRubberBand->addPoint(mapPoint);
    QgsPoint mapPnt2 = toMapCoords(vlayer, layerPnt2);
    mRubberBand->addPoint(mapPnt2);
  }
  else if (mTool == MoveVertex)
  {
    QgsDebugMsg("QGis::MoveVertex.");

    // Find the closest line segment to the mouse position
    // Then find the closest vertex on that line segment
    // Then set up the rubber band to its adjoining vertexes

    QgsPoint snapPoint;

    snapPoint = layerPoint;
    if(vlayer->vectorType() == QGis::Point)//snap to point for point/multipoint layers
    {
      if(!snapVertexWithContext(snapPoint))
      {
        displaySnapToleranceWarning();
        return;
      }
    }
    else //snap to segment and take the closest vertex in case of line/multiline/polygon/multipolygon layers
    {
      if (!snapSegmentWithContext(snapPoint))
      {
        displaySnapToleranceWarning();
        return;
      }

      if (!snapVertexOfSnappedSegment(snapPoint))
      {
        QMessageBox::warning(0, QObject::tr("Error"), 
          QObject::tr("Could not snap vertex. Have you set the tolerance in Settings > Project Properties > General?"));
        return;
      }

      QgsDebugMsg("Creating rubber band for moveVertex");

      index = mSnappedAtVertex;
      createRubberBand();
      if(mRubberBandIndex1 != -1)
      {
        rb1Index.push_back(mRubberBandIndex1);
        QgsPoint layerPnt1 = mSnappedAtGeometry.vertexAt(rb1Index);
        QgsPoint mapPnt1 = toMapCoords(vlayer, layerPnt1);
        mRubberBand->addPoint(mapPnt1);
        mStartPointValid = true;
      }
      else
      {
        mStartPointValid = false;
      }
      if(mRubberBandIndex1 != -1 && mRubberBandIndex2 != -1)
      {
        QgsPoint mapPoint = toMapCoords(vlayer, layerPoint);
        mRubberBand->addPoint(mapPoint);
      }
      if(mRubberBandIndex2 != -1)
      {
        rb2Index.push_back(mRubberBandIndex2);
        QgsPoint layerPnt2 = mSnappedAtGeometry.vertexAt(rb2Index);
        QgsPoint mapPnt2 = toMapCoords(vlayer, layerPnt2);
        mRubberBand->addPoint(mapPnt2);
      }
      QgsDebugMsg("Creating rubber band for moveVertex");
    }
  }
  else if (mTool == DeleteVertex)
  {
    QgsDebugMsg("QGis::DeleteVertex.");

    // TODO: Find nearest node of the selected line, show a big X symbol

    // TODO: Find nearest segment of the selected line, move that node to the mouse location
    if (!snapVertexWithContext(layerPoint))
    {
      QMessageBox::warning(0, QObject::tr("Error"), 
        QObject::tr("Could not snap vertex. Have you set the tolerance in Settings > Project Properties > General?"));
      return;
    }

    // Get the point of the snapped-to vertex
    QgsPoint layerPnt = mSnappedAtGeometry.vertexAt(mSnappedAtVertex);
    QgsPoint mapPnt = toMapCoords(vlayer, layerPnt);

    mCross = new QgsVertexMarker(mCanvas);
    mCross->setIconType(QgsVertexMarker::ICON_X);
    mCross->setCenter(mapPnt);
  }

}

double QgsMapToolVertexEdit::tolerance()
{
  return QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
}

void QgsMapToolVertexEdit::displaySnapToleranceWarning()
{
  QSettings myQSettings;
  QString myQSettingsLabel = "/UI/displaySnapWarning";
  bool displaySnapWarning = myQSettings.value(myQSettingsLabel, true).toBool();

  if (displaySnapWarning)
  {
    QgsMessageViewer* m = new QgsMessageViewer(0);
    m->setWindowTitle(tr("Snap tolerance"));
    m->setCheckBoxText(tr("Don't show this message again"));
    m->setCheckBoxVisible(true);
    m->setCheckBoxQSettingsLabel(myQSettingsLabel);
    m->setMessageAsHtml("<p>" + 
                        tr("Could not snap segment.") +
                        "</p><p>" +
                        tr("Have you set the tolerance in "
                           "Settings > Project Properties > General?") +
                        "</p>");
    m->exec();
  }
}

bool QgsMapToolVertexEdit::snapSegmentWithContext(QgsPoint& point)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
          
  QgsGeometryVertexIndex beforeVertex;
  int atFeatureId;
  QgsGeometry atGeometry;
  
  if (!vlayer)
    {
      return FALSE;
    }
  
  if (!vlayer->snapSegmentWithContext(point, beforeVertex, atFeatureId, atGeometry, tolerance()))
  {
    mSnappedAtFeatureId = -1;
    return FALSE;
  }
  else
  {
    QgsDebugMsg("Snapped to segment fid " + QString::number(atFeatureId) + ".");
    
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
    {
      return FALSE;
    }
  
  if (!vlayer->snapVertexWithContext(point, atVertex, mRubberBandIndex1, mRubberBandIndex2, atFeatureId, atGeometry, tolerance()))
  {
    mSnappedAtFeatureId = -1;
    return FALSE;
  }
  else
  {
    QgsDebugMsg("Snapped to segment fid " + QString::number(atFeatureId) + ".");
    
    // Save where we snapped to
    mSnappedAtVertex     = atVertex;
    mSnappedAtFeatureId  = atFeatureId;
    mSnappedAtGeometry   = atGeometry;
    return TRUE;
  }
}

bool QgsMapToolVertexEdit::snapVertexOfSnappedSegment(QgsPoint& point)
{
  double twoBeforeVertexSqrDist;
  double    beforeVertexSqrDist;

  // Set up the "other side" of the snapped-to segment
  QgsGeometryVertexIndex snappedTwoBeforeVertex(mSnappedBeforeVertex);
  snappedTwoBeforeVertex.decrement_back();

  QgsDebugMsg( "Choice of " + snappedTwoBeforeVertex.toString()
             + " or " + mSnappedBeforeVertex.toString() + ".");

  twoBeforeVertexSqrDist = mSnappedAtGeometry.sqrDistToVertexAt(point, snappedTwoBeforeVertex);
  beforeVertexSqrDist    = mSnappedAtGeometry.sqrDistToVertexAt(point, mSnappedBeforeVertex);

  QgsDebugMsg( "Choice of " + QString::number(twoBeforeVertexSqrDist)
             + " or " + QString::number(beforeVertexSqrDist) + ".");

  // See which of the two verticies is closer (i.e. smaller squared distance)
  if (twoBeforeVertexSqrDist < beforeVertexSqrDist)
  {
    mSnappedAtVertex = snappedTwoBeforeVertex;
  }
  else
  {
    mSnappedAtVertex = mSnappedBeforeVertex;
  }

  QgsDebugMsg("Chose " + mSnappedAtVertex.toString() + ".");

  // Now determine the rubber band verticies to use with this snapped vertex
  mSnappedAtGeometry.adjacentVerticies(mSnappedAtVertex, mRubberBandIndex1, mRubberBandIndex2);

  return TRUE;
}

void QgsMapToolVertexEdit::snapVertex(QgsPoint& point, int exclFeatureId, int exclVertexNr)
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  if(vlayer)
  {
    QgsGeometryVertexIndex vIndex;
    int snappedFeatureId;
    int rbPoint1, rbPoint2;
    QgsGeometry snappedGeometry;
    QgsPoint cpyPoint = point;
    vlayer->snapVertexWithContext(cpyPoint, vIndex, rbPoint1, rbPoint2, snappedFeatureId, snappedGeometry, tolerance());
    if(snappedFeatureId != exclFeatureId || vIndex.back() != exclVertexNr)
    {
      //success, assign snapped coordinates to point
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
    QMessageBox::information(0, QObject::tr("Not a vector layer"),
            QObject::tr("The current layer is not a vector layer"));
    return;
  }
  
  if (!(vlayer->getDataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries))
  {
    QMessageBox::information(0, QObject::tr("Change geometry"),
            QObject::tr("Data provider of the current layer doesn't allow changing geometries"));
    return;
  }
  
  if (!vlayer->isEditable())
  {
    QMessageBox::information(0, QObject::tr("Layer not editable"),
            QObject::tr("Cannot edit the vector layer. Use 'Start editing' in the legend item menu"));
    return;
  }
  
  QgsPoint layerPoint = toLayerCoords(vlayer, e->pos());
  
  if (mTool == AddVertex)
  {

    //snap to nearest vertex of vectorlayer
    snapVertex(layerPoint, mSnappedAtFeatureId, mSnappedBeforeVertex.back());

    QgsDebugMsg("AddVertex.");

    deleteRubberBand();

    // Add the new vertex
    vlayer->insertVertexBefore(layerPoint.x(), layerPoint.y(), mSnappedAtFeatureId, mSnappedBeforeVertex);
    mCanvas->refresh();
  }
  else if (mTool == MoveVertex)
  {
    //snap to nearest vertex of vectorlayer
    snapVertex(layerPoint, mSnappedAtFeatureId, mSnappedAtVertex.back());

    QgsDebugMsg("MoveVertex.");

    delete mRubberBand;
    mRubberBand = 0;
    vlayer->moveVertexAt(layerPoint.x(), layerPoint.y(), mSnappedAtFeatureId, mSnappedAtVertex);
    mCanvas->refresh();
  }
  else if (mTool == DeleteVertex)
  {
    QgsDebugMsg("DeleteVertex.");

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

void QgsMapToolVertexEdit::deleteRubberBand()
{
  if (mRubberBand)
  {
    delete mRubberBand;
    mRubberBand = 0;

    // also remove reference to the indicies to the ends of the rubber band
    mRubberBandIndex1 = -1;
    mRubberBandIndex2 = -1;
  }
}

void QgsMapToolVertexEdit::deactivate()
{
  deleteRubberBand();
}
