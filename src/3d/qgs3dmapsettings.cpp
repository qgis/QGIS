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
#include "moc_qgs3dmapsettings.cpp"

#include "qgs3d.h"
#include "qgs3dutils.h"
#include "qgsprojectviewsettings.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgslightsource.h"
#include "qgscolorutils.h"
#include "qgsrasterlayer.h"
#include "qgspointlightsettings.h"
#include "qgsdirectionallightsettings.h"
#include "qgs3drendercontext.h"
#include "qgsthreadingutils.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsabstractterrainsettings.h"
#include "qgsflatterrainsettings.h"
#include "qgs3dterrainregistry.h"

#include <QDomDocument>
#include <QDomElement>


Qgs3DMapSettings::Qgs3DMapSettings()
  : QObject( nullptr )
{
  connect( this, &Qgs3DMapSettings::settingsChanged, [&]() {
    QgsProject::instance()->setDirty();
  } );
  connectChangedSignalsToSettingsChanged();
  mTerrainSettings = std::make_unique<QgsFlatTerrainSettings>();
}

Qgs3DMapSettings::Qgs3DMapSettings( const Qgs3DMapSettings &other )
  : QObject( nullptr )
  , QgsTemporalRangeObject( other )
  , mOrigin( other.mOrigin )
  , mCrs( other.mCrs )
  , mBackgroundColor( other.mBackgroundColor )
  , mSelectionColor( other.mSelectionColor )
  , mTerrainShadingEnabled( other.mTerrainShadingEnabled )
  , mTerrainShadingMaterial( other.mTerrainShadingMaterial )
  , mTerrainMapTheme( other.mTerrainMapTheme )
  , mShowTerrainBoundingBoxes( other.mShowTerrainBoundingBoxes )
  , mShowTerrainTileInfo( other.mShowTerrainTileInfo )
  , mShowCameraViewCenter( other.mShowCameraViewCenter )
  , mShowCameraRotationCenter( other.mShowCameraRotationCenter )
  , mShowLightSources( other.mShowLightSources )
  , mShowLabels( other.mShowLabels )
  , mStopUpdates( other.mStopUpdates )
  , mShowDebugPanel( other.mShowDebugPanel )
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
  setTerrainSettings( other.mTerrainSettings ? other.mTerrainSettings->clone() : new QgsFlatTerrainSettings() );

  for ( QgsLightSource *source : std::as_const( other.mLightSources ) )
  {
    if ( source )
      mLightSources << source->clone();
  }

  connect( this, &Qgs3DMapSettings::settingsChanged, [&]() {
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsProjectDirtyBlocker blocker( QgsProject::instance() );
  QDomElement elemOrigin = elem.firstChildElement( QStringLiteral( "origin" ) );
  mOrigin = QgsVector3D(
    elemOrigin.attribute( QStringLiteral( "x" ) ).toDouble(),
    elemOrigin.attribute( QStringLiteral( "y" ) ).toDouble(),
    elemOrigin.attribute( QStringLiteral( "z" ) ).toDouble()
  );

  QDomElement elemExtent = elem.firstChildElement( QStringLiteral( "extent" ) );
  if ( !elemExtent.isNull() )
  {
    mExtent = QgsRectangle(
      elemExtent.attribute( QStringLiteral( "xMin" ) ).toDouble(),
      elemExtent.attribute( QStringLiteral( "yMin" ) ).toDouble(),
      elemExtent.attribute( QStringLiteral( "xMax" ) ).toDouble(),
      elemExtent.attribute( QStringLiteral( "yMax" ) ).toDouble()
    );

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
    mProjectionType = static_cast<Qt3DRender::QCameraLens::ProjectionType>( elemCamera.attribute( QStringLiteral( "projection-type" ), QStringLiteral( "1" ) ).toInt() );
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
    mBackgroundColor = QgsColorUtils::colorFromString( elemColor.attribute( QStringLiteral( "background" ) ) );
    mSelectionColor = QgsColorUtils::colorFromString( elemColor.attribute( QStringLiteral( "selection" ) ) );
  }

  QDomElement elemCrs = elem.firstChildElement( QStringLiteral( "crs" ) );
  mCrs.readXml( elemCrs );

  QDomElement elemTerrain = elem.firstChildElement( QStringLiteral( "terrain" ) );
  mTerrainRenderingEnabled = elemTerrain.attribute( QStringLiteral( "terrain-rendering-enabled" ), QStringLiteral( "1" ) ).toInt();
  mTerrainShadingEnabled = elemTerrain.attribute( QStringLiteral( "shading-enabled" ), QStringLiteral( "0" ) ).toInt();

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
        std::unique_ptr<QgsPointLightSettings> pointLight = std::make_unique<QgsPointLightSettings>();
        pointLight->readXml( elemPointLight, context );
        mLightSources << pointLight.release();
        elemPointLight = elemPointLight.nextSiblingElement( QStringLiteral( "point-light" ) );
      }
    }
    else
    {
      // QGIS <= 3.4 did not have light configuration
      std::unique_ptr<QgsPointLightSettings> defaultLight = std::make_unique<QgsPointLightSettings>();
      defaultLight->setPosition( QgsVector3D( 0, 1000, 0 ) );
      mLightSources << defaultLight.release();
    }

    QDomElement elemDirectionalLights = elem.firstChildElement( QStringLiteral( "directional-lights" ) );
    if ( !elemDirectionalLights.isNull() )
    {
      QDomElement elemDirectionalLight = elemDirectionalLights.firstChildElement( QStringLiteral( "directional-light" ) );
      while ( !elemDirectionalLight.isNull() )
      {
        std::unique_ptr<QgsDirectionalLightSettings> directionalLight = std::make_unique<QgsDirectionalLightSettings>();
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
  mLayers = mapLayers; // needs to resolve refs afterwards

  QDomElement elemTerrainGenerator = elemTerrain.firstChildElement( QStringLiteral( "generator" ) );
  const QString terrainGenType = elemTerrainGenerator.attribute( QStringLiteral( "type" ) );
  std::unique_ptr<QgsAbstractTerrainSettings> terrainSettings( Qgs3D::terrainRegistry()->createTerrainSettings( terrainGenType ) );
  if ( terrainSettings )
  {
    terrainSettings->readXml( elemTerrain, context );
    setTerrainSettings( terrainSettings.release() );
  }

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
  mStopUpdates = elemDebug.attribute( QStringLiteral( "stop-updates" ), QStringLiteral( "0" ) ).toInt();
  mShowDebugPanel = elemDebug.attribute( QStringLiteral( "debug-panel" ), QStringLiteral( "0" ) ).toInt();

  QDomElement elemTemporalRange = elem.firstChildElement( QStringLiteral( "temporal-range" ) );
  QDateTime start = QDateTime::fromString( elemTemporalRange.attribute( QStringLiteral( "start" ) ), Qt::ISODate );
  QDateTime end = QDateTime::fromString( elemTemporalRange.attribute( QStringLiteral( "end" ) ), Qt::ISODate );
  setTemporalRange( QgsDateTimeRange( start, end ) );

  QDomElement elem3dAxis = elem.firstChildElement( QStringLiteral( "axis3d" ) );
  m3dAxisSettings.readXml( elem3dAxis, context );
}

QDomElement Qgs3DMapSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  elemCamera.setAttribute( QStringLiteral( "projection-type" ), static_cast<int>( mProjectionType ) );
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
  elemColor.setAttribute( QStringLiteral( "background" ), QgsColorUtils::colorToString( mBackgroundColor ) );
  elemColor.setAttribute( QStringLiteral( "selection" ), QgsColorUtils::colorToString( mSelectionColor ) );
  elem.appendChild( elemColor );

  QDomElement elemCrs = doc.createElement( QStringLiteral( "crs" ) );
  mCrs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( QStringLiteral( "terrain" ) );
  elemTerrain.setAttribute( QStringLiteral( "terrain-rendering-enabled" ), mTerrainRenderingEnabled ? 1 : 0 );
  elemTerrain.setAttribute( QStringLiteral( "shading-enabled" ), mTerrainShadingEnabled ? 1 : 0 );

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
  elemTerrainGenerator.setAttribute( QStringLiteral( "type" ), mTerrainSettings->type() );
  mTerrainSettings->writeXml( elemTerrain, context );
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
  elemDebug.setAttribute( QStringLiteral( "stop-updates" ), mStopUpdates ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "debug-panel" ), mShowDebugPanel ? 1 : 0 );
  elem.appendChild( elemDebug );

  QDomElement elemEyeDomeLighting = doc.createElement( QStringLiteral( "eye-dome-lighting" ) );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "enabled" ), mEyeDomeLightingEnabled ? 1 : 0 );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "eye-dome-lighting-strength" ), mEyeDomeLightingStrength );
  elemEyeDomeLighting.setAttribute( QStringLiteral( "eye-dome-lighting-distance" ), mEyeDomeLightingDistance );
  elem.appendChild( elemEyeDomeLighting );

  QDomElement elemNavigationSync = doc.createElement( QStringLiteral( "navigation-sync" ) );
  elemNavigationSync.setAttribute( QStringLiteral( "view-sync-mode" ), ( int ) mViewSyncMode );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  for ( int i = 0; i < mLayers.count(); ++i )
  {
    QgsMapLayerRef &layerRef = mLayers[i];
    layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
  }

  if ( mTerrainSettings )
  {
    mTerrainSettings->resolveReferences( &project );

    std::unique_ptr<QgsTerrainGenerator> terrainGenerator = mTerrainSettings->createTerrainGenerator( Qgs3DRenderContext::fromMapSettings( this ) );
    if ( terrainGenerator )
    {
      setTerrainGenerator( terrainGenerator.release() );
    }
    emit terrainSettingsChanged();
  }
}

