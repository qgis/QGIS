/***************************************************************************
  qgs3dmaptoolidentify.cpp
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolidentify.h"

#include "qgsapplication.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsterrainentity_p.h"
#include "qgsvector3d.h"

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentifyaction.h"

#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudlayer3drenderer.h"

#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgswindow3dengine.h"


Qgs3DMapToolIdentify::Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolIdentify::~Qgs3DMapToolIdentify() = default;


void Qgs3DMapToolIdentify::mousePressEvent( QMouseEvent *event )
{
  Q_UNUSED( event )

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();
}

void Qgs3DMapToolIdentify::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::MouseButton::LeftButton )
    return;

  // point cloud identification
  Qgs3DMapCanvas *canvas = this->canvas();

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), canvas->windowSize(), canvas->cameraController()->camera() );
  const QVector<RayHit> allHits = mCanvas->scene()->castRay( ray );


  QMap<QgsMapLayer *, QVector<QVariantMap>> pointCloudResults;

  QList<QgsMapToolIdentify::IdentifyResult> identifyResults;
  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();

  bool showTerrainResults = true;

  for ( const auto &hit : allHits )
  {
    if ( !pointCloudResults.contains( hit.layer ) &&
         qobject_cast<QgsPointCloudLayer * >( hit.layer ) )
    {
      pointCloudResults[ hit.layer ] = QVector<QVariantMap>();
    }

    if ( qobject_cast<QgsPointCloudLayer * >( hit.layer ) )
    {
      pointCloudResults[ hit.layer ].append( hit.attributes );
      showTerrainResults = false;
    }
    else if ( hit.layer && !FID_IS_NULL( hit.fid ) )
    {
      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->map()->origin() );
      const QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer * >( hit.layer );
      identifyTool2D->showResultsForFeature( vLayer, hit.fid, pt );
      showTerrainResults = false;
    }
    else if ( showTerrainResults )
    {
      // estimate search radius
      Qgs3DMapScene *scene = mCanvas->scene();
      const double searchRadiusMM = QgsMapTool::searchRadiusMM();
      const double pixelsPerMM = mCanvas->logicalDpiX() / 25.4;
      const double searchRadiusPx = searchRadiusMM * pixelsPerMM;
      const double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, hit.distance );

      QgsMapCanvas *canvas2D = identifyTool2D->canvas();

      // transform the point and search radius to CRS of the map canvas (if they are different)
      const QgsCoordinateTransform ct( mCanvas->map()->crs(), canvas2D->mapSettings().destinationCrs(), canvas2D->mapSettings().transformContext() );

      const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hit.pos, mCanvas->map()->origin() );
      const QgsPointXY mapPoint( mapCoords.x(), mapCoords.y() );

      QgsPointXY mapPointCanvas2D = mapPoint;
      double searchRadiusCanvas2D = searchRadiusMapUnits;
      try
      {
        mapPointCanvas2D = ct.transform( mapPoint );
        const QgsPointXY mapPointSearchRadius( mapPoint.x() + searchRadiusMapUnits, mapPoint.y() );
        const QgsPointXY mapPointSearchRadiusCanvas2D = ct.transform( mapPointSearchRadius );
        searchRadiusCanvas2D = mapPointCanvas2D.distance( mapPointSearchRadiusCanvas2D );
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e )
        QgsDebugMsg( QStringLiteral( "Caught exception %1" ).arg( e.what() ) );
      }

      identifyTool2D->identifyAndShowResults( QgsGeometry::fromPointXY( mapPointCanvas2D ), searchRadiusCanvas2D );
    }
  }




  for ( auto it = pointCloudResults.constKeyValueBegin(); it != pointCloudResults.constKeyValueEnd(); ++it )
  {
    if ( QgsPointCloudLayer *pcLayer = qobject_cast<QgsPointCloudLayer * >( ( *it ).first ) )
    {
      QgsMapToolIdentify::fromPointCloudIdentificationToIdentifyResults( pcLayer, ( *it ).second, identifyResults );
      identifyTool2D->showIdentifyResults( identifyResults );
    }
  }




}

void Qgs3DMapToolIdentify::activate()
{
  mIsActive = true;
}

void Qgs3DMapToolIdentify::deactivate()
{
  mIsActive = false;
}

QCursor Qgs3DMapToolIdentify::cursor() const
{
  return QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify );
}
