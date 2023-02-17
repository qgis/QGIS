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
  QVector<QPair<QgsMapLayer *, QVector<QVariantMap>>> layerPoints;
  Qgs3DMapCanvas *canvas = this->canvas();

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), canvas->windowSize(), canvas->cameraController()->camera() );

  QHash<QgsPointCloudLayer *, QVector<IndexedPointCloudNode>> layerChunks;
  for ( QgsMapLayer *layer : canvas->map()->layers() )
  {
    if ( QgsPointCloudLayer *pc = qobject_cast<QgsPointCloudLayer *>( layer ) )
    {
      QVector<IndexedPointCloudNode> pointCloudNodes;
      for ( const QgsChunkNode *n : canvas->scene()->getLayerActiveChunkNodes( pc ) )
      {
        const QgsChunkNodeId id = n->tileId();
        pointCloudNodes.push_back( IndexedPointCloudNode( id.d, id.x, id.y, id.z ) );
      }
      if ( pointCloudNodes.empty() )
        continue;
      layerChunks[ pc ] = pointCloudNodes;
    }
  }

  for ( auto it = layerChunks.constBegin(); it != layerChunks.constEnd(); ++it )
  {
    QgsPointCloudLayer *layer = it.key();
    // transform ray
    const QgsVector3D originMapCoords = canvas->map()->worldToMapCoordinates( ray.origin() );
    const QgsVector3D pointMapCoords = canvas->map()->worldToMapCoordinates( ray.origin() + ray.origin().length() * ray.direction().normalized() );
    QgsVector3D directionMapCoords = pointMapCoords - originMapCoords;
    directionMapCoords.normalize();

    const QVector3D rayOriginMapCoords( originMapCoords.x(), originMapCoords.y(), originMapCoords.z() );
    const QVector3D rayDirectionMapCoords( directionMapCoords.x(), directionMapCoords.y(), directionMapCoords.z() );

    const QSize size = canvas->engine()->size();
    const int screenSizePx = std::max( size.width(), size.height() ); // TODO: is this correct? (see sceneState_)
    QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() );
    const QgsPointCloud3DSymbol *symbol = renderer->symbol();
    // Symbol can be null in case of no rendering enabled
    if ( !symbol )
      continue;
    const double pointSize = symbol->pointSize();
    const double limitAngle = 2 * pointSize / screenSizePx * canvas->cameraController()->camera()->fieldOfView();

    // adjust ray to elevation properties
    QgsPointCloudLayerElevationProperties *elevationProps = dynamic_cast<QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() );
    const QVector3D adjutedRayOrigin = QVector3D( rayOriginMapCoords.x(), rayOriginMapCoords.y(), ( rayOriginMapCoords.z() -  elevationProps->zOffset() ) / elevationProps->zScale() );
    QVector3D adjutedRayDirection = QVector3D( rayDirectionMapCoords.x(), rayDirectionMapCoords.y(), rayDirectionMapCoords.z() / elevationProps->zScale() );
    adjutedRayDirection.normalize();

    const QgsRay3D layerRay( adjutedRayOrigin, adjutedRayDirection );

    QgsPointCloudDataProvider *provider = layer->dataProvider();
    QgsPointCloudIndex *index = provider->index();
    QVector<QVariantMap> points;
    const QgsPointCloudAttributeCollection attributeCollection = index->attributes();
    QgsPointCloudRequest request;
    request.setAttributes( attributeCollection );
    for ( const IndexedPointCloudNode &n : it.value() )
    {
      if ( !index->hasNode( n ) )
        continue;
      std::unique_ptr<QgsPointCloudBlock> block( index->nodeData( n, request ) );
      if ( !block )
        continue;

      const QgsVector3D blockScale = block->scale();
      const QgsVector3D blockOffset = block->offset();

      const char *ptr = block->data();
      const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
      const std::size_t recordSize = blockAttributes.pointRecordSize();
      int xOffset = 0, yOffset = 0, zOffset = 0;
      const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
      const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
      const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
      for ( int i = 0; i < block->pointCount(); ++i )
      {
        double x, y, z;
        QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );
        const QVector3D point( x, y, z );

        // check whether point is in front of the ray
        if ( !layerRay.isInFront( point ) )
          continue;

        // calculate the angle between the point and the projected point
        if ( layerRay.angleToPoint( point ) > limitAngle )
          continue;

        // Note : applying elevation properties is done in fromPointCloudIdentificationToIdentifyResults
        QVariantMap pointAttr = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, blockAttributes );
        pointAttr[ QStringLiteral( "X" ) ] = x;
        pointAttr[ QStringLiteral( "Y" ) ] = y;
        pointAttr[ QStringLiteral( "Z" ) ] = z;
        pointAttr[ tr( "Distance to camera" ) ] = ( point - layerRay.origin() ).length();
        points.push_back( pointAttr );
      }

    }
    layerPoints.push_back( qMakePair( layer, points ) );
  }

  QList<QgsMapToolIdentify::IdentifyResult> identifyResults;
  for ( int i = 0; i < layerPoints.size(); ++i )
  {
    QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layerPoints[i].first );
    QgsMapToolIdentify::fromPointCloudIdentificationToIdentifyResults( pcLayer, layerPoints[i].second, identifyResults );
  }

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->showIdentifyResults( identifyResults );
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
