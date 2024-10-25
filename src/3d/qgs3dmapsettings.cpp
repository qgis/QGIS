/***************************************************************************
  qgs3dmapsettings.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapsettings.h"

#include "qgs3dutils.h"
#include "qgsflatterraingenerator.h"
#include "qgsdemterraingenerator.h"
#include "qgsmeshterraingenerator.h"
#include "qgsonlineterraingenerator.h"
#include "qgsprojectviewsettings.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgslightsource.h"
#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"
#include "qgspointlightsettings.h"
#include "qgsdirectionallightsettings.h"
#include "qgsmaplayerlistutils_p.h"

#include <QDomDocument>
#include <QDomElement>


Qgs3DMapSettings::Qgs3DMapSettings()
  : QObject( nullptr )
{
  connect( this, &Qgs3DMapSettings::settingsChanged, [&]()
  {
    QgsProject::instance()->setDirty();
  } );
  connectChangedSignalsToSettingsChanged();
}

Qgs3DMapSettings::Qgs3DMapSettings( const Qgs3DMapSettings &other )
  : QObject( nullptr )
  , QgsTemporalRangeObject( other )
  , mOrigin( other.mOrigin )
  , mCrs( other.mCrs )
  , mBackgroundColor( other.mBackgroundColor )
  , mSelectionColor( other.mSelectionColor )
  , mTerrainVerticalScale( other.mTerrainVerticalScale )
  , mTerrainGenerator( other.mTerrainGenerator ? other.mTerrainGenerator->clone() : nullptr )
  , mMapTileResolution( other.mMapTileResolution )
  , mMaxTerrainScreenError( other.mMaxTerrainScreenError )
  , mMaxTerrainGroundError( other.mMaxTerrainGroundError )
  , mTerrainElevationOffset( other.mTerrainElevationOffset )
  , mTerrainShadingEnabled( other.mTerrainShadingEnabled )
  , mTerrainShadingMaterial( other.mTerrainShadingMaterial )
  , mTerrainMapTheme( other.mTerrainMapTheme )
  , mShowTerrainBoundingBoxes( other.mShowTerrainBoundingBoxes )
  , mShowTerrainTileInfo( other.mShowTerrainTileInfo )
  , mShowCameraViewCenter( other.mShowCameraViewCenter )
  , mShowCameraRotationCenter( other.mShowCameraRotationCenter )
  , mShowLightSources( other.mShowLightSources )
  , mShowLabels( other.mShowLabels )
  , mFieldOfView( other.mFieldOfView )
  , mProjectionType( other.mProjectionType )
  , mCameraNavigationMode( other.mCameraNavigationMode )
  , mCameraMovementSpeed( other.mCameraMovementSpeed )
  , mLayers( other.mLayers )
  , mTransformContext( other.mTransformContext )
  , mPathResolver( other.mPathResolver )
  , mMapThemes( other.mMapThemes )
  , mDpi( other.mDpi )
  , mIsFpsCounterEnabled( other.mIsFpsCounterEnabled )
  , mIsSkyboxEnabled( other.mIsSkyboxEnabled )
  , mSkyboxSettings( other.mSkyboxSettings )
  , mShadowSettings( other.mShadowSettings )
  , mAmbientOcclusionSettings( other.mAmbientOcclusionSettings )
  , mEyeDomeLightingEnabled( other.mEyeDomeLightingEnabled )
  , mEyeDomeLightingStrength( other.mEyeDomeLightingStrength )
  , mEyeDomeLightingDistance( other.mEyeDomeLightingDistance )
  , mViewSyncMode( other.mViewSyncMode )
  , mVisualizeViewFrustum( other.mVisualizeViewFrustum )
  , mDebugShadowMapEnabled( other.mDebugShadowMapEnabled )
  , mDebugShadowMapCorner( other.mDebugShadowMapCorner )
  , mDebugShadowMapSize( other.mDebugShadowMapSize )
  , mDebugDepthMapEnabled( other.mDebugDepthMapEnabled )
  , mDebugDepthMapCorner( other.mDebugDepthMapCorner )
  , mDebugDepthMapSize( other.mDebugDepthMapSize )
  , mTerrainRenderingEnabled( other.mTerrainRenderingEnabled )
  , mRendererUsage( other.mRendererUsage )
  , m3dAxisSettings( other.m3dAxisSettings )
  , mIsDebugOverlayEnabled( other.mIsDebugOverlayEnabled )
  , mExtent( other.mExtent )
  , mShowExtentIn2DView( other.mShowExtentIn2DView )
{
  for ( QgsLightSource *source : std::as_const( other.mLightSources ) )
  {
    if ( source )
      mLightSources << source->clone();
  }

  connect( this, &Qgs3DMapSettings::settingsChanged, [&]()
  {
    QgsProject::instance()->setDirty();
  } );
  connectChangedSignalsToSettingsChanged();
}

Qgs3DMapSettings::~Qgs3DMapSettings()
{
  qDeleteAll( mLightSources );
}

void Qgs3DMapSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsProjectDirtyBlocker blocker( QgsProject::instance() );
  QDomElement elemOrigin = elem.firstChildElement( QStringLiteral( "origin" ) );
  mOrigin = QgsVector3D(
              elemOrigin.attribute( QStringLiteral( "x" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "y" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "z" ) ).toDouble() );

  QDomElement elemExtent = elem.firstChildElement( QStringLiteral( "extent" ) );
  if ( !elemExtent.isNull() )
  {
    mExtent = QgsRectangle(
                elemExtent.attribute( QStringLiteral( "xMin" ) ).toDouble(),
                elemExtent.attribute( QStringLiteral( "yMin" ) ).toDouble(),
                elemExtent.attribute( QStringLiteral( "xMax" ) ).toDouble(),
                elemExtent.attribute( QStringLiteral( "yMax" ) ).toDouble() );

    mShowExtentIn2DView = elemExtent.attribute( QStringLiteral( "showIn2dView" ), QStringLiteral( "0" ) ).toInt();
  }
  else
  {
    mExtent = QgsProject::instance()->viewSettings()->fullExtent();
  }

  QDomElement elemCamera = elem.firstChildElement( QStringLiteral( "camera" ) );
  if ( !elemCamera.isNull() )
  {
    mFieldOfView = elemCamera.attribute( QStringLiteral( "field-of-view" ), QStringLiteral( "45" ) ).toFloat();
    mProjectionType = static_cast< Qt3DRender::QCameraLens::ProjectionType >( elemCamera.attribute( QStringLiteral( "projection-type" ), QStringLiteral( "1" ) ).toInt() );
    QString cameraNavigationMode = elemCamera.attribute( QStringLiteral( "camera-navigation-mode" ), QStringLiteral( "basic-navigation" ) );
    if ( cameraNavigationMode == QLatin1String( "terrain-based-navigation" ) )
      mCameraNavigationMode = Qgis::NavigationMode::TerrainBased;
    else if ( cameraNavigationMode == QLatin1String( "walk-navigation" ) )
      mCameraNavigationMode = Qgis::NavigationMode::Walk;
    mCameraMovementSpeed = elemCamera.attribute( QStringLiteral( "camera-movement-speed" ), QStringLiteral( "5.0" ) ).toDouble();
  }

  QDomElement elemColor = elem.firstChildElement( QStringLiteral( "color" ) );
  if ( !elemColor.isNull() )
  {
    mBackgroundColor = QgsSymbolLayerUtils::decodeColor( elemColor.attribute( QStringLiteral( "background" ) ) );
    mSelectionColor = QgsSymbolLayerUtils::decodeColor( elemColor.attribute( QStringLiteral( "selection" ) ) );
  }

  QDomElement elemCrs = elem.firstChildElement( QStringLiteral( "crs" ) );
  mCrs.readXml( elemCrs );

  QDomElement elemTerrain = elem.firstChildElement( QStringLiteral( "terrain" ) );
  mTerrainRenderingEnabled = elemTerrain.attribute( QStringLiteral( "terrain-rendering-enabled" ), QStringLiteral( "1" ) ).toInt();
  mTerrainVerticalScale = elemTerrain.attribute( QStringLiteral( "exaggeration" ), QStringLiteral( "1" ) ).toFloat();
  mMapTileResolution = elemTerrain.attribute( QStringLiteral( "texture-size" ), QStringLiteral( "512" ) ).toInt();
  mMaxTerrainScreenError = elemTerrain.attribute( QStringLiteral( "max-terrain-error" ), QStringLiteral( "3" ) ).toFloat();
  mMaxTerrainGroundError = elemTerrain.attribute( QStringLiteral( "max-ground-error" ), QStringLiteral( "1" ) ).toFloat();
  mTerrainShadingEnabled = elemTerrain.attribute( QStringLiteral( "shading-enabled" ), QStringLiteral( "0" ) ).toInt();
  mTerrainElevationOffset = elemTerrain.attribute( QStringLiteral( "elevation-offset" ), QStringLiteral( "0.0" ) ).toFloat();

  QDomElement elemTerrainShadingMaterial = elemTerrain.firstChildElement( QStringLiteral( "shading-material" ) );
  if ( !elemTerrainShadingMaterial.isNull() )
    mTerrainShadingMaterial.readXml( elemTerrainShadingMaterial, context );
  mTerrainMapTheme = elemTerrain.attribute( QStringLiteral( "map-theme" ) );
  mShowLabels = elemTerrain.attribute( QStringLiteral( "show-labels" ), QStringLiteral( "0" ) ).toInt();

  qDeleteAll( mLightSources );
  mLightSources.clear();
  const QDomElement lightsElem = elem.firstChildElement( QStringLiteral( "lights" ) );
  if ( !lightsElem.isNull() )
  {
    const QDomNodeList lightNodes = lightsElem.childNodes();
    for ( int i = 0; i < lightNodes.size(); ++i )
    {
      const QDomElement lightElement = lightNodes.at( i ).toElement();
      if ( QgsLightSource *light = QgsLightSource::createFromXml( lightElement, context ) )
        mLightSources << light;
    }
  }
  else
  {
    // older project format
    QDomElement elemPointLights = elem.firstChildElement( QStringLiteral( "point-lights" ) );
    if ( !elemPointLights.isNull() )
    {
      QDomElement elemPointLight = elemPointLights.firstChildElement( QStringLiteral( "point-light" ) );
      while ( !elemPointLight.isNull() )
      {
        std::unique_ptr< QgsPointLightSettings > pointLight = std::make_unique< QgsPointLightSettings >();
        pointLight->readXml( elemPointLight, context );
        mLightSources << pointLight.release();
        elemPointLight = elemPointLight.nextSiblingElement( QStringLiteral( "point-light" ) );
      }
    }
    else
    {
      // QGIS <= 3.4 did not have light configuration
      std::unique_ptr< QgsPointLightSettings > defaultLight = std::make_unique< QgsPointLightSettings >();
      defaultLight->setPosition( QgsVector3D( 0, 1000, 0 ) );
      mLightSources << defaultLight.release();
    }

    QDomElement elemDirectionalLights = elem.firstChildElement( QStringLiteral( "directional-lights" ) );
    if ( !elemDirectionalLights.isNull() )
    {
      QDomElement elemDirectionalLight = elemDirectionalLights.firstChildElement( QStringLiteral( "directional-light" ) );
      while ( !elemDirectionalLight.isNull() )
      {
        std::unique_ptr< QgsDirectionalLightSettings > directionalLight = std::make_unique< QgsDirectionalLightSettings >();
        directionalLight->readXml( elemDirectionalLight, context );
        mLightSources << directionalLight.release();
        elemDirectionalLight = elemDirectionalLight.nextSiblingElement( QStringLiteral( "directional-light" ) );
      }
    }
  }

  QDomElement elemMapLayers = elemTerrain.firstChildElement( QStringLiteral( "layers" ) );
  QDomElement elemMapLayer = elemMapLayers.firstChildElement( QStringLiteral( "layer" ) );
  QList<QgsMapLayerRef> mapLayers;
  while ( !elemMapLayer.isNull() )
  {
    mapLayers << QgsMapLayerRef( elemMapLayer.attribute( QStringLiteral( "id" ) ) );
    elemMapLayer = elemMapLayer.nextSiblingElement( QStringLiteral( "layer" ) );
  }
  mLayers = mapLayers;  // needs to resolve refs afterwards

  QDomElement elemTerrainGenerator = elemTerrain.firstChildElement( QStringLiteral( "generator" ) );
  QString terrainGenType = elemTerrainGenerator.attribute( QStringLiteral( "type" ) );
  if ( terrainGenType == QLatin1String( "dem" ) )
  {
    QgsDemTerrainGenerator *demTerrainGenerator = new QgsDemTerrainGenerator;
    demTerrainGenerator->setCrs( mCrs, mTransformContext );
    setTerrainGenerator( demTerrainGenerator );
  }
  else if ( terrainGenType == QLatin1String( "online" ) )
  {
    QgsOnlineTerrainGenerator *onlineTerrainGenerator = new QgsOnlineTerrainGenerator;
    onlineTerrainGenerator->setCrs( mCrs, mTransformContext );
    setTerrainGenerator( onlineTerrainGenerator );
  }
  else if ( terrainGenType == QLatin1String( "mesh" ) )
  {
    QgsMeshTerrainGenerator *meshTerrainGenerator = new QgsMeshTerrainGenerator;
    meshTerrainGenerator->setCrs( mCrs, mTransformContext );
    setTerrainGenerator( meshTerrainGenerator );
  }
  else // "flat"
  {
    QgsFlatTerrainGenerator *flatGen = new QgsFlatTerrainGenerator;
    flatGen->setCrs( mCrs );
    setTerrainGenerator( flatGen );
  }
  mTerrainGenerator->readXml( elemTerrainGenerator );

  QDomElement elemSkybox = elem.firstChildElement( QStringLiteral( "skybox" ) );
  mIsSkyboxEnabled = elemSkybox.attribute( QStringLiteral( "skybox-enabled" ) ).toInt();
  mSkyboxSettings.readXml( elemSkybox, context );

  QDomElement elemShadows = elem.firstChildElement( QStringLiteral( "shadow-rendering" ) );
  mShadowSettings.readXml( elemShadows, context );

  QDomElement elemAmbientOcclusion = elem.firstChildElement( QStringLiteral( "screen-space-ambient-occlusion" ) );
  mAmbientOcclusionSettings.readXml( elemAmbientOcclusion, context );

  QDomElement elemEyeDomeLighting = elem.firstChildElement( QStringLiteral( "eye-dome-lighting" ) );
  mEyeDomeLightingEnabled = elemEyeDomeLighting.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
  mEyeDomeLightingStrength = elemEyeDomeLighting.attribute( "eye-dome-lighting-strength", QStringLiteral( "1000.0" ) ).toDouble();
  mEyeDomeLightingDistance = elemEyeDomeLighting.attribute( "eye-dome-lighting-distance", QStringLiteral( "1" ) ).toInt();

  QDomElement elemNavigationSync = elem.firstChildElement( QStringLiteral( "navigation-sync" ) );
  mViewSyncMode = ( Qgis::ViewSyncModeFlags )( elemNavigationSync.attribute( QStringLiteral( "view-sync-mode" ), QStringLiteral( "0" ) ).toInt() );
  mVisualizeViewFrustum = elemNavigationSync.attribute( QStringLiteral( "view-frustum-visualization-enabled" ), QStringLiteral( "0" ) ).toInt();

  QDomElement elemDebugSettings = elem.firstChildElement( QStringLiteral( "debug-settings" ) );
  mDebugShadowMapEnabled = elemDebugSettings.attribute( QStringLiteral( "shadowmap-enabled" ), QStringLiteral( "0" ) ).toInt();
  mDebugShadowMapCorner = static_cast<Qt::Corner>( elemDebugSettings.attribute( QStringLiteral( "shadowmap-corner" ), "0" ).toInt() );
  mDebugShadowMapSize = elemDebugSettings.attribute( QStringLiteral( "shadowmap-size" ), QStringLiteral( "0.2" ) ).toDouble();

  mDebugDepthMapEnabled = elemDebugSettings.attribute( QStringLiteral( "depthmap-enabled" ), QStringLiteral( "0" ) ).toInt();
  mDebugDepthMapCorner = static_cast<Qt::Corner>( elemDebugSettings.attribute( QStringLiteral( "depthmap-corner" ), QStringLiteral( "1" ) ).toInt() );
  mDebugDepthMapSize = elemDebugSettings.attribute( QStringLiteral( "depthmap-size" ), QStringLiteral( "0.2" ) ).toDouble();

  QDomElement elemDebug = elem.firstChildElement( QStringLiteral( "debug" ) );
  mShowTerrainBoundingBoxes = elemDebug.attribute( QStringLiteral( "bounding-boxes" ), QStringLiteral( "0" ) ).toInt();
  mShowTerrainTileInfo = elemDebug.attribute( QStringLiteral( "terrain-tile-info" ), QStringLiteral( "0" ) ).toInt();
  mShowCameraViewCenter = elemDebug.attribute( QStringLiteral( "camera-view-center" ), QStringLiteral( "0" ) ).toInt();
  mShowCameraRotationCenter = elemDebug.attribute( QStringLiteral( "camera-rotation-center" ), QStringLiteral( "0" ) ).toInt();
  mShowLightSources = elemDebug.attribute( QStringLiteral( "show-light-sources" ), QStringLiteral( "0" ) ).toInt();
  mIsFpsCounterEnabled = elemDebug.attribute( QStringLiteral( "show-fps-counter" ), QStringLiteral( "0" ) ).toInt();

  QDomElement elemTemporalRange = elem.firstChildElement( QStringLiteral( "temporal-range" ) );
  QDateTime start = QDateTime::fromString( elemTemporalRange.attribute( QStringLiteral( "start" ) ), Qt::ISODate );
  QDateTime end = QDateTime::fromString( elemTemporalRange.attribute( QStringLiteral( "end" ) ), Qt::ISODate );
  setTemporalRange( QgsDateTimeRange( start, end ) );

  QDomElement elem3dAxis = elem.firstChildElement( QStringLiteral( "axis3d" ) );
  m3dAxisSettings.readXml( elem3dAxis, context );

}

QDomElement Qgs3DMapSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "qgis3d" ) );

  QDomElement elemOrigin = doc.createElement( QStringLiteral( "origin" ) );
  elemOrigin.setAttribute( QStringLiteral( "x" ), QString::number( mOrigin.x() ) );
  elemOrigin.setAttribute( QStringLiteral( "y" ), QString::number( mOrigin.y() ) );
  elemOrigin.setAttribute( QStringLiteral( "z" ), QString::number( mOrigin.z() ) );
  elem.appendChild( elemOrigin );

  QDomElement elemExtent = doc.createElement( QStringLiteral( "extent" ) );
  elemExtent.setAttribute( QStringLiteral( "xMin" ), mExtent.xMinimum() );
  elemExtent.setAttribute( QStringLiteral( "yMin" ), mExtent.yMinimum() );
  elemExtent.setAttribute( QStringLiteral( "xMax" ), mExtent.xMaximum() );
  elemExtent.setAttribute( QStringLiteral( "yMax" ), mExtent.yMaximum() );
  elemExtent.setAttribute( QStringLiteral( "showIn2dView" ), mShowExtentIn2DView );
  elem.appendChild( elemExtent );

  QDomElement elemCamera = doc.createElement( QStringLiteral( "camera" ) );
  elemCamera.setAttribute( QStringLiteral( "field-of-view" ), mFieldOfView );
  elemCamera.setAttribute( QStringLiteral( "projection-type" ), static_cast< int >( mProjectionType ) );
  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::TerrainBased:
      elemCamera.setAttribute( QStringLiteral( "camera-navigation-mode" ), QStringLiteral( "terrain-based-navigation" ) );
      break;
    case Qgis::NavigationMode::Walk:
      elemCamera.setAttribute( QStringLiteral( "camera-navigation-mode" ), QStringLiteral( "walk-navigation" ) );
      break;
  }
  elemCamera.setAttribute( QStringLiteral( "camera-movement-speed" ), mCameraMovementSpeed );
  elem.appendChild( elemCamera );

  QDomElement elemColor = doc.createElement( QStringLiteral( "color" ) );
  elemColor.setAttribute( QStringLiteral( "background" ), QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );
  elemColor.setAttribute( QStringLiteral( "selection" ), QgsSymbolLayerUtils::encodeColor( mSelectionColor ) );
  elem.appendChild( elemColor );

  QDomElement elemCrs = doc.createElement( QStringLiteral( "crs" ) );
  mCrs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( QStringLiteral( "terrain" ) );
  elemTerrain.setAttribute( QStringLiteral( "terrain-rendering-enabled" ), mTerrainRenderingEnabled ? 1 : 0 );
  elemTerrain.setAttribute( QStringLiteral( "exaggeration" ), QString::number( mTerrainVerticalScale ) );
  elemTerrain.setAttribute( QStringLiteral( "texture-size" ), mMapTileResolution );
  elemTerrain.setAttribute( QStringLiteral( "max-terrain-error" ), QString::number( mMaxTerrainScreenError ) );
  elemTerrain.setAttribute( QStringLiteral( "max-ground-error" ), QString::number( mMaxTerrainGroundError ) );
  elemTerrain.setAttribute( QStringLiteral( "shading-enabled" ), mTerrainShadingEnabled ? 1 : 0 );
  elemTerrain.setAttribute( QStringLiteral( "elevation-offset" ), mTerrainElevationOffset );

  QDomElement elemTerrainShadingMaterial = doc.createElement( QStringLiteral( "shading-material" ) );
  mTerrainShadingMaterial.writeXml( elemTerrainShadingMaterial, context );
  elemTerrain.appendChild( elemTerrainShadingMaterial );
  elemTerrain.setAttribute( QStringLiteral( "map-theme" ), mTerrainMapTheme );
  elemTerrain.setAttribute( QStringLiteral( "show-labels" ), mShowLabels ? 1 : 0 );

  {
    QDomElement elemLights = doc.createElement( QStringLiteral( "lights" ) );
    for ( const QgsLightSource *light : mLightSources )
    {
      const QDomElement elemLight = light->writeXml( doc, context );
      elemLights.appendChild( elemLight );
    }
    elem.appendChild( elemLights );
  }

  QDomElement elemMapLayers = doc.createElement( QStringLiteral( "layers" ) );
  for ( const QgsMapLayerRef &layerRef : mLayers )
  {
    QDomElement elemMapLayer = doc.createElement( QStringLiteral( "layer" ) );
    elemMapLayer.setAttribute( QStringLiteral( "id" ), layerRef.layerId );
    elemMapLayers.appendChild( elemMapLayer );
  }
  elemTerrain.appendChild( elemMapLayers );

  QDomElement elemTerrainGenerator = doc.createElement( QStringLiteral( "generator" ) );
  elemTerrainGenerator.setAttribute( QStringLiteral( "type" ), QgsTerrainGenerator::typeToString( mTerrainGenerator->type() ) );
  mTerrainGenerator->writeXml( elemTerrainGenerator );
  elemTerrain.appendChild( elemTerrainGenerator );
  elem.appendChild( elemTerrain );

  QDomElement elemSkybox = doc.createElement( QStringLiteral( "skybox" ) );
  elemSkybox.setAttribute( QStringLiteral( "skybox-enabled" ), mIsSkyboxEnabled );
  mSkyboxSettings.writeXml( elemSkybox, context );
  elem.appendChild( elemSkybox );

  QDomElement elemShadows = doc.createElement( QStringLiteral( "shadow-rendering" ) );
  mShadowSettings.writeXml( elemShadows, context );
  elem.appendChild( elemShadows );

  QDomElement elemAmbientOcclusion = doc.createElement( QStringLiteral( "screen-space-ambient-occlusion" ) );
  mAmbientOcclusionSettings.writeXml( elemAmbientOcclusion, context );
  elem.appendChild( elemAmbientOcclusion );

  QDomElement elemDebug = doc.createElement( QStringLiteral( "debug" ) );
  elemDebug.setAttribute( QStringLiteral( "bounding-boxes" ), mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "terrain-tile-info" ), mShowTerrainTileInfo ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "camera-view-center" ), mShowCameraViewCenter ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "camera-rotation-center" ), mShowCameraRotationCenter ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "show-light-sources" ), mShowLightSources ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "show-fps-counter" ), mIsFpsCounterEnabled ? 1 : 0 );
  elem.appendChild( elemDebug );

  QDomElement elemEyeDomeLighting = doc.createElement( QStringLiteral( "eye-dome-lighting" ) );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "enabled" ), mEyeDomeLightingEnabled ? 1 : 0 );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "eye-dome-lighting-strength" ), mEyeDomeLightingStrength );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "eye-dome-lighting-distance" ), mEyeDomeLightingDistance );
  elem.appendChild( elemEyeDomeLighting );

  QDomElement elemNavigationSync = doc.createElement( QStringLiteral( "navigation-sync" ) );
  elemNavigationSync.setAttribute( QStringLiteral( "view-sync-mode" ), ( int )mViewSyncMode );
  elemNavigationSync.setAttribute( QStringLiteral( "view-frustum-visualization-enabled" ), mVisualizeViewFrustum ? 1 : 0 );
  elem.appendChild( elemNavigationSync );

  QDomElement elemDebugSettings = doc.createElement( QStringLiteral( "debug-settings" ) );
  elemDebugSettings.setAttribute( QStringLiteral( "shadowmap-enabled" ), mDebugShadowMapEnabled );
  elemDebugSettings.setAttribute( QStringLiteral( "shadowmap-corner" ), mDebugShadowMapCorner );
  elemDebugSettings.setAttribute( QStringLiteral( "shadowmap-size" ), mDebugShadowMapSize );
  elemDebugSettings.setAttribute( QStringLiteral( "depthmap-enabled" ), mDebugDepthMapEnabled );
  elemDebugSettings.setAttribute( QStringLiteral( "depthmap-corner" ), mDebugDepthMapCorner );
  elemDebugSettings.setAttribute( QStringLiteral( "depthmap-size" ), mDebugDepthMapSize );
  elem.appendChild( elemDebugSettings );

  QDomElement elemTemporalRange = doc.createElement( QStringLiteral( "temporal-range" ) );
  elemTemporalRange.setAttribute( QStringLiteral( "start" ), temporalRange().begin().toString( Qt::ISODate ) );
  elemTemporalRange.setAttribute( QStringLiteral( "end" ), temporalRange().end().toString( Qt::ISODate ) );

  QDomElement elem3dAxis = doc.createElement( QStringLiteral( "axis3d" ) );
  m3dAxisSettings.writeXml( elem3dAxis, context );
  elem.appendChild( elem3dAxis );

  return elem;
}

void Qgs3DMapSettings::resolveReferences( const QgsProject &project )
{
  for ( int i = 0; i < mLayers.count(); ++i )
  {
    QgsMapLayerRef &layerRef = mLayers[i];
    layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
  }

  mTerrainGenerator->resolveReferences( project );
}

void Qgs3DMapSettings::setExtent( const QgsRectangle &extent )
{
  if ( extent == mExtent )
    return;

  mExtent = extent;
  const QgsPointXY center = mExtent.center();
  setOrigin( QgsVector3D( center.x(), center.y(), 0 ) );
  if ( mTerrainGenerator )
  {
    QgsRectangle terrainExtent = Qgs3DUtils::tryReprojectExtent2D( mExtent, mCrs, mTerrainGenerator->crs(), mTransformContext );
    mTerrainGenerator->setExtent( terrainExtent );
  }
  emit extentChanged();
}

QgsVector3D Qgs3DMapSettings::mapToWorldCoordinates( const QgsVector3D &mapCoords ) const
{
  return Qgs3DUtils::mapToWorldCoordinates( mapCoords, mOrigin );
}

QgsVector3D Qgs3DMapSettings::worldToMapCoordinates( const QgsVector3D &worldCoords ) const
{
  return Qgs3DUtils::worldToMapCoordinates( worldCoords, mOrigin );
}

void Qgs3DMapSettings::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

QgsCoordinateTransformContext Qgs3DMapSettings::transformContext() const
{
  return mTransformContext;
}

void Qgs3DMapSettings::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
}

void Qgs3DMapSettings::setBackgroundColor( const QColor &color )
{
  if ( color == mBackgroundColor )
    return;

  mBackgroundColor = color;
  emit backgroundColorChanged();
}

QColor Qgs3DMapSettings::backgroundColor() const
{
  return mBackgroundColor;
}

void Qgs3DMapSettings::setSelectionColor( const QColor &color )
{
  if ( color == mSelectionColor )
    return;

  mSelectionColor = color;
  emit selectionColorChanged();
}

QColor Qgs3DMapSettings::selectionColor() const
{
  return mSelectionColor;
}

void Qgs3DMapSettings::setTerrainVerticalScale( double zScale )
{
  if ( zScale == mTerrainVerticalScale )
    return;

  mTerrainVerticalScale = zScale;
  emit terrainVerticalScaleChanged();
}

double Qgs3DMapSettings::terrainVerticalScale() const
{
  return mTerrainVerticalScale;
}

void Qgs3DMapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  const QList<QgsMapLayer *> raw = _qgis_listRefToRaw( mLayers );

  if ( layers == raw )
    return;

  mLayers = _qgis_listRawToRef( layers );
  emit layersChanged();
}

QList<QgsMapLayer *> Qgs3DMapSettings::layers() const
{
  QList<QgsMapLayer *> lst;
  lst.reserve( mLayers.count() );
  for ( const QgsMapLayerRef &layerRef : mLayers )
  {
    if ( layerRef.layer )
      lst.append( layerRef.layer );
  }
  return lst;
}

void Qgs3DMapSettings::configureTerrainFromProject( QgsProjectElevationProperties *properties, const QgsRectangle &fullExtent )
{
  setExtent( fullExtent );
  if ( properties->terrainProvider()->type() == QLatin1String( "flat" ) )
  {
    QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
    flatTerrain->setCrs( crs() );
    setTerrainGenerator( flatTerrain );

    setTerrainElevationOffset( properties->terrainProvider()->offset() );
  }
  else if ( properties->terrainProvider()->type() == QLatin1String( "raster" ) )
  {
    QgsRasterDemTerrainProvider *rasterProvider = qgis::down_cast< QgsRasterDemTerrainProvider * >( properties->terrainProvider() );

    QgsDemTerrainGenerator *demTerrainGen = new QgsDemTerrainGenerator;
    demTerrainGen->setCrs( crs(), QgsProject::instance()->transformContext() );
    demTerrainGen->setLayer( rasterProvider->layer() );
    setTerrainGenerator( demTerrainGen );

    setTerrainElevationOffset( properties->terrainProvider()->offset() );
    setTerrainVerticalScale( properties->terrainProvider()->scale() );
  }
  else if ( properties->terrainProvider()->type() == QLatin1String( "mesh" ) )
  {
    QgsMeshTerrainProvider *meshProvider = qgis::down_cast< QgsMeshTerrainProvider * >( properties->terrainProvider() );

    QgsMeshTerrainGenerator *newTerrainGenerator = new QgsMeshTerrainGenerator;
    newTerrainGenerator->setCrs( crs(), QgsProject::instance()->transformContext() );
    newTerrainGenerator->setLayer( meshProvider->layer() );
    std::unique_ptr< QgsMesh3DSymbol > symbol( newTerrainGenerator->symbol()->clone() );
    symbol->setVerticalScale( properties->terrainProvider()->scale() );
    newTerrainGenerator->setSymbol( symbol.release() );
    setTerrainGenerator( newTerrainGenerator );

    setTerrainElevationOffset( properties->terrainProvider()->offset() );
    setTerrainVerticalScale( properties->terrainProvider()->scale() );
  }
  else
  {
    QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
    flatTerrain->setCrs( crs() );
    setTerrainGenerator( flatTerrain );
  }
}

void Qgs3DMapSettings::setMapTileResolution( int res )
{
  if ( mMapTileResolution == res )
    return;

  mMapTileResolution = res;
  emit mapTileResolutionChanged();
}

int Qgs3DMapSettings::mapTileResolution() const
{
  return mMapTileResolution;
}

void Qgs3DMapSettings::setMaxTerrainScreenError( float error )
{
  if ( mMaxTerrainScreenError == error )
    return;

  mMaxTerrainScreenError = error;
  emit maxTerrainScreenErrorChanged();
}

float Qgs3DMapSettings::maxTerrainScreenError() const
{
  return mMaxTerrainScreenError;
}

void Qgs3DMapSettings::setMaxTerrainGroundError( float error )
{
  if ( mMaxTerrainGroundError == error )
    return;

  mMaxTerrainGroundError = error;
  emit maxTerrainGroundErrorChanged();
}

void Qgs3DMapSettings::setTerrainElevationOffset( float offset )
{
  if ( mTerrainElevationOffset == offset )
    return;
  mTerrainElevationOffset = offset;
  emit terrainElevationOffsetChanged( mTerrainElevationOffset );
}

float Qgs3DMapSettings::maxTerrainGroundError() const
{
  return mMaxTerrainGroundError;
}

void Qgs3DMapSettings::setTerrainGenerator( QgsTerrainGenerator *gen )
{
  if ( mTerrainGenerator )
  {
    disconnect( mTerrainGenerator.get(), &QgsTerrainGenerator::terrainChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );
  }

  QgsRectangle terrainExtent = Qgs3DUtils::tryReprojectExtent2D( mExtent, mCrs, gen->crs(), mTransformContext );
  gen->setExtent( terrainExtent );
  mTerrainGenerator.reset( gen );
  connect( mTerrainGenerator.get(), &QgsTerrainGenerator::terrainChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );

  emit terrainGeneratorChanged();
}

void Qgs3DMapSettings::setTerrainShadingEnabled( bool enabled )
{
  if ( mTerrainShadingEnabled == enabled )
    return;

  mTerrainShadingEnabled = enabled;
  emit terrainShadingChanged();
}

void Qgs3DMapSettings::setTerrainShadingMaterial( const QgsPhongMaterialSettings &material )
{
  if ( mTerrainShadingMaterial == material )
    return;

  mTerrainShadingMaterial = material;
  emit terrainShadingChanged();
}

void Qgs3DMapSettings::setTerrainMapTheme( const QString &theme )
{
  if ( mTerrainMapTheme == theme )
    return;

  mTerrainMapTheme = theme;
  emit terrainMapThemeChanged();
}

void Qgs3DMapSettings::setShowTerrainBoundingBoxes( bool enabled )
{
  if ( mShowTerrainBoundingBoxes == enabled )
    return;

  mShowTerrainBoundingBoxes = enabled;
  emit showTerrainBoundingBoxesChanged();
}

void Qgs3DMapSettings::setShowTerrainTilesInfo( bool enabled )
{
  if ( mShowTerrainTileInfo == enabled )
    return;

  mShowTerrainTileInfo = enabled;
  emit showTerrainTilesInfoChanged();
}

void Qgs3DMapSettings::setShowCameraViewCenter( bool enabled )
{
  if ( mShowCameraViewCenter == enabled )
    return;

  mShowCameraViewCenter = enabled;
  emit showCameraViewCenterChanged();
}

void Qgs3DMapSettings::setShowCameraRotationCenter( bool enabled )
{
  if ( mShowCameraRotationCenter == enabled )
    return;

  mShowCameraRotationCenter = enabled;
  emit showCameraRotationCenterChanged();
}


void Qgs3DMapSettings::setShowLightSourceOrigins( bool enabled )
{
  if ( mShowLightSources == enabled )
    return;

  mShowLightSources = enabled;
  emit showLightSourceOriginsChanged();
}

void Qgs3DMapSettings::setShowLabels( bool enabled )
{
  if ( mShowLabels == enabled )
    return;

  mShowLabels = enabled;
  emit showLabelsChanged();
}

void Qgs3DMapSettings::setEyeDomeLightingEnabled( bool enabled )
{
  if ( mEyeDomeLightingEnabled == enabled )
    return;
  mEyeDomeLightingEnabled = enabled;
  emit eyeDomeLightingEnabledChanged();
}

void Qgs3DMapSettings::setEyeDomeLightingStrength( double strength )
{
  if ( mEyeDomeLightingStrength == strength )
    return;
  mEyeDomeLightingStrength = strength;
  emit eyeDomeLightingStrengthChanged();
}

void Qgs3DMapSettings::setEyeDomeLightingDistance( int distance )
{
  if ( mEyeDomeLightingDistance == distance )
    return;
  mEyeDomeLightingDistance = distance;
  emit eyeDomeLightingDistanceChanged();
}

QList<QgsLightSource *> Qgs3DMapSettings::lightSources() const
{
  return mLightSources;
}

void Qgs3DMapSettings::setLightSources( const QList<QgsLightSource *> &lights )
{
  qDeleteAll( mLightSources );
  mLightSources = lights;

  emit directionalLightsChanged();
  emit pointLightsChanged();
  emit lightSourcesChanged();
}

void Qgs3DMapSettings::setFieldOfView( const float fieldOfView )
{
  if ( mFieldOfView == fieldOfView )
    return;

  mFieldOfView = fieldOfView;
  emit fieldOfViewChanged();
}

void Qgs3DMapSettings::setProjectionType( const Qt3DRender::QCameraLens::ProjectionType projectionType )
{
  if ( mProjectionType == projectionType )
    return;

  mProjectionType = projectionType;
  emit projectionTypeChanged();
}

void Qgs3DMapSettings::setCameraNavigationMode( Qgis::NavigationMode navigationMode )
{
  if ( mCameraNavigationMode == navigationMode )
    return;

  mCameraNavigationMode = navigationMode;
  emit cameraNavigationModeChanged();
}

void Qgs3DMapSettings::setCameraMovementSpeed( double movementSpeed )
{
  if ( mCameraMovementSpeed == movementSpeed )
    return;

  mCameraMovementSpeed = movementSpeed;
  emit cameraMovementSpeedChanged();
}

void Qgs3DMapSettings::setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
{
  mSkyboxSettings = skyboxSettings;
  emit skyboxSettingsChanged();
}

void Qgs3DMapSettings::setShadowSettings( const QgsShadowSettings &shadowSettings )
{
  mShadowSettings = shadowSettings;
  emit shadowSettingsChanged();
}

void Qgs3DMapSettings::setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &ambientOcclusionSettings )
{
  mAmbientOcclusionSettings = ambientOcclusionSettings;
  emit ambientOcclusionSettingsChanged();
}

void Qgs3DMapSettings::setDebugShadowMapSettings( bool enabled, Qt::Corner corner, double size )
{
  mDebugShadowMapEnabled = enabled;
  mDebugShadowMapCorner = corner;
  mDebugShadowMapSize = size;
  emit debugShadowMapSettingsChanged();
}

void Qgs3DMapSettings::setDebugDepthMapSettings( bool enabled, Qt::Corner corner, double size )
{
  mDebugDepthMapEnabled = enabled;
  mDebugDepthMapCorner = corner;
  mDebugDepthMapSize = size;
  emit debugDepthMapSettingsChanged();
}

void Qgs3DMapSettings::setIsFpsCounterEnabled( bool fpsCounterEnabled )
{
  if ( fpsCounterEnabled == mIsFpsCounterEnabled )
    return;
  mIsFpsCounterEnabled = fpsCounterEnabled;
  emit fpsCounterEnabledChanged( mIsFpsCounterEnabled );
}

void Qgs3DMapSettings::setTerrainRenderingEnabled( bool terrainRenderingEnabled )
{
  if ( terrainRenderingEnabled == mTerrainRenderingEnabled )
    return;
  mTerrainRenderingEnabled = terrainRenderingEnabled;
  emit terrainGeneratorChanged();
}

Qgis::RendererUsage Qgs3DMapSettings::rendererUsage() const
{
  return mRendererUsage;
}

void Qgs3DMapSettings::setRendererUsage( Qgis::RendererUsage rendererUsage )
{
  mRendererUsage = rendererUsage;
}

void Qgs3DMapSettings::setViewSyncMode( Qgis::ViewSyncModeFlags mode )
{
  mViewSyncMode = mode;
}

void Qgs3DMapSettings::setViewFrustumVisualizationEnabled( bool enabled )
{
  if ( mVisualizeViewFrustum != enabled )
  {
    mVisualizeViewFrustum = enabled;
    emit viewFrustumVisualizationEnabledChanged();
  }
}

void Qgs3DMapSettings::setIsDebugOverlayEnabled( bool debugOverlayEnabled )
{
  if ( debugOverlayEnabled == mIsDebugOverlayEnabled )
    return;

  mIsDebugOverlayEnabled = debugOverlayEnabled;
  emit debugOverlayEnabledChanged( mIsDebugOverlayEnabled );
}


void Qgs3DMapSettings::connectChangedSignalsToSettingsChanged()
{
  connect( this, &Qgs3DMapSettings::selectionColorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::layersChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainGeneratorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainVerticalScaleChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::mapTileResolutionChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::maxTerrainScreenErrorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::maxTerrainGroundErrorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainElevationOffsetChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainShadingChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainMapThemeChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::renderersChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showCameraViewCenterChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showCameraRotationCenterChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showLightSourceOriginsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::eyeDomeLightingEnabledChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::eyeDomeLightingStrengthChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::eyeDomeLightingDistanceChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::debugShadowMapSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::debugDepthMapSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::lightSourcesChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::fieldOfViewChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::projectionTypeChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::cameraNavigationModeChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::cameraMovementSpeedChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::skyboxSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::shadowSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::fpsCounterEnabledChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::axisSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::ambientOcclusionSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::extentChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showExtentIn2DViewChanged, this, &Qgs3DMapSettings::settingsChanged );
}


void Qgs3DMapSettings::set3DAxisSettings( const Qgs3DAxisSettings &axisSettings, bool force )
{
  if ( axisSettings == m3dAxisSettings )
  {
    if ( force )
    {
      // ie. refresh. We need to disconnect and to reconnect to avoid 'dirty' project
      disconnect( this, &Qgs3DMapSettings::axisSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
      emit axisSettingsChanged();
      connect( this, &Qgs3DMapSettings::axisSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
    }
  }
  else
  {
    m3dAxisSettings = axisSettings;
    emit axisSettingsChanged();
  }
}

void Qgs3DMapSettings::setShowExtentIn2DView( bool show )
{
  if ( show == mShowExtentIn2DView )
    return;

  mShowExtentIn2DView = show;
  emit showExtentIn2DViewChanged();
}
