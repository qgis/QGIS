/***************************************************************************
    qgsmaptoolcapturelayergeometry.cpp  -  base class for map tools digitizing layer geometries
    ---------------------
    begin                : January 2022
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaptoolcapturelayergeometry.h"
#include "moc_qgsmaptoolcapturelayergeometry.cpp"
#include "qgsproject.h"
#include "qgscurvepolygon.h"
#include "qgscurve.h"


void QgsMapToolCaptureLayerGeometry::geometryCaptured( const QgsGeometry &geometry )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
  if ( !vlayer )
    return;

  QgsGeometry g( geometry );

  switch ( mode() )
  {
    case QgsMapToolCapture::CaptureNone:
    case QgsMapToolCapture::CapturePoint:
      break;
    case QgsMapToolCapture::CaptureLine:
    case QgsMapToolCapture::CapturePolygon:
      //does provider support circular strings?
      const bool datasetIsCurved = QgsWkbTypes::isCurvedType( vlayer->wkbType() );
      const bool providerSupportsCurvedSegments = vlayer && ( vlayer->dataProvider()->capabilities() & Qgis::VectorProviderCapability::CircularGeometries );
      if ( !datasetIsCurved || !providerSupportsCurvedSegments )
        g = QgsGeometry( g.constGet()->segmentize() );

      QList<QgsVectorLayer *> avoidIntersectionsLayers;
      switch ( QgsProject::instance()->avoidIntersectionsMode() )
      {
        case Qgis::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
          if ( vlayer )
            avoidIntersectionsLayers.append( vlayer );
          break;
        case Qgis::AvoidIntersectionsMode::AvoidIntersectionsLayers:
          avoidIntersectionsLayers = QgsProject::instance()->avoidIntersectionsLayers();
          break;
        case Qgis::AvoidIntersectionsMode::AllowIntersections:
          break;
      }
      if ( avoidIntersectionsLayers.size() > 0 )
      {
        const Qgis::GeometryOperationResult avoidIntersectionsReturn = g.avoidIntersectionsV2( avoidIntersectionsLayers );
        if ( avoidIntersectionsReturn == Qgis::GeometryOperationResult::InvalidBaseGeometry )
        {
          emit messageEmitted( tr( "The feature has been added, but at least one geometry intersected is invalid. These geometries must be manually repaired." ), Qgis::MessageLevel::Warning );
        }
        if ( g.isEmpty() ) //avoid intersection might have removed the whole geometry
        {
          emit messageEmitted( tr( "The feature cannot be added because its geometry collapsed due to intersection avoidance" ), Qgis::MessageLevel::Critical );
          stopCapturing();
          return;
        }
      }
      break;
  }

  layerGeometryCaptured( g );

  switch ( mode() )
  {
    case QgsMapToolCapture::CaptureNone:
      break;
    case QgsMapToolCapture::CapturePoint:
      layerPointCaptured( *qgsgeometry_cast<const QgsPoint *>( g.constGet() ) );
      break;
    case QgsMapToolCapture::CaptureLine:
      layerLineCaptured( qgsgeometry_cast<const QgsCurve *>( g.constGet() ) );
      break;
    case QgsMapToolCapture::CapturePolygon:
      layerPolygonCaptured( qgsgeometry_cast<const QgsCurvePolygon *>( g.constGet() ) );
      break;
  }
}
