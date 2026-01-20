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

#include "qgs3d.h"
#include "qgs3drendercontext.h"
#include "qgs3dterrainregistry.h"
#include "qgs3dutils.h"
#include "qgsabstractterrainsettings.h"
#include "qgscolorutils.h"
#include "qgsdirectionallightsettings.h"
#include "qgsflatterrainsettings.h"
#include "qgslightsource.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgspointlightsettings.h"
#include "qgsprojectelevationproperties.h"
#include "qgsprojectviewsettings.h"
#include "qgsrasterlayer.h"
#include "qgsterrainprovider.h"
#include "qgsthreadingutils.h"

#include <QDomDocument>
#include <QDomElement>

#include "moc_qgs3dmapsettings.cpp"

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
  QDomElement elemOrigin = elem.firstChildElement( u"origin"_s );
  mOrigin = QgsVector3D(
    elemOrigin.attribute( u"x"_s ).toDouble(),
    elemOrigin.attribute( u"y"_s ).toDouble(),
    elemOrigin.attribute( u"z"_s ).toDouble()
  );

  QDomElement elemExtent = elem.firstChildElement( u"extent"_s );
  if ( !elemExtent.isNull() )
  {
    mExtent = QgsRectangle(
      elemExtent.attribute( u"xMin"_s ).toDouble(),
      elemExtent.attribute( u"yMin"_s ).toDouble(),
      elemExtent.attribute( u"xMax"_s ).toDouble(),
      elemExtent.attribute( u"yMax"_s ).toDouble()
    );

    mShowExtentIn2DView = elemExtent.attribute( u"showIn2dView"_s, u"0"_s ).toInt();
  }
  else
  {
    mExtent = QgsProject::instance()->viewSettings()->fullExtent();
  }

  QDomElement elemCamera = elem.firstChildElement( u"camera"_s );
  if ( !elemCamera.isNull() )
  {
    mFieldOfView = elemCamera.attribute( u"field-of-view"_s, u"45"_s ).toFloat();
    mProjectionType = static_cast<Qt3DRender::QCameraLens::ProjectionType>( elemCamera.attribute( u"projection-type"_s, u"1"_s ).toInt() );
    QString cameraNavigationMode = elemCamera.attribute( u"camera-navigation-mode"_s, u"basic-navigation"_s );
    if ( cameraNavigationMode == "terrain-based-navigation"_L1 )
      mCameraNavigationMode = Qgis::NavigationMode::TerrainBased;
    else if ( cameraNavigationMode == "walk-navigation"_L1 )
      mCameraNavigationMode = Qgis::NavigationMode::Walk;
    else if ( cameraNavigationMode == "globe-terrain-based-navigation"_L1 )
      mCameraNavigationMode = Qgis::NavigationMode::GlobeTerrainBased;
    mCameraMovementSpeed = elemCamera.attribute( u"camera-movement-speed"_s, u"5.0"_s ).toDouble();
  }

  QDomElement elemColor = elem.firstChildElement( u"color"_s );
  if ( !elemColor.isNull() )
  {
    mBackgroundColor = QgsColorUtils::colorFromString( elemColor.attribute( u"background"_s ) );
    mSelectionColor = QgsColorUtils::colorFromString( elemColor.attribute( u"selection"_s ) );
  }

  QDomElement elemCrs = elem.firstChildElement( u"crs"_s );
  mCrs.readXml( elemCrs );

  QDomElement elemTerrain = elem.firstChildElement( u"terrain"_s );
  mTerrainRenderingEnabled = elemTerrain.attribute( u"terrain-rendering-enabled"_s, u"1"_s ).toInt();
  mTerrainShadingEnabled = elemTerrain.attribute( u"shading-enabled"_s, u"0"_s ).toInt();

  QDomElement elemTerrainShadingMaterial = elemTerrain.firstChildElement( u"shading-material"_s );
  if ( !elemTerrainShadingMaterial.isNull() )
    mTerrainShadingMaterial.readXml( elemTerrainShadingMaterial, context );
  mTerrainMapTheme = elemTerrain.attribute( u"map-theme"_s );
  mShowLabels = elemTerrain.attribute( u"show-labels"_s, u"0"_s ).toInt();

  qDeleteAll( mLightSources );
  mLightSources.clear();
  const QDomElement lightsElem = elem.firstChildElement( u"lights"_s );
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
    QDomElement elemPointLights = elem.firstChildElement( u"point-lights"_s );
    if ( !elemPointLights.isNull() )
    {
      QDomElement elemPointLight = elemPointLights.firstChildElement( u"point-light"_s );
      while ( !elemPointLight.isNull() )
      {
        auto pointLight = std::make_unique<QgsPointLightSettings>();
        pointLight->readXml( elemPointLight, context );
        mLightSources << pointLight.release();
        elemPointLight = elemPointLight.nextSiblingElement( u"point-light"_s );
      }
    }
    else
    {
      // QGIS <= 3.4 did not have light configuration
      auto defaultLight = std::make_unique<QgsPointLightSettings>();
      defaultLight->setPosition( QgsVector3D( 0, 1000, 0 ) );
      mLightSources << defaultLight.release();
    }

    QDomElement elemDirectionalLights = elem.firstChildElement( u"directional-lights"_s );
    if ( !elemDirectionalLights.isNull() )
    {
      QDomElement elemDirectionalLight = elemDirectionalLights.firstChildElement( u"directional-light"_s );
      while ( !elemDirectionalLight.isNull() )
      {
        auto directionalLight = std::make_unique<QgsDirectionalLightSettings>();
        directionalLight->readXml( elemDirectionalLight, context );
        mLightSources << directionalLight.release();
        elemDirectionalLight = elemDirectionalLight.nextSiblingElement( u"directional-light"_s );
      }
    }
  }

  QDomElement elemMapLayers = elemTerrain.firstChildElement( u"layers"_s );
  QDomElement elemMapLayer = elemMapLayers.firstChildElement( u"layer"_s );
  QList<QgsMapLayerRef> mapLayers;
  while ( !elemMapLayer.isNull() )
  {
    mapLayers << QgsMapLayerRef( elemMapLayer.attribute( u"id"_s ) );
    elemMapLayer = elemMapLayer.nextSiblingElement( u"layer"_s );
  }
  mLayers = mapLayers; // needs to resolve refs afterwards

  QDomElement elemTerrainGenerator = elemTerrain.firstChildElement( u"generator"_s );
  const QString terrainGenType = elemTerrainGenerator.attribute( u"type"_s );
  std::unique_ptr<QgsAbstractTerrainSettings> terrainSettings( Qgs3D::terrainRegistry()->createTerrainSettings( terrainGenType ) );
  if ( terrainSettings )
  {
    terrainSettings->readXml( elemTerrain, context );
    setTerrainSettings( terrainSettings.release() );
  }

  QDomElement elemSkybox = elem.firstChildElement( u"skybox"_s );
  mIsSkyboxEnabled = elemSkybox.attribute( u"skybox-enabled"_s ).toInt();
  mSkyboxSettings.readXml( elemSkybox, context );

  QDomElement elemShadows = elem.firstChildElement( u"shadow-rendering"_s );
  mShadowSettings.readXml( elemShadows, context );

  QDomElement elemAmbientOcclusion = elem.firstChildElement( u"screen-space-ambient-occlusion"_s );
  mAmbientOcclusionSettings.readXml( elemAmbientOcclusion, context );

  QDomElement elemEyeDomeLighting = elem.firstChildElement( u"eye-dome-lighting"_s );
  mEyeDomeLightingEnabled = elemEyeDomeLighting.attribute( "enabled", u"0"_s ).toInt();
  mEyeDomeLightingStrength = elemEyeDomeLighting.attribute( "eye-dome-lighting-strength", u"1000.0"_s ).toDouble();
  mEyeDomeLightingDistance = elemEyeDomeLighting.attribute( "eye-dome-lighting-distance", u"1"_s ).toInt();

  QDomElement elemNavigationSync = elem.firstChildElement( u"navigation-sync"_s );
  mViewSyncMode = ( Qgis::ViewSyncModeFlags )( elemNavigationSync.attribute( u"view-sync-mode"_s, u"0"_s ).toInt() );
  mVisualizeViewFrustum = elemNavigationSync.attribute( u"view-frustum-visualization-enabled"_s, u"0"_s ).toInt();

  QDomElement elemDebugSettings = elem.firstChildElement( u"debug-settings"_s );
  mDebugShadowMapEnabled = elemDebugSettings.attribute( u"shadowmap-enabled"_s, u"0"_s ).toInt();
  mDebugShadowMapCorner = static_cast<Qt::Corner>( elemDebugSettings.attribute( u"shadowmap-corner"_s, "0" ).toInt() );
  mDebugShadowMapSize = elemDebugSettings.attribute( u"shadowmap-size"_s, u"0.2"_s ).toDouble();

  mDebugDepthMapEnabled = elemDebugSettings.attribute( u"depthmap-enabled"_s, u"0"_s ).toInt();
  mDebugDepthMapCorner = static_cast<Qt::Corner>( elemDebugSettings.attribute( u"depthmap-corner"_s, u"1"_s ).toInt() );
  mDebugDepthMapSize = elemDebugSettings.attribute( u"depthmap-size"_s, u"0.2"_s ).toDouble();

  QDomElement elemDebug = elem.firstChildElement( u"debug"_s );
  mShowTerrainBoundingBoxes = elemDebug.attribute( u"bounding-boxes"_s, u"0"_s ).toInt();
  mShowTerrainTileInfo = elemDebug.attribute( u"terrain-tile-info"_s, u"0"_s ).toInt();
  mShowCameraViewCenter = elemDebug.attribute( u"camera-view-center"_s, u"0"_s ).toInt();
  mShowCameraRotationCenter = elemDebug.attribute( u"camera-rotation-center"_s, u"0"_s ).toInt();
  mShowLightSources = elemDebug.attribute( u"show-light-sources"_s, u"0"_s ).toInt();
  mIsFpsCounterEnabled = elemDebug.attribute( u"show-fps-counter"_s, u"0"_s ).toInt();
  mStopUpdates = elemDebug.attribute( u"stop-updates"_s, u"0"_s ).toInt();
  mShowDebugPanel = elemDebug.attribute( u"debug-panel"_s, u"0"_s ).toInt();

  QDomElement elemTemporalRange = elem.firstChildElement( u"temporal-range"_s );
  QDateTime start = QDateTime::fromString( elemTemporalRange.attribute( u"start"_s ), Qt::ISODate );
  QDateTime end = QDateTime::fromString( elemTemporalRange.attribute( u"end"_s ), Qt::ISODate );
  setTemporalRange( QgsDateTimeRange( start, end ) );

  QDomElement elem3dAxis = elem.firstChildElement( u"axis3d"_s );
  m3dAxisSettings.readXml( elem3dAxis, context );
}