QgsRectangle Qgs3DMapSettings::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

void Qgs3DMapSettings::setExtent( const QgsRectangle &extent )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

void Qgs3DMapSettings::setOrigin( const QgsVector3D &origin )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( origin == mOrigin )
    return;

  mOrigin = origin;
  emit originChanged();
}

QgsVector3D Qgs3DMapSettings::origin() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mOrigin;
}

QgsVector3D Qgs3DMapSettings::mapToWorldCoordinates( const QgsVector3D &mapCoords ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgs3DUtils::mapToWorldCoordinates( mapCoords, mOrigin );
}

QgsVector3D Qgs3DMapSettings::worldToMapCoordinates( const QgsVector3D &worldCoords ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgs3DUtils::worldToMapCoordinates( worldCoords, mOrigin );
}

void Qgs3DMapSettings::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mCrs = crs;
}

QgsCoordinateReferenceSystem Qgs3DMapSettings::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QgsCoordinateTransformContext Qgs3DMapSettings::transformContext() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTransformContext;
}

void Qgs3DMapSettings::setTransformContext( const QgsCoordinateTransformContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mTransformContext = context;
}

const QgsPathResolver &Qgs3DMapSettings::pathResolver() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPathResolver;
}

void Qgs3DMapSettings::setPathResolver( const QgsPathResolver &resolver )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mPathResolver = resolver;
}

