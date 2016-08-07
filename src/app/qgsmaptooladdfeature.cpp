/***************************************************************************
    qgsmaptooladdfeature.cpp
    ------------------------
    begin                : April 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdfeature.h"
#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgscsexception.h"
#include "qgscurvepolygon.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsfeatureaction.h"
#include "qgisapp.h"

#include <QMouseEvent>
#include <QSettings>

QgsMapToolAddFeature::QgsMapToolAddFeature( QgsMapCanvas* canvas, CaptureMode mode )
    : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
    , mCheckGeometryType( true )
{
  mToolName = tr( "Add feature" );
}

QgsMapToolAddFeature::~QgsMapToolAddFeature()
{
}

bool QgsMapToolAddFeature::addFeature( QgsVectorLayer *vlayer, QgsFeature *f, bool showModal )
{
  QgsFeatureAction *action = new QgsFeatureAction( tr( "add feature" ), *f, vlayer, -1, -1, this );
  bool res = action->addFeature( QgsAttributeMap(), showModal );
  if ( showModal )
    delete action;
  return res;
}

void QgsMapToolAddFeature::activate()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( vlayer && vlayer->geometryType() == QgsWkbTypes::NullGeometry )
  {
    QgsFeature f;
    addFeature( vlayer, &f, false );
    return;
  }

  QgsMapToolCapture::activate();
}

void QgsMapToolAddFeature::cadCanvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsVectorLayer* vlayer = currentVectorLayer();

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  QgsWkbTypes::Type layerWKBType = vlayer->wkbType();

  QgsVectorDataProvider* provider = vlayer->dataProvider();

  if ( !( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    emit messageEmitted( tr( "The data provider for this layer does not support the addition of features." ), QgsMessageBar::WARNING );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  // POINT CAPTURING
  if ( mode() == CapturePoint )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    //check we only use this tool for point/multipoint layers
    if ( vlayer->geometryType() != QgsWkbTypes::PointGeometry && mCheckGeometryType )
    {
      emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture point' tool on this vector layer" ), QgsMessageBar::WARNING );
      return;
    }



    QgsPoint savePoint; //point in layer coordinates
    try
    {
      QgsPointV2 fetchPoint;
      int res;
      res = fetchLayerPoint( e->mapPointMatch(), fetchPoint );
      if ( res == 0 )
      {
        savePoint = QgsPoint( fetchPoint.x(), fetchPoint.y() );
      }
      else
      {
        savePoint = toLayerCoordinates( vlayer, e->mapPoint() );
      }
      QgsDebugMsg( "savePoint = " + savePoint.toString() );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), QgsMessageBar::WARNING );
      return;
    }

    //only do the rest for provider with feature addition support
    //note that for the grass provider, this will return false since
    //grass provider has its own mechanism of feature addition
    if ( provider->capabilities() & QgsVectorDataProvider::AddFeatures )
    {
      QgsFeature f( vlayer->fields(), 0 );

      QgsGeometry g;
      if ( layerWKBType == QgsWkbTypes::Point )
      {
        g = QgsGeometry::fromPoint( savePoint );
      }
      else if ( layerWKBType == QgsWkbTypes::Point25D )
      {
        g = QgsGeometry( new QgsPointV2( QgsWkbTypes::PointZ, savePoint.x(), savePoint.y(), 0.0 ) );
      }
      else if ( layerWKBType == QgsWkbTypes::MultiPoint )
      {
        g = QgsGeometry::fromMultiPoint( QgsMultiPoint() << savePoint );
      }
      else if ( layerWKBType == QgsWkbTypes::MultiPoint25D )
      {
        QgsMultiPointV2* mp = new QgsMultiPointV2();
        mp->addGeometry( new QgsPointV2( QgsWkbTypes::PointZ, savePoint.x(), savePoint.y(), 0.0 ) );
        g = QgsGeometry( mp );
      }
      else
      {
        // if layer supports more types (mCheckGeometryType is false)
        g = QgsGeometry::fromPoint( savePoint );
      }

      f.setGeometry( g );
      f.setValid( true );

      addFeature( vlayer, &f, false );

      vlayer->triggerRepaint();
    }
  }

  // LINE AND POLYGON CAPTURING
  else if ( mode() == CaptureLine || mode() == CapturePolygon )
  {
    //check we only use the line tool for line/multiline layers
    if ( mode() == CaptureLine && vlayer->geometryType() != QgsWkbTypes::LineGeometry && mCheckGeometryType )
    {
      emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture line' tool on this vector layer" ), QgsMessageBar::WARNING );
      return;
    }

    //check we only use the polygon tool for polygon/multipolygon layers
    if ( mode() == CapturePolygon && vlayer->geometryType() != QgsWkbTypes::PolygonGeometry && mCheckGeometryType )
    {
      emit messageEmitted( tr( "Wrong editing tool, cannot apply the 'capture polygon' tool on this vector layer" ), QgsMessageBar::WARNING );
      return;
    }

    //add point to list and to rubber band
    if ( e->button() == Qt::LeftButton )
    {
      int error = addVertex( e->mapPoint(), e->mapPointMatch() );
      if ( error == 1 )
      {
        //current layer is not a vector layer
        return;
      }
      else if ( error == 2 )
      {
        //problem with coordinate transformation
        emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), QgsMessageBar::WARNING );
        return;
      }

      startCapturing();
    }
    else if ( e->button() == Qt::RightButton )
    {
      // End of string
      deleteTempRubberBand();

      //lines: bail out if there are not at least two vertices
      if ( mode() == CaptureLine && size() < 2 )
      {
        stopCapturing();
        return;
      }

      //polygons: bail out if there are not at least two vertices
      if ( mode() == CapturePolygon && size() < 3 )
      {
        stopCapturing();
        return;
      }

      if ( mode() == CapturePolygon )
      {
        closePolygon();
      }

      //create QgsFeature with wkb representation
      QScopedPointer< QgsFeature > f( new QgsFeature( vlayer->fields(), 0 ) );

      //does compoundcurve contain circular strings?
      //does provider support circular strings?
      bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
      bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::CircularGeometries;

      QgsCurve* curveToAdd = nullptr;
      if ( hasCurvedSegments && providerSupportsCurvedSegments )
      {
        curveToAdd = captureCurve()->clone();
      }
      else
      {
        curveToAdd = captureCurve()->curveToLine();
      }

      if ( mode() == CaptureLine )
      {
        QgsGeometry* g = new QgsGeometry( curveToAdd );
        f->setGeometry( *g );
        delete g;
      }
      else
      {
        QgsCurvePolygon* poly = nullptr;
        if ( hasCurvedSegments && providerSupportsCurvedSegments )
        {
          poly = new QgsCurvePolygon();
        }
        else
        {
          poly = new QgsPolygonV2();
        }
        poly->setExteriorRing( curveToAdd );
        QgsGeometry* g = new QgsGeometry( poly );
        f->setGeometry( *g );
        delete g;

        QgsGeometry featGeom = f->geometry();
        int avoidIntersectionsReturn = featGeom.avoidIntersections();
        f->setGeometry( featGeom );
        if ( avoidIntersectionsReturn == 1 )
        {
          //not a polygon type. Impossible to get there
        }
        if ( f->geometry().isGeosEmpty() ) //avoid intersection might have removed the whole geometry
        {
          emit messageEmitted( tr( "The feature cannot be added because it's geometry collapsed due to intersection avoidance" ), QgsMessageBar::CRITICAL );
          stopCapturing();
          return;
        }
      }
      f->setValid( true );

      if ( addFeature( vlayer, f.data(), false ) )
      {
        //add points to other features to keep topology up-to-date
        int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );

        //use always topological editing for avoidIntersection.
        //Otherwise, no way to guarantee the geometries don't have a small gap in between.
        QStringList intersectionLayers = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList" );
        bool avoidIntersection = !intersectionLayers.isEmpty();
        if ( avoidIntersection ) //try to add topological points also to background layers
        {
          QStringList::const_iterator lIt = intersectionLayers.constBegin();
          for ( ; lIt != intersectionLayers.constEnd(); ++lIt )
          {
            QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( *lIt );
            QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );
            //can only add topological points if background layer is editable...
            if ( vl && vl->geometryType() == QgsWkbTypes::PolygonGeometry && vl->isEditable() )
            {
              vl->addTopologicalPoints( f->geometry() );
            }
          }
        }
        else if ( topologicalEditing )
        {
          vlayer->addTopologicalPoints( f->geometry() );
        }
      }

      stopCapturing();
    }
  }
}