QDomElement Qgs3DMapSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement elem = doc.createElement( u"qgis3d"_s );

  QDomElement elemOrigin = doc.createElement( u"origin"_s );
  elemOrigin.setAttribute( u"x"_s, QString::number( mOrigin.x() ) );
  elemOrigin.setAttribute( u"y"_s, QString::number( mOrigin.y() ) );
  elemOrigin.setAttribute( u"z"_s, QString::number( mOrigin.z() ) );
  elem.appendChild( elemOrigin );

  QDomElement elemExtent = doc.createElement( u"extent"_s );
  elemExtent.setAttribute( u"xMin"_s, mExtent.xMinimum() );
  elemExtent.setAttribute( u"yMin"_s, mExtent.yMinimum() );
  elemExtent.setAttribute( u"xMax"_s, mExtent.xMaximum() );
  elemExtent.setAttribute( u"yMax"_s, mExtent.yMaximum() );
  elemExtent.setAttribute( u"showIn2dView"_s, mShowExtentIn2DView );
  elem.appendChild( elemExtent );

  QDomElement elemCamera = doc.createElement( u"camera"_s );
  elemCamera.setAttribute( u"field-of-view"_s, mFieldOfView );
  elemCamera.setAttribute( u"projection-type"_s, static_cast<int>( mProjectionType ) );
  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::TerrainBased:
      elemCamera.setAttribute( u"camera-navigation-mode"_s, u"terrain-based-navigation"_s );
      break;
    case Qgis::NavigationMode::Walk:
      elemCamera.setAttribute( u"camera-navigation-mode"_s, u"walk-navigation"_s );
      break;
    case Qgis::NavigationMode::GlobeTerrainBased:
      elemCamera.setAttribute( u"camera-navigation-mode"_s, u"globe-terrain-based-navigation"_s );
      break;
  }
  elemCamera.setAttribute( u"camera-movement-speed"_s, mCameraMovementSpeed );
  elem.appendChild( elemCamera );

  QDomElement elemColor = doc.createElement( u"color"_s );
  elemColor.setAttribute( u"background"_s, QgsColorUtils::colorToString( mBackgroundColor ) );
  elemColor.setAttribute( u"selection"_s, QgsColorUtils::colorToString( mSelectionColor ) );
  elem.appendChild( elemColor );

  QDomElement elemCrs = doc.createElement( u"crs"_s );
  mCrs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( u"terrain"_s );
  elemTerrain.setAttribute( u"terrain-rendering-enabled"_s, mTerrainRenderingEnabled ? 1 : 0 );
  elemTerrain.setAttribute( u"shading-enabled"_s, mTerrainShadingEnabled ? 1 : 0 );

  QDomElement elemTerrainShadingMaterial = doc.createElement( u"shading-material"_s );
  mTerrainShadingMaterial.writeXml( elemTerrainShadingMaterial, context );
  elemTerrain.appendChild( elemTerrainShadingMaterial );
  elemTerrain.setAttribute( u"map-theme"_s, mTerrainMapTheme );
  elemTerrain.setAttribute( u"show-labels"_s, mShowLabels ? 1 : 0 );

  {
    QDomElement elemLights = doc.createElement( u"lights"_s );
    for ( const QgsLightSource *light : mLightSources )
    {
      const QDomElement elemLight = light->writeXml( doc, context );
      elemLights.appendChild( elemLight );
    }
    elem.appendChild( elemLights );
  }

  QDomElement elemMapLayers = doc.createElement( u"layers"_s );
  for ( const QgsMapLayerRef &layerRef : mLayers )
  {
    QDomElement elemMapLayer = doc.createElement( u"layer"_s );
    elemMapLayer.setAttribute( u"id"_s, layerRef.layerId );
    elemMapLayers.appendChild( elemMapLayer );
  }
  elemTerrain.appendChild( elemMapLayers );

  QDomElement elemTerrainGenerator = doc.createElement( u"generator"_s );
  elemTerrainGenerator.setAttribute( u"type"_s, mTerrainSettings->type() );
  mTerrainSettings->writeXml( elemTerrain, context );
  elemTerrain.appendChild( elemTerrainGenerator );
  elem.appendChild( elemTerrain );

  QDomElement elemSkybox = doc.createElement( u"skybox"_s );
  elemSkybox.setAttribute( u"skybox-enabled"_s, mIsSkyboxEnabled );
  mSkyboxSettings.writeXml( elemSkybox, context );
  elem.appendChild( elemSkybox );

  QDomElement elemShadows = doc.createElement( u"shadow-rendering"_s );
  mShadowSettings.writeXml( elemShadows, context );
  elem.appendChild( elemShadows );

  QDomElement elemAmbientOcclusion = doc.createElement( u"screen-space-ambient-occlusion"_s );
  mAmbientOcclusionSettings.writeXml( elemAmbientOcclusion, context );
  elem.appendChild( elemAmbientOcclusion );

  QDomElement elemDebug = doc.createElement( u"debug"_s );
  elemDebug.setAttribute( u"bounding-boxes"_s, mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( u"terrain-tile-info"_s, mShowTerrainTileInfo ? 1 : 0 );
  elemDebug.setAttribute( u"camera-view-center"_s, mShowCameraViewCenter ? 1 : 0 );
  elemDebug.setAttribute( u"camera-rotation-center"_s, mShowCameraRotationCenter ? 1 : 0 );
  elemDebug.setAttribute( u"show-light-sources"_s, mShowLightSources ? 1 : 0 );
  elemDebug.setAttribute( u"show-fps-counter"_s, mIsFpsCounterEnabled ? 1 : 0 );
  elemDebug.setAttribute( u"stop-updates"_s, mStopUpdates ? 1 : 0 );
  elemDebug.setAttribute( u"debug-panel"_s, mShowDebugPanel ? 1 : 0 );
  elem.appendChild( elemDebug );

  QDomElement elemEyeDomeLighting = doc.createElement( u"eye-dome-lighting"_s );
  elemEyeDomeLighting.setAttribute( u"enabled"_s, mEyeDomeLightingEnabled ? 1 : 0 );
  elemEyeDomeLighting.setAttribute( u"eye-dome-lighting-strength"_s, mEyeDomeLightingStrength );
  elemEyeDomeLighting.setAttribute( u"eye-dome-lighting-distance"_s, mEyeDomeLightingDistance );
  elem.appendChild( elemEyeDomeLighting );

  QDomElement elemNavigationSync = doc.createElement( u"navigation-sync"_s );
  elemNavigationSync.setAttribute( u"view-sync-mode"_s, ( int ) mViewSyncMode );
  elemNavigationSync.setAttribute( u"view-frustum-visualization-enabled"_s, mVisualizeViewFrustum ? 1 : 0 );
  elem.appendChild( elemNavigationSync );

  QDomElement elemDebugSettings = doc.createElement( u"debug-settings"_s );
  elemDebugSettings.setAttribute( u"shadowmap-enabled"_s, mDebugShadowMapEnabled );
  elemDebugSettings.setAttribute( u"shadowmap-corner"_s, mDebugShadowMapCorner );
  elemDebugSettings.setAttribute( u"shadowmap-size"_s, mDebugShadowMapSize );
  elemDebugSettings.setAttribute( u"depthmap-enabled"_s, mDebugDepthMapEnabled );
  elemDebugSettings.setAttribute( u"depthmap-corner"_s, mDebugDepthMapCorner );
  elemDebugSettings.setAttribute( u"depthmap-size"_s, mDebugDepthMapSize );
  elem.appendChild( elemDebugSettings );

  QDomElement elemTemporalRange = doc.createElement( u"temporal-range"_s );
  elemTemporalRange.setAttribute( u"start"_s, temporalRange().begin().toString( Qt::ISODate ) );
  elemTemporalRange.setAttribute( u"end"_s, temporalRange().end().toString( Qt::ISODate ) );

  QDomElement elem3dAxis = doc.createElement( u"axis3d"_s );
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

  if ( sceneMode() == Qgis::SceneMode::Globe )
  {
    QgsDebugError( u"extent() should not be used with globe!"_s );
  }

  return mExtent;
}