QgsMapThemeCollection *Qgs3DMapSettings::mapThemeCollection() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMapThemes;
}

void Qgs3DMapSettings::setMapThemeCollection( QgsMapThemeCollection *mapThemes )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mMapThemes = mapThemes;
}

void Qgs3DMapSettings::setBackgroundColor( const QColor &color )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( color == mBackgroundColor )
    return;

  mBackgroundColor = color;
  emit backgroundColorChanged();
}

QColor Qgs3DMapSettings::backgroundColor() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mBackgroundColor;
}

void Qgs3DMapSettings::setSelectionColor( const QColor &color )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( color == mSelectionColor )
    return;

  mSelectionColor = color;
  emit selectionColorChanged();
}

QColor Qgs3DMapSettings::selectionColor() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSelectionColor;
}

void Qgs3DMapSettings::setTerrainVerticalScale( double zScale )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( zScale == mTerrainSettings->verticalScale() )
    return;

  mTerrainSettings->setVerticalScale( zScale );
  Q_NOWARN_DEPRECATED_PUSH
  emit terrainVerticalScaleChanged();
  Q_NOWARN_DEPRECATED_POP
  emit terrainSettingsChanged();
}

double Qgs3DMapSettings::terrainVerticalScale() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings->verticalScale();
}

void Qgs3DMapSettings::setLayers( const QList<QgsMapLayer *> &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QList<QgsMapLayer *> raw = _qgis_listRefToRaw( mLayers );

  if ( layers == raw )
    return;

  mLayers = _qgis_listRawToRef( layers );
  emit layersChanged();
}

