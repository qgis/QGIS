/***************************************************************************
  qgs3dsnappingmanager.cpp
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsnappingmanager.h"

#include "qgs3dmapcanvas.h"
#include "qgs3dmapcanvaswidget.h"
#include "qgs3dmapscene.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturesource.h"
#include "qgsframegraph.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsraycastcontext.h"
#include "qgsraycasthit.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgswindow3dengine.h"

Qgs3DSnappingManager::Qgs3DSnappingManager( double tolerance )
  : mType( SnappingType3D::Off )
  , mTolerance( tolerance )
{
}

void Qgs3DSnappingManager::start( Qgs3DMapCanvas *canvas )
{
  mCanvas = canvas;
  Q_ASSERT( mCanvas );

  Qt3DCore::QEntity *rubberBandRootEntity = mCanvas->engine()->frameGraph()->rubberBandsRootEntity();
  mHighlightedPointBB.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), rubberBandRootEntity, Qgis::GeometryType::Point ) );
  mHighlightedPointBB->setColor( QColor( 255, 0, 0, 150 ) );
  mHighlightedPointBB->setOutlineColor( QColor( 0, 255, 255 ) );
  mHighlightedPointBB->setWidth( 9.f );
  mHighlightedPointBB->setEnabled( false );
}

void Qgs3DSnappingManager::reset()
{
  if ( mHighlightedPointBB )
  {
    mHighlightedPointBB->reset();
    mHighlightedPointBB->setEnabled( false );
  }
}

void Qgs3DSnappingManager::finish()
{
  reset();

  mHighlightedPointBB.reset();
}

void Qgs3DSnappingManager::setSnappingType( SnappingType3D mode )
{
  mType = mode;
}

void Qgs3DSnappingManager::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

QgsPoint Qgs3DSnappingManager::screenToMap( const QPoint &screenPos, bool *success, bool highlightSnappedPoint )
{
  QString layerId;
  QgsFeatureId nearestFid;
  SnappingType3D snapFound;

  QVector3D worldPoint = screenToWorld( screenPos, success, &snapFound, &layerId, &nearestFid );
  if ( !*success )
  {
    // Unable to compute position
    QgsDebugMsgLevel( u"Qgs3DSnappingManager::screenToMap: Unable to compute position"_s, 2 );
    reset();
    return QgsPoint();
  }

  if ( highlightSnappedPoint )
  {
    if ( !layerId.isEmpty() && nearestFid > 0 && nearestFid < std::numeric_limits<int>::max() )
    {
      // Inside a feature
      // Highlight the feature and display a symbol if a snapPoint was found
      const QList<QgsMapLayer *> layers = mCanvas->scene()->layers();
      for ( QgsMapLayer *layer : layers )
      {
        QgsFeatureSource *featureLayer = dynamic_cast<QgsFeatureSource *>( layer );
        if ( featureLayer && layer->id() == layerId )
        {
          QgsFeatureRequest req( nearestFid );
          req.setCoordinateTransform( QgsCoordinateTransform( layer->crs3D(), mCanvas->mapSettings()->crs(), mCanvas->mapSettings()->transformContext() ) );
          QgsFeatureIterator ite = featureLayer->getFeatures( req );
          QgsFeature feature;
          if ( ite.nextFeature( feature ) )
          {
            const QVector3D highlightedPointInWorld = snapFound != SnappingType3D::Off ? worldPoint : QVector3D();
            updateHighlightedPoint( highlightedPointInWorld, snapFound );
          }
          break;
        }
      }
    }
  }
  else
  {
    reset();
  }

  QgsVector3D mapPoint = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldPoint ), mCanvas->mapSettings()->origin() );
  return QgsPoint( mapPoint.x(), mapPoint.y(), mapPoint.z() );
}

QVector3D Qgs3DSnappingManager::screenToWorld( const QPoint &screenPos, bool *success, SnappingType3D *snapFound, QString *layerId, QgsFeatureId *nearestFid ) const
{
  *success = false;
  *snapFound = SnappingType3D::Off;
  *layerId = QString();
  *nearestFid = QgsFeatureId();

  QgsRayCastContext context;
  context.setSingleResult( false );
  context.setMaximumDistance( mCanvas->cameraController()->camera()->farPlane() );
  context.setAngleThreshold( 1.0f );
  const QgsRayCastResult results = mCanvas->castRay( screenPos, context );

  if ( results.isEmpty() )
  {
    return QVector3D();
  }

  QVector3D facePoints[3];
  QgsVector3D mapCoords;
  double minDist = -1;
  const QList<QgsRayCastHit> allHits = results.allHits();
  QgsRayCastHit hitLayer;
  QgsRayCastHit hitTerrain;
  bool hitLayerFound = false;
  for ( const QgsRayCastHit &hit : allHits )
  {
    const double resDist = hit.distance();
    const QString layer = hit.properties().value( u"layerId"_s, QString() ).toString();
    if ( layer == "terrain"_L1 )
    {
      hitTerrain = hit;
    }
    else if ( minDist < 0 || resDist < minDist )
    {
      minDist = resDist;
      hitLayer = hit;
      hitLayerFound = true;
    }
  }

  const QgsRayCastHit bestHit = hitLayerFound ? hitLayer : hitTerrain;

  *layerId = bestHit.properties().value( u"layerId"_s, QString() ).toString();
  *nearestFid = bestHit.properties().value( u"fid"_s, -1 ).toLongLong();
  if ( bestHit.properties().contains( u"facePoint0"_s ) )
  {
    facePoints[0] = qvariant_cast<QVector3D>( bestHit.properties().value( u"facePoint0"_s ) );
    facePoints[1] = qvariant_cast<QVector3D>( bestHit.properties().value( u"facePoint1"_s ) );
    facePoints[2] = qvariant_cast<QVector3D>( bestHit.properties().value( u"facePoint2"_s ) );
  }

  mapCoords = bestHit.mapCoordinates();
  if ( std::isnan( mapCoords.z() ) )
  {
    mapCoords.setZ( 0.0 );
  }

  QVector3D worldPoint = Qgs3DUtils::mapToWorldCoordinates( mapCoords, mCanvas->mapSettings()->origin() ).toVector3D();
  QVector3D outPoint = worldPoint;
  float minSnapDistance = static_cast<float>( mTolerance );

  if ( mType & SnappingType3D::Vertex )
  {
    const QVector3D snapPoint = facePoints[0];
    const float snapDistance = ( snapPoint - worldPoint ).length();
    if ( snapDistance < minSnapDistance )
    {
      *snapFound = SnappingType3D::Vertex;
      minSnapDistance = snapDistance;
      outPoint = snapPoint;
    }
  }
  if ( mType & SnappingType3D::AlongEdge )
  {
    const QVector3D origin = facePoints[0];
    const QVector3D dest = facePoints[1];
    const QVector3D direction = ( dest - origin ).normalized();
    const QVector3D snapPoint = origin + QVector3D::dotProduct( worldPoint - origin, direction ) * direction;

    if ( QVector3D::dotProduct( dest - origin, snapPoint - origin ) >= 0 && QVector3D::dotProduct( origin - dest, snapPoint - dest ) >= 0 )
    {
      const float snapDistance = ( snapPoint - worldPoint ).length();
      if ( snapDistance < minSnapDistance )
      {
        *snapFound = SnappingType3D::AlongEdge;
        minSnapDistance = snapDistance;
        outPoint = snapPoint;
      }
    }
    // else outside segment
  }
  if ( mType & SnappingType3D::MiddleEdge )
  {
    const QVector3D snapPoint = ( facePoints[0] + facePoints[1] ) / 2.0;
    const float snapDistance = ( snapPoint - worldPoint ).length();
    if ( snapDistance < minSnapDistance )
    {
      *snapFound = SnappingType3D::MiddleEdge;
      minSnapDistance = snapDistance;
      outPoint = snapPoint;
    }
  }
  if ( static_cast<int>( mType & Qgs3DSnappingManager::SnappingType3D::CenterFace ) != 0 )
  {
    const QVector3D snapPoint = ( facePoints[0] + facePoints[1] + facePoints[2] ) / 3.0;
    const float snapDistance = ( snapPoint - worldPoint ).length();
    if ( snapDistance < minSnapDistance )
    {
      *snapFound = SnappingType3D::CenterFace;
      minSnapDistance = snapDistance;
      outPoint = snapPoint;
    }
  }

  *success = true;
  return outPoint;
}

void Qgs3DSnappingManager::updateHighlightedPoint( const QVector3D &highlightedPointInWorld, SnappingType3D snapFound )
{
  if ( mType != SnappingType3D::Off && mPreviousHighlightedPoint != highlightedPointInWorld )
  {
    mPreviousHighlightedPoint = highlightedPointInWorld;
    // Remove the snap billboard entity if necessary
    if ( highlightedPointInWorld.isNull() )
    {
      mHighlightedPointBB->reset();
      mHighlightedPointBB->setEnabled( false );
    }
    else
    {
      // Highlight nearest vertex
      QgsVector3D mapPoint = Qgs3DUtils::worldToMapCoordinates( highlightedPointInWorld, mCanvas->mapSettings()->origin() );
      mHighlightedPointBB->reset();
      mHighlightedPointBB->addPoint( QgsPoint( mapPoint.x(), mapPoint.y(), mapPoint.z() ) );
      // NOLINTBEGIN(bugprone-branch-clone)
      switch ( snapFound )
      {
        case SnappingType3D::Vertex:
          mHighlightedPointBB->setMarkerShape( Qgis::MarkerShape::Square );
          break;
        case SnappingType3D::MiddleEdge:
          mHighlightedPointBB->setMarkerShape( Qgis::MarkerShape::Triangle );
          break;
        case SnappingType3D::AlongEdge:
          mHighlightedPointBB->setMarkerShape( Qgis::MarkerShape::Cross2 );
          break;
        case SnappingType3D::CenterFace:
          mHighlightedPointBB->setMarkerShape( Qgis::MarkerShape::Circle );
          break;
        case SnappingType3D::Off:
          break;
      }
      mHighlightedPointBB->setEnabled( true );
      // NOLINTEND(bugprone-branch-clone)
    }
  }
}
