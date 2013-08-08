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
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsfeatureaction.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>

QgsMapToolAddFeature::QgsMapToolAddFeature( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas )
{
}

QgsMapToolAddFeature::~QgsMapToolAddFeature()
{
}

bool QgsMapToolAddFeature::addFeature( QgsVectorLayer *vlayer, QgsFeature *f )
{
  QgsFeatureAction action( tr( "add feature" ), *f, vlayer, -1, -1, this );
  return action.addFeature();
}

void QgsMapToolAddFeature::canvasReleaseEvent( QMouseEvent * e )
{
  QgsDebugMsg( "entered." );

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  QGis::WkbType layerWKBType = vlayer->wkbType();

  QgsVectorDataProvider* provider = vlayer->dataProvider();

  if ( !( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    QMessageBox::information( 0, tr( "Layer cannot be added to" ),
                              tr( "The data provider for this layer does not support the addition of features." ) );
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
    //check we only use this tool for point/multipoint layers
    if ( vlayer->geometryType() != QGis::Point )
    {
      QMessageBox::information( 0, tr( "Wrong editing tool" ),
                                tr( "Cannot apply the 'capture point' tool on this vector layer" ) );
      return;
    }


    QgsPoint idPoint; //point in map coordinates
    QList<QgsSnappingResult> snapResults;
    QgsPoint savePoint; //point in layer coordinates

    if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
    {
      idPoint = snapPointFromResults( snapResults, e->pos() );
      try
      {
        savePoint = toLayerCoordinates( vlayer, idPoint );
        QgsDebugMsg( "savePoint = " + savePoint.toString() );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QMessageBox::information( 0, tr( "Coordinate transform error" ),
                                  tr( "Cannot transform the point to the layers coordinate system" ) );
        return;
      }
    }

    //only do the rest for provider with feature addition support
    //note that for the grass provider, this will return false since
    //grass provider has its own mechanism of feature addition
    if ( provider->capabilities() & QgsVectorDataProvider::AddFeatures )
    {
      QgsFeature* f = new QgsFeature( vlayer->pendingFields(), 0 );

      QgsGeometry *g = 0;
      if ( layerWKBType == QGis::WKBPoint || layerWKBType == QGis::WKBPoint25D )
      {
        g = QgsGeometry::fromPoint( savePoint );
      }
      else if ( layerWKBType == QGis::WKBMultiPoint || layerWKBType == QGis::WKBMultiPoint25D )
      {
        g = QgsGeometry::fromMultiPoint( QgsMultiPoint() << savePoint );
      }

      f->setGeometry( g );

      vlayer->beginEditCommand( tr( "Feature added" ) );

      if ( addFeature( vlayer, f ) )
      {
        vlayer->endEditCommand();
      }
      else
      {
        delete f;
        vlayer->destroyEditCommand();
      }

      mCanvas->refresh();
    }
  }

  // LINE AND POLYGON CAPTURING
  else if ( mode() == CaptureLine || mode() == CapturePolygon )
  {
    //check we only use the line tool for line/multiline layers
    if ( mode() == CaptureLine && vlayer->geometryType() != QGis::Line )
    {
      QMessageBox::information( 0, tr( "Wrong editing tool" ),
                                tr( "Cannot apply the 'capture line' tool on this vector layer" ) );
      return;
    }

    //check we only use the polygon tool for polygon/multipolygon layers
    if ( mode() == CapturePolygon && vlayer->geometryType() != QGis::Polygon )
    {
      QMessageBox::information( 0, tr( "Wrong editing tool" ),
                                tr( "Cannot apply the 'capture polygon' tool on this vector layer" ) );
      return;
    }

    //add point to list and to rubber band
    if ( e->button() == Qt::LeftButton )
    {
      int error = addVertex( e->pos() );
      if ( error == 1 )
      {
        //current layer is not a vector layer
        return;
      }
      else if ( error == 2 )
      {
        //problem with coordinate transformation
        QMessageBox::information( 0, tr( "Coordinate transform error" ),
                                     tr( "Cannot transform the point to the layers coordinate system" ) );
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

      //create QgsFeature with wkb representation
      QgsFeature* f = new QgsFeature( vlayer->pendingFields(),  0 );

      QgsGeometry *g;

      if ( mode() == CaptureLine )
      {
        if ( layerWKBType == QGis::WKBLineString || layerWKBType == QGis::WKBLineString25D )
        {
          g = QgsGeometry::fromPolyline( points().toVector() );
        }
        else if ( layerWKBType == QGis::WKBMultiLineString || layerWKBType == QGis::WKBMultiLineString25D )
        {
          g = QgsGeometry::fromMultiPolyline( QgsMultiPolyline() << points().toVector() );
        }
        else
        {
          QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot add feature. Unknown WKB type" ) );
          stopCapturing();
          return; //unknown wkbtype
        }

        f->setGeometry( g );
      }
      else // polygon
      {
        if ( layerWKBType == QGis::WKBPolygon ||  layerWKBType == QGis::WKBPolygon25D )
        {
          g = QgsGeometry::fromPolygon( QgsPolygon() << points().toVector() );
        }
        else if ( layerWKBType == QGis::WKBMultiPolygon ||  layerWKBType == QGis::WKBMultiPolygon25D )
        {
          g = QgsGeometry::fromMultiPolygon( QgsMultiPolygon() << ( QgsPolygon() << points().toVector() ) );
        }
        else
        {
          QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot add feature. Unknown WKB type" ) );
          stopCapturing();
          return; //unknown wkbtype
        }

        if ( !g )
        {
          stopCapturing();
          delete f;
          return; // invalid geometry; one possibility is from duplicate points
        }
        f->setGeometry( g );

        int avoidIntersectionsReturn = f->geometry()->avoidIntersections();
        if ( avoidIntersectionsReturn == 1 )
        {
          //not a polygon type. Impossible to get there
        }
#if 0
        else if ( avoidIntersectionsReturn == 2 ) //MH120131: disable this error message until there is a better way to cope with the single type / multi type problem
        {
          //bail out...
          QMessageBox::critical( 0, tr( "Error" ), tr( "The feature could not be added because removing the polygon intersections would change the geometry type" ) );
          delete f;
          stopCapturing();
          return;
        }
#endif
        else if ( avoidIntersectionsReturn == 3 )
        {
          QMessageBox::critical( 0, tr( "Error" ), tr( "An error was reported during intersection removal" ) );
        }

        if ( !f->geometry()->asWkb() ) //avoid intersection might have removed the whole geometry
        {
          QString reason;
          if ( avoidIntersectionsReturn != 2 )
          {
            reason = tr( "The feature cannot be added because it's geometry is empty" );
          }
          else
          {
            reason = tr( "The feature cannot be added because it's geometry collapsed due to intersection avoidance" );
          }
          QMessageBox::critical( 0, tr( "Error" ), reason );
          delete f;
          stopCapturing();
          return;
        }
      }

      vlayer->beginEditCommand( tr( "Feature added" ) );

      if ( addFeature( vlayer, f ) )
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
            if ( vl && vl->geometryType() == QGis::Polygon && vl->isEditable() )
            {
              vl->addTopologicalPoints( f->geometry() );
            }
          }
        }
        else if ( topologicalEditing )
        {
          vlayer->addTopologicalPoints( f->geometry() );
        }

        vlayer->endEditCommand();
      }
      else
      {
        delete f;
        vlayer->destroyEditCommand();
      }

      stopCapturing();
    }
  }
}