QList<QgsMapLayer *> Qgs3DMapSettings::layers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setExtent( fullExtent );

  std::unique_ptr<QgsAbstractTerrainSettings> terrainSettings( Qgs3D::terrainRegistry()->configureTerrainFromProject( properties ) );
  if ( terrainSettings )
  {
    setTerrainSettings( terrainSettings.release() );
  }
}

const QgsAbstractTerrainSettings *Qgs3DMapSettings::terrainSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings.get();
}

void Qgs3DMapSettings::setTerrainSettings( QgsAbstractTerrainSettings *settings )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool hasChanged = false;
  if ( settings == mTerrainSettings.get() )
  {
    // emit signals anyway. We don't know if the caller changed settings on the pointer before calling this..
    hasChanged = true;
  }
  else
  {
    hasChanged = !settings->equals( mTerrainSettings.get() );
    mTerrainSettings.reset( settings );
  }

  if ( hasChanged )
  {
    std::unique_ptr<QgsTerrainGenerator> terrainGenerator = mTerrainSettings->createTerrainGenerator( Qgs3DRenderContext::fromMapSettings( this ) );
    if ( terrainGenerator )
    {
      setTerrainGenerator( terrainGenerator.release() );
    }

    // emit all the signals, we don't know exactly what's changed
    Q_NOWARN_DEPRECATED_PUSH
    emit mapTileResolutionChanged();
    emit maxTerrainScreenErrorChanged();
    emit maxTerrainGroundErrorChanged();
    emit terrainElevationOffsetChanged( mTerrainSettings->elevationOffset() );
    emit terrainVerticalScaleChanged();
    Q_NOWARN_DEPRECATED_POP

    emit terrainSettingsChanged();
  }
}

void Qgs3DMapSettings::setMapTileResolution( int res )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainSettings->mapTileResolution() == res )
    return;

  mTerrainSettings->setMapTileResolution( res );
  Q_NOWARN_DEPRECATED_PUSH
  emit mapTileResolutionChanged();
  Q_NOWARN_DEPRECATED_POP
  emit terrainSettingsChanged();
}

int Qgs3DMapSettings::mapTileResolution() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings->mapTileResolution();
}

void Qgs3DMapSettings::setMaxTerrainScreenError( double error )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainSettings->maximumScreenError() == error )
    return;

  mTerrainSettings->setMaximumScreenError( error );
  Q_NOWARN_DEPRECATED_PUSH
  emit maxTerrainScreenErrorChanged();
  Q_NOWARN_DEPRECATED_POP
  emit terrainSettingsChanged();
}

double Qgs3DMapSettings::maxTerrainScreenError() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings->maximumScreenError();
}

void Qgs3DMapSettings::setMaxTerrainGroundError( double error )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainSettings->maximumGroundError() == error )
    return;

  mTerrainSettings->setMaximumGroundError( error );
  Q_NOWARN_DEPRECATED_PUSH
  emit maxTerrainGroundErrorChanged();
  Q_NOWARN_DEPRECATED_POP

  emit terrainSettingsChanged();
}

void Qgs3DMapSettings::setTerrainElevationOffset( double offset )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainSettings->elevationOffset() == offset )
    return;
  mTerrainSettings->setElevationOffset( offset );
  Q_NOWARN_DEPRECATED_PUSH
  emit terrainElevationOffsetChanged( offset );
  Q_NOWARN_DEPRECATED_POP
  emit terrainSettingsChanged();
}

double Qgs3DMapSettings::terrainElevationOffset() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings->elevationOffset();
}

double Qgs3DMapSettings::maxTerrainGroundError() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainSettings->maximumGroundError();
}

void Qgs3DMapSettings::setTerrainGenerator( QgsTerrainGenerator *gen )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainGenerator )
  {
    disconnect( mTerrainGenerator.get(), &QgsTerrainGenerator::terrainChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );
  }

  if ( gen->crs().isValid() ) // Don't bother setting an extent rect in the wrong CRS
  {
    QgsRectangle terrainExtent = Qgs3DUtils::tryReprojectExtent2D( mExtent, mCrs, gen->crs(), mTransformContext );
    gen->setExtent( terrainExtent );
  }
  mTerrainGenerator.reset( gen );
  connect( mTerrainGenerator.get(), &QgsTerrainGenerator::terrainChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );

  emit terrainGeneratorChanged();
}

