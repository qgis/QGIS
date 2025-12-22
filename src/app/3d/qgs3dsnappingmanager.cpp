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
#include "qgsrubberband3d.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgswindow3dengine.h"

#include <qstringliteral.h>

Qgs3DSnappingManager::Qgs3DSnappingManager( double tolerance )
  : mType( SnappingType3D::Off )
  , mTolerance( tolerance )
{
}

void Qgs3DSnappingManager::start( Qgs3DMapCanvas *canvas )
{
  mCanvas = canvas;
  Q_ASSERT( mCanvas != nullptr );

  Qt3DCore::QEntity *highlightsRootEntity = mCanvas->engine()->frameGraph()->highlightsRootEntity();
  mHighlightedFeature.reset( new Qt3DCore::QEntity( highlightsRootEntity ) );
  mHighlightedFeature->setEnabled( false );

  Qt3DCore::QEntity *alwaysEntity = mCanvas->engine()->frameGraph()->rubberBandsRootEntity();
  mHighlightedPointBB.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), alwaysEntity, Qgis::GeometryType::Point ) );
  mHighlightedPointBB->setColor( QColor( 255, 0, 0, 150 ) );
  mHighlightedPointBB->setOutlineColor( QColor( 0, 255, 255 ) );
  mHighlightedPointBB->setWidth( 9.f );
  mHighlightedPointBB->setEnabled( false );
}

void Qgs3DSnappingManager::reset()
{
  if ( mHighlightedFeature )
  {
    // Create a new entity to delete the children
    Qt3DCore::QEntity *highlightsRootEntity = mCanvas->engine()->frameGraph()->highlightsRootEntity();
    mHighlightedFeature.reset( new Qt3DCore::QEntity( highlightsRootEntity ) );
    mHighlightedFeature->setEnabled( false );
  }
  if ( mHighlightedPointBB )
  {
    mHighlightedPointBB->reset();
    mHighlightedPointBB->setEnabled( false );
  }
  mHighlightedFeatureId = -1;
}

void Qgs3DSnappingManager::finish()
{
  reset();

  mHighlightedFeature.reset();
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

QgsPoint Qgs3DSnappingManager::screenToMap( const QPoint &screenPos, bool *success, bool highlightEntity, bool highlightSnappedPoint )
{
  QString layerId;
  QgsFeatureId nearestFid;
  SnappingType3D snapFound;

  QVector3D worldPoint = screenToWorld( screenPos, success, &snapFound, &layerId, &nearestFid );
  if ( !*success )
  {
    // Unable to compute position
    QgsDebugMsgLevel( QStringLiteral( "Qgs3DSnappingManager::screenToMap: Unable to compute position " ), 2 );
    reset();
    return QgsPoint();
  }

  if ( highlightSnappedPoint || highlightEntity )
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
            updateHighlightedEntities( layer, feature, highlightedPointInWorld, snapFound, highlightEntity, highlightSnappedPoint );
          }
          break;
        }
      }
    }
    else if ( mHighlightedFeatureId != -1 )
    {
      // Not Inside a feature anymore
      // clear all the highlights
      reset();
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
    const QString layer = hit.properties().value( QStringLiteral( "layerId" ), QString() ).toString();
    if ( layer == QLatin1String( "terrain" ) )
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

  *layerId = bestHit.properties().value( QStringLiteral( "layerId" ), QString() ).toString();
  *nearestFid = bestHit.properties().value( QStringLiteral( "fid" ), -1 ).toLongLong();
  if ( bestHit.properties().contains( QStringLiteral( "facePoint0" ) ) )
  {
    facePoints[0] = qvariant_cast<QVector3D>( bestHit.properties().value( QStringLiteral( "facePoint0" ) ) );
    facePoints[1] = qvariant_cast<QVector3D>( bestHit.properties().value( QStringLiteral( "facePoint1" ) ) );
    facePoints[2] = qvariant_cast<QVector3D>( bestHit.properties().value( QStringLiteral( "facePoint2" ) ) );
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

void Qgs3DSnappingManager::updateHighlightedEntities( QgsMapLayer *layer, const QgsFeature &feature, const QVector3D &highlightedPointInWorld, SnappingType3D snapFound, bool highlightEntity, bool highlightSnappedPoint )
{
  if ( mHighlightedFeatureId != feature.id() )
  {
    reset();

    if ( highlightEntity )
    {
      mHighlightedFeatureId = feature.id();

      QgsVectorLayer *vLayer = dynamic_cast<QgsVectorLayer *>( layer );
      if ( vLayer )
      {
        QgsAbstractVectorLayer3DRenderer *vectorRenderer = dynamic_cast<QgsAbstractVectorLayer3DRenderer *>( layer->renderer3D() );
        if ( vectorRenderer )
        {
          std::unique_ptr<QgsAbstractVectorLayer3DHighlightFactory> highlightFactory = vectorRenderer->createHighlightFactory( mCanvas->mapSettings() );
          QColor edgeColor;
          QColor fillColor;
          computeFeatureColors( fillColor, edgeColor );
          highlightFactory->create( feature, mCanvas->engine(), mHighlightedFeature.get(), fillColor, edgeColor );
          mHighlightedFeature->setEnabled( true );
        }
      }
    }
  } // end if feature id changed

  if ( highlightSnappedPoint && mType != SnappingType3D::Off && mPreviousHighlightedPoint != highlightedPointInWorld )
  {
    updateHighlightedPoint( highlightedPointInWorld, snapFound );
  }
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
      // HL nearest vertex

      QgsVector3D mapPoint = Qgs3DUtils::worldToMapCoordinates( highlightedPointInWorld, mCanvas->mapSettings()->origin() );
      mHighlightedPointBB->reset();
      mHighlightedPointBB->addPoint( QgsPoint( mapPoint.x(), mapPoint.y(), mapPoint.z() ) );
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
    }
  }
}

void Qgs3DSnappingManager::computeFeatureColors( QColor &backgroundColor, QColor &edgeColor )
{
  const QgsSettings settings;
  const int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  const int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  const int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();

  const QColor baseColor( myRed, myGreen, myBlue );
  const int h = baseColor.hue();
  const int s = baseColor.saturation();
  const int l = baseColor.lightness();

  int safeH = ( h < 0 ) ? 0 : h;

  // compute background color
  // greay shade
  int bgL = ( l < 140 ) ? 245 : 25;
  int bgS = std::min( s, 20 );
  backgroundColor.setHsl( safeH, bgS, bgL );

  if ( h < 0 )
  {
    // Bright yellow on a dark background, dark magenta on a light background.
    edgeColor = ( bgL < 128 ) ? QColor( 255, 255, 0 ) : QColor( 139, 0, 139 );
  }
  else
  {
    int outlineH = ( h + 180 ) % 360;
    // Maximum saturation for a "flash" selection effect.
    int outlineS = 255;
    // Brightness opposite to the background to ensure the edge remains visible.
    int outlineL = ( bgL < 128 ) ? 180 : 70;
    edgeColor.setHsl( outlineH, outlineS, outlineL );
  }
}