void Qgs3DMapSettings::setExtent( const QgsRectangle &extent )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( extent == mExtent )
    return;

  if ( sceneMode() == Qgis::SceneMode::Globe )
  {
    QgsDebugError( u"setExtent() should not be used with globe!"_s );
  }

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

  // for convenience, make sure the navigation mode is consistent with the scene mode
  if ( sceneMode() == Qgis::SceneMode::Globe && mCameraNavigationMode == Qgis::NavigationMode::TerrainBased )
  {
    setCameraNavigationMode( Qgis::NavigationMode::GlobeTerrainBased );
  }
  else if ( sceneMode() == Qgis::SceneMode::Local && mCameraNavigationMode == Qgis::NavigationMode::GlobeTerrainBased )
  {
    setCameraNavigationMode( Qgis::NavigationMode::TerrainBased );
  }
}

QgsCoordinateReferenceSystem Qgs3DMapSettings::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

Qgis::SceneMode Qgs3DMapSettings::sceneMode() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs.type() == Qgis::CrsType::Geocentric ? Qgis::SceneMode::Globe : Qgis::SceneMode::Local;
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
    // ensure to generate the terrain if the settings have changed or if the terrain has never been generated.
    hasChanged = !settings->equals( mTerrainSettings.get() ) || !mTerrainGenerator;
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