QgsTerrainGenerator *Qgs3DMapSettings::terrainGenerator() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainGenerator.get();
}

void Qgs3DMapSettings::setTerrainShadingEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainShadingEnabled == enabled )
    return;

  mTerrainShadingEnabled = enabled;
  emit terrainShadingChanged();
}

bool Qgs3DMapSettings::isTerrainShadingEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainShadingEnabled;
}

void Qgs3DMapSettings::setTerrainShadingMaterial( const QgsPhongMaterialSettings &material )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainShadingMaterial == material )
    return;

  mTerrainShadingMaterial = material;
  emit terrainShadingChanged();
}

QgsPhongMaterialSettings Qgs3DMapSettings::terrainShadingMaterial() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainShadingMaterial;
}

void Qgs3DMapSettings::setTerrainMapTheme( const QString &theme )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mTerrainMapTheme == theme )
    return;

  mTerrainMapTheme = theme;
  emit terrainMapThemeChanged();
}

QString Qgs3DMapSettings::terrainMapTheme() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainMapTheme;
}

void Qgs3DMapSettings::setShowTerrainBoundingBoxes( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowTerrainBoundingBoxes == enabled )
    return;

  mShowTerrainBoundingBoxes = enabled;
  emit showTerrainBoundingBoxesChanged();
}

bool Qgs3DMapSettings::showTerrainBoundingBoxes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowTerrainBoundingBoxes;
}


void Qgs3DMapSettings::setShowTerrainTilesInfo( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowTerrainTileInfo == enabled )
    return;

  mShowTerrainTileInfo = enabled;
  emit showTerrainTilesInfoChanged();
}

bool Qgs3DMapSettings::showTerrainTilesInfo() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowTerrainTileInfo;
}

void Qgs3DMapSettings::setShowCameraViewCenter( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowCameraViewCenter == enabled )
    return;

  mShowCameraViewCenter = enabled;
  emit showCameraViewCenterChanged();
}

bool Qgs3DMapSettings::showCameraViewCenter() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowCameraViewCenter;
}

void Qgs3DMapSettings::setShowCameraRotationCenter( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowCameraRotationCenter == enabled )
    return;

  mShowCameraRotationCenter = enabled;
  emit showCameraRotationCenterChanged();
}

bool Qgs3DMapSettings::showCameraRotationCenter() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowCameraRotationCenter;
}

void Qgs3DMapSettings::setShowLightSourceOrigins( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowLightSources == enabled )
    return;

  mShowLightSources = enabled;
  emit showLightSourceOriginsChanged();
}

bool Qgs3DMapSettings::showLightSourceOrigins() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowLightSources;
}

void Qgs3DMapSettings::setShowLabels( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowLabels == enabled )
    return;

  mShowLabels = enabled;
  emit showLabelsChanged();
}

bool Qgs3DMapSettings::showLabels() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowLabels;
}

void Qgs3DMapSettings::setStopUpdates( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mStopUpdates == enabled )
    return;

  mStopUpdates = enabled;
  emit stopUpdatesChanged();
}

bool Qgs3DMapSettings::stopUpdates() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mStopUpdates;
}

void Qgs3DMapSettings::setEyeDomeLightingEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mEyeDomeLightingEnabled == enabled )
    return;
  mEyeDomeLightingEnabled = enabled;
  emit eyeDomeLightingEnabledChanged();
}

bool Qgs3DMapSettings::eyeDomeLightingEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mEyeDomeLightingEnabled;
}

void Qgs3DMapSettings::setEyeDomeLightingStrength( double strength )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mEyeDomeLightingStrength == strength )
    return;
  mEyeDomeLightingStrength = strength;
  emit eyeDomeLightingStrengthChanged();
}

double Qgs3DMapSettings::eyeDomeLightingStrength() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mEyeDomeLightingStrength;
}

void Qgs3DMapSettings::setEyeDomeLightingDistance( int distance )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mEyeDomeLightingDistance == distance )
    return;
  mEyeDomeLightingDistance = distance;
  emit eyeDomeLightingDistanceChanged();
}

