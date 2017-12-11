/***************************************************************************
  qgsmaptooldigitizefeature.cpp

 ---------------------
 begin                : 7.12.2017
 copyright            : (C) 2017 by David Signer
 email                : david@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldigitizefeature.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgsexception.h"
#include "qgscurvepolygon.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsmapcanvas.h"
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

QgsMapToolDigitizeFeature::QgsMapToolDigitizeFeature( QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mCheckGeometryType( true )
{
  mToolName = tr( "Digitize feature" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolDigitizeFeature::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolDigitizeFeature::stopCapturing );
}

void QgsMapToolDigitizeFeature::digitized( QgsFeature *f )
{
  emit digitizingFinished( static_cast< const QgsFeature & >( *f ) );
}

void QgsMapToolDigitizeFeature::activate()
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( vlayer && vlayer->geometryType() == QgsWkbTypes::NullGeometry )
  {
    QgsFeature f;
    digitized( &f );
    emit digitizingFinalized();
    return;
  }

  //refresh the layer, with the current layer - so capturemode will be set at activate
  canvas()->setCurrentLayer( canvas()->currentLayer() );

  QgsMapToolCapture::activate();
}

void QgsMapToolDigitizeFeature::deactivate()
{
  QgsMapToolCapture::deactivate();
  emit digitizingAborted();
}

bool QgsMapToolDigitizeFeature::checkGeometryType() const
{
  return mCheckGeometryType;
}

void QgsMapToolDigitizeFeature::setCheckGeometryType( bool checkGeometryType )
{
  mCheckGeometryType = checkGeometryType;
}

void QgsMapToolDigitizeFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsVectorLayer *vlayer = currentVectorLayer();

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  QgsWkbTypes::Type layerWKBType = vlayer->wkbType();

  QgsVectorDataProvider *provider = vlayer->dataProvider();

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



    QgsPointXY savePoint; //point in layer coordinates
    try
    {
      QgsPoint fetchPoint;
      int res;
      res = fetchLayerPoint( e->mapPointMatch(), fetchPoint );
      if ( res == 0 )
      {
        savePoint = QgsPointXY( fetchPoint.x(), fetchPoint.y() );
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
        g = QgsGeometry::fromPointXY( savePoint );
      }
      else if ( !QgsWkbTypes::isMultiType( layerWKBType ) && QgsWkbTypes::hasZ( layerWKBType ) )
      {
        g = QgsGeometry( new QgsPoint( QgsWkbTypes::PointZ, savePoint.x(), savePoint.y(), defaultZValue() ) );
      }
      else if ( QgsWkbTypes::isMultiType( layerWKBType ) && !QgsWkbTypes::hasZ( layerWKBType ) )
      {
        g = QgsGeometry::fromMultiPointXY( QgsMultiPointXY() << savePoint );
      }
      else if ( QgsWkbTypes::isMultiType( layerWKBType ) && QgsWkbTypes::hasZ( layerWKBType ) )
      {
        QgsMultiPoint *mp = new QgsMultiPoint();
        mp->addGeometry( new QgsPoint( QgsWkbTypes::PointZ, savePoint.x(), savePoint.y(), defaultZValue() ) );
        g = QgsGeometry( mp );
      }
      else
      {
        // if layer supports more types (mCheckGeometryType is false)
        g = QgsGeometry::fromPointXY( savePoint );
      }

      if ( QgsWkbTypes::hasM( layerWKBType ) )
      {
        g.get()->addMValue();
      }

      f.setGeometry( g );
      f.setValid( true );

      digitized( &f );

      // we are done with digitizing for now so instruct advanced digitizing dock to reset its CAD points
      cadDockWidget()->clearPoints();

      emit digitizingFinalized();
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
      std::unique_ptr< QgsFeature > f( new QgsFeature( vlayer->fields(), 0 ) );

      //does compoundcurve contain circular strings?
      //does provider support circular strings?
      bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
      bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::CircularGeometries;

      QList<QgsPointLocator::Match> snappingMatchesList;
      QgsCurve *curveToAdd = nullptr;
      if ( hasCurvedSegments && providerSupportsCurvedSegments )
      {
        curveToAdd = captureCurve()->clone();
      }
      else
      {
        curveToAdd = captureCurve()->curveToLine();
        snappingMatchesList = snappingMatches();
      }

      if ( mode() == CaptureLine )
      {
        QgsGeometry g( curveToAdd );
        f->setGeometry( g );
      }
      else
      {
        QgsCurvePolygon *poly = nullptr;
        if ( hasCurvedSegments && providerSupportsCurvedSegments )
        {
          poly = new QgsCurvePolygon();
        }
        else
        {
          poly = new QgsPolygon();
        }
        poly->setExteriorRing( curveToAdd );
        QgsGeometry g( poly );
        f->setGeometry( g );

        QgsGeometry featGeom = f->geometry();
        int avoidIntersectionsReturn = featGeom.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );
        f->setGeometry( featGeom );
        if ( avoidIntersectionsReturn == 1 )
        {
          //not a polygon type. Impossible to get there
        }
        if ( f->geometry().isEmpty() ) //avoid intersection might have removed the whole geometry
        {
          emit messageEmitted( tr( "The feature cannot be added because it's geometry collapsed due to intersection avoidance" ), QgsMessageBar::CRITICAL );
          stopCapturing();
          return;
        }
      }
      f->setValid( true );

      digitized( f.get() );

      stopCapturing();

      emit digitizingFinalized();
    }
  }
}