int Qgs3DMapSettings::eyeDomeLightingDistance() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mEyeDomeLightingDistance;
}

QList<QgsLightSource *> Qgs3DMapSettings::lightSources() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLightSources;
}

void Qgs3DMapSettings::setLightSources( const QList<QgsLightSource *> &lights )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // have lights actually changed?
  if ( mLightSources.count() == lights.count() )
  {
    bool same = true;
    for ( int i = 0; i < mLightSources.count(); ++i )
    {
      if ( mLightSources[i]->type() == lights[i]->type() )
      {
        switch ( mLightSources[i]->type() )
        {
          case Qgis::LightSourceType::Point:
            if ( *static_cast<QgsPointLightSettings *>( mLightSources[i] ) == *static_cast<QgsPointLightSettings *>( lights[i] ) )
              continue;
            break;
          case Qgis::LightSourceType::Directional:
            if ( *static_cast<QgsDirectionalLightSettings *>( mLightSources[i] ) == *static_cast<QgsDirectionalLightSettings *>( lights[i] ) )
              continue;
            break;
        }
      }
      same = false;
      break;
    }
    if ( same )
    {
      qDeleteAll( lights );
      return;
    }
  }

  qDeleteAll( mLightSources );
  mLightSources = lights;

  emit directionalLightsChanged();
  emit pointLightsChanged();
  emit lightSourcesChanged();
}

float Qgs3DMapSettings::fieldOfView() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFieldOfView;
}

void Qgs3DMapSettings::setFieldOfView( const float fieldOfView )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mFieldOfView == fieldOfView )
    return;

  mFieldOfView = fieldOfView;
  emit fieldOfViewChanged();
}

Qt3DRender::QCameraLens::ProjectionType Qgs3DMapSettings::projectionType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mProjectionType;
}

void Qgs3DMapSettings::setProjectionType( const Qt3DRender::QCameraLens::ProjectionType projectionType )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mProjectionType == projectionType )
    return;

  mProjectionType = projectionType;
  emit projectionTypeChanged();
}

Qgis::NavigationMode Qgs3DMapSettings::cameraNavigationMode() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCameraNavigationMode;
}

void Qgs3DMapSettings::setCameraNavigationMode( Qgis::NavigationMode navigationMode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mCameraNavigationMode == navigationMode )
    return;

  mCameraNavigationMode = navigationMode;
  emit cameraNavigationModeChanged();
}

double Qgs3DMapSettings::cameraMovementSpeed() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCameraMovementSpeed;
}

void Qgs3DMapSettings::setCameraMovementSpeed( double movementSpeed )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mCameraMovementSpeed == movementSpeed )
    return;

  mCameraMovementSpeed = movementSpeed;
  emit cameraMovementSpeedChanged();
}

void Qgs3DMapSettings::setOutputDpi( const double dpi )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDpi = dpi;
}

double Qgs3DMapSettings::outputDpi() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDpi;
}

QgsSkyboxSettings Qgs3DMapSettings::skyboxSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSkyboxSettings;
}

QgsShadowSettings Qgs3DMapSettings::shadowSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShadowSettings;
}

QgsAmbientOcclusionSettings Qgs3DMapSettings::ambientOcclusionSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAmbientOcclusionSettings;
}

void Qgs3DMapSettings::setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSkyboxSettings = skyboxSettings;
  emit skyboxSettingsChanged();
}

void Qgs3DMapSettings::setShadowSettings( const QgsShadowSettings &shadowSettings )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mShadowSettings = shadowSettings;
  emit shadowSettingsChanged();
}

void Qgs3DMapSettings::setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &ambientOcclusionSettings )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mAmbientOcclusionSettings = ambientOcclusionSettings;
  emit ambientOcclusionSettingsChanged();
}

bool Qgs3DMapSettings::isSkyboxEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsSkyboxEnabled;
}

void Qgs3DMapSettings::setIsSkyboxEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mIsSkyboxEnabled = enabled;
}

bool Qgs3DMapSettings::isFpsCounterEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsFpsCounterEnabled;
}

void Qgs3DMapSettings::setShowDebugPanel( const bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mShowDebugPanel == enabled )
    return;

  mShowDebugPanel = enabled;
  emit showDebugPanelChanged( enabled );
}

bool Qgs3DMapSettings::showDebugPanel() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowDebugPanel;
}

void Qgs3DMapSettings::setDebugShadowMapSettings( bool enabled, Qt::Corner corner, double size )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDebugShadowMapEnabled = enabled;
  mDebugShadowMapCorner = corner;
  mDebugShadowMapSize = size;
  emit debugShadowMapSettingsChanged();
}

bool Qgs3DMapSettings::debugShadowMapEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugShadowMapEnabled;
}

Qt::Corner Qgs3DMapSettings::debugShadowMapCorner() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugShadowMapCorner;
}

double Qgs3DMapSettings::debugShadowMapSize() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugShadowMapSize;
}

void Qgs3DMapSettings::setDebugDepthMapSettings( bool enabled, Qt::Corner corner, double size )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDebugDepthMapEnabled = enabled;
  mDebugDepthMapCorner = corner;
  mDebugDepthMapSize = size;
  emit debugDepthMapSettingsChanged();
}

bool Qgs3DMapSettings::debugDepthMapEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugDepthMapEnabled;
}

Qt::Corner Qgs3DMapSettings::debugDepthMapCorner() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugDepthMapCorner;
}

double Qgs3DMapSettings::debugDepthMapSize() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDebugDepthMapSize;
}

void Qgs3DMapSettings::setIsFpsCounterEnabled( bool fpsCounterEnabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( fpsCounterEnabled == mIsFpsCounterEnabled )
    return;
  mIsFpsCounterEnabled = fpsCounterEnabled;
  emit fpsCounterEnabledChanged( mIsFpsCounterEnabled );
}

bool Qgs3DMapSettings::terrainRenderingEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTerrainRenderingEnabled;
}

void Qgs3DMapSettings::setTerrainRenderingEnabled( bool terrainRenderingEnabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( terrainRenderingEnabled == mTerrainRenderingEnabled )
    return;
  mTerrainRenderingEnabled = terrainRenderingEnabled;
  emit terrainGeneratorChanged();
}

Qgis::RendererUsage Qgs3DMapSettings::rendererUsage() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRendererUsage;
}

void Qgs3DMapSettings::setRendererUsage( Qgis::RendererUsage rendererUsage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mRendererUsage = rendererUsage;
}

Qgis::ViewSyncModeFlags Qgs3DMapSettings::viewSyncMode() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mViewSyncMode;
}

void Qgs3DMapSettings::setViewSyncMode( Qgis::ViewSyncModeFlags mode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mViewSyncMode = mode;
}

bool Qgs3DMapSettings::viewFrustumVisualizationEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mVisualizeViewFrustum;
}

void Qgs3DMapSettings::setViewFrustumVisualizationEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mVisualizeViewFrustum != enabled )
  {
    mVisualizeViewFrustum = enabled;
    emit viewFrustumVisualizationEnabledChanged();
  }
}

Qgs3DAxisSettings Qgs3DMapSettings::get3DAxisSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return m3dAxisSettings;
}

void Qgs3DMapSettings::setIsDebugOverlayEnabled( bool debugOverlayEnabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( debugOverlayEnabled == mIsDebugOverlayEnabled )
    return;

  mIsDebugOverlayEnabled = debugOverlayEnabled;
  emit debugOverlayEnabledChanged( mIsDebugOverlayEnabled );
}

bool Qgs3DMapSettings::showExtentIn2DView() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShowExtentIn2DView;
}

void Qgs3DMapSettings::connectChangedSignalsToSettingsChanged()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  connect( this, &Qgs3DMapSettings::selectionColorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::layersChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainGeneratorChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::terrainSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
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
  connect( this, &Qgs3DMapSettings::stopUpdatesChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::showDebugPanelChanged, this, &Qgs3DMapSettings::settingsChanged );
}


void Qgs3DMapSettings::set3DAxisSettings( const Qgs3DAxisSettings &axisSettings, bool force )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

bool Qgs3DMapSettings::isDebugOverlayEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsDebugOverlayEnabled;
}

void Qgs3DMapSettings::setShowExtentIn2DView( bool show )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( show == mShowExtentIn2DView )
    return;

  mShowExtentIn2DView = show;
  emit showExtentIn2DViewChanged();
}
