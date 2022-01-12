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
#include "qgsvectorlayer3drenderer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgspointcloudlayer3drenderer.h"

#include <QDomDocument>
#include <QDomElement>

#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"

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
  , mPointLights( other.mPointLights )
  , mDirectionalLights( other.mDirectionalLights )
  , mFieldOfView( other.mFieldOfView )
  , mProjectionType( other.mProjectionType )
  , mCameraNavigationMode( other.mCameraNavigationMode )
  , mCameraMovementSpeed( other.mCameraMovementSpeed )
  , mLayers( other.mLayers )
  , mRenderers() // initialized in body
  , mTransformContext( other.mTransformContext )
  , mPathResolver( other.mPathResolver )
  , mMapThemes( other.mMapThemes )
  , mDpi( other.mDpi )
  , mIsFpsCounterEnabled( other.mIsFpsCounterEnabled )
  , mIsSkyboxEnabled( other.mIsSkyboxEnabled )
  , mSkyboxSettings( other.mSkyboxSettings )
  , mShadowSettings( other.mShadowSettings )
  , mEyeDomeLightingEnabled( other.mEyeDomeLightingEnabled )
  , mEyeDomeLightingStrength( other.mEyeDomeLightingStrength )
  , mEyeDomeLightingDistance( other.mEyeDomeLightingDistance )
  , mDebugShadowMapEnabled( other.mDebugShadowMapEnabled )
  , mDebugShadowMapCorner( other.mDebugShadowMapCorner )
  , mDebugShadowMapSize( other.mDebugShadowMapSize )
  , mDebugDepthMapEnabled( other.mDebugDepthMapEnabled )
  , mDebugDepthMapCorner( other.mDebugDepthMapCorner )
  , mDebugDepthMapSize( other.mDebugDepthMapSize )
  , mTerrainRenderingEnabled( other.mTerrainRenderingEnabled )
  , mRendererUsage( other.mRendererUsage )
{
  for ( QgsAbstract3DRenderer *renderer : std::as_const( other.mRenderers ) )
  {
    mRenderers << renderer->clone();
  }

  connect( this, &Qgs3DMapSettings::settingsChanged, [&]()
  {
    QgsProject::instance()->setDirty();
  } );
  connectChangedSignalsToSettingsChanged();
}

Qgs3DMapSettings::~Qgs3DMapSettings()
{
  qDeleteAll( mRenderers );
}

void Qgs3DMapSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QgsProjectDirtyBlocker blocker( QgsProject::instance() );
  QDomElement elemOrigin = elem.firstChildElement( QStringLiteral( "origin" ) );
  mOrigin = QgsVector3D(
              elemOrigin.attribute( QStringLiteral( "x" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "y" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "z" ) ).toDouble() );

  QDomElement elemCamera = elem.firstChildElement( QStringLiteral( "camera" ) );
  if ( !elemCamera.isNull() )
  {
    mFieldOfView = elemCamera.attribute( QStringLiteral( "field-of-view" ), QStringLiteral( "45" ) ).toFloat();
    mProjectionType = static_cast< Qt3DRender::QCameraLens::ProjectionType >( elemCamera.attribute( QStringLiteral( "projection-type" ), QStringLiteral( "1" ) ).toInt() );
    QString cameraNavigationMode = elemCamera.attribute( QStringLiteral( "camera-navigation-mode" ), QStringLiteral( "basic-navigation" ) );
    if ( cameraNavigationMode == QLatin1String( "terrain-based-navigation" ) )
      mCameraNavigationMode = QgsCameraController::NavigationMode::TerrainBasedNavigation;
    else if ( cameraNavigationMode == QLatin1String( "walk-navigation" ) )
      mCameraNavigationMode = QgsCameraController::NavigationMode::WalkNavigation;
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

  mPointLights.clear();
  QDomElement elemPointLights = elem.firstChildElement( QStringLiteral( "point-lights" ) );
  if ( !elemPointLights.isNull() )
  {
    QDomElement elemPointLight = elemPointLights.firstChildElement( QStringLiteral( "point-light" ) );
    while ( !elemPointLight.isNull() )
    {
      QgsPointLightSettings pointLight;
      pointLight.readXml( elemPointLight );
      mPointLights << pointLight;
      elemPointLight = elemPointLight.nextSiblingElement( QStringLiteral( "point-light" ) );
    }
  }
  else
  {
    // QGIS <= 3.4 did not have light configuration
    QgsPointLightSettings defaultLight;
    defaultLight.setPosition( QgsVector3D( 0, 1000, 0 ) );
    mPointLights << defaultLight;
  }

  mDirectionalLights.clear();
  QDomElement elemDirectionalLights = elem.firstChildElement( QStringLiteral( "directional-lights" ) );
  if ( !elemDirectionalLights.isNull() )
  {
    QDomElement elemDirectionalLight = elemDirectionalLights.firstChildElement( QStringLiteral( "directional-light" ) );
    while ( !elemDirectionalLight.isNull() )
    {
      QgsDirectionalLightSettings directionalLight;
      directionalLight.readXml( elemDirectionalLight );
      mDirectionalLights << directionalLight;
      elemDirectionalLight = elemDirectionalLight.nextSiblingElement( QStringLiteral( "directional-light" ) );
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

  qDeleteAll( mRenderers );
  mRenderers.clear();

  QDomElement elemRenderers = elem.firstChildElement( QStringLiteral( "renderers" ) );
  QDomElement elemRenderer = elemRenderers.firstChildElement( QStringLiteral( "renderer" ) );
  while ( !elemRenderer.isNull() )
  {
    QgsAbstract3DRenderer *renderer = nullptr;
    QString type = elemRenderer.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "vector" ) )
    {
      renderer = new QgsVectorLayer3DRenderer;
    }
    else if ( type == QLatin1String( "mesh" ) )
    {
      renderer = new QgsMeshLayer3DRenderer;
    }
    else if ( type == QLatin1String( "pointcloud" ) )
    {
      renderer = new QgsPointCloudLayer3DRenderer;
    }

    if ( renderer )
    {
      renderer->readXml( elemRenderer, context );
      mRenderers.append( renderer );
    }
    elemRenderer = elemRenderer.nextSiblingElement( QStringLiteral( "renderer" ) );
  }

  QDomElement elemSkybox = elem.firstChildElement( QStringLiteral( "skybox" ) );
  mIsSkyboxEnabled = elemSkybox.attribute( QStringLiteral( "skybox-enabled" ) ).toInt();
  mSkyboxSettings.readXml( elemSkybox, context );

  QDomElement elemShadows = elem.firstChildElement( QStringLiteral( "shadow-rendering" ) );
  mShadowSettings.readXml( elemShadows, context );

  QDomElement elemEyeDomeLighting = elem.firstChildElement( QStringLiteral( "eye-dome-lighting" ) );
  mEyeDomeLightingEnabled = elemEyeDomeLighting.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
  mEyeDomeLightingStrength = elemEyeDomeLighting.attribute( "eye-dome-lighting-strength", QStringLiteral( "1000.0" ) ).toDouble();
  mEyeDomeLightingDistance = elemEyeDomeLighting.attribute( "eye-dome-lighting-distance", QStringLiteral( "1" ) ).toInt();

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
}

QDomElement Qgs3DMapSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "qgis3d" ) );

  QDomElement elemOrigin = doc.createElement( QStringLiteral( "origin" ) );
  elemOrigin.setAttribute( QStringLiteral( "x" ), QString::number( mOrigin.x() ) );
  elemOrigin.setAttribute( QStringLiteral( "y" ), QString::number( mOrigin.y() ) );
  elemOrigin.setAttribute( QStringLiteral( "z" ), QString::number( mOrigin.z() ) );
  elem.appendChild( elemOrigin );

  QDomElement elemCamera = doc.createElement( QStringLiteral( "camera" ) );
  elemCamera.setAttribute( QStringLiteral( "field-of-view" ), mFieldOfView );
  elemCamera.setAttribute( QStringLiteral( "projection-type" ), static_cast< int >( mProjectionType ) );
  switch ( mCameraNavigationMode )
  {
    case QgsCameraController::TerrainBasedNavigation:
      elemCamera.setAttribute( QStringLiteral( "camera-navigation-mode" ), QStringLiteral( "terrain-based-navigation" ) );
      break;
    case QgsCameraController::WalkNavigation:
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

  QDomElement elemPointLights = doc.createElement( QStringLiteral( "point-lights" ) );
  for ( const QgsPointLightSettings &pointLight : std::as_const( mPointLights ) )
  {
    QDomElement elemPointLight = pointLight.writeXml( doc );
    elemPointLights.appendChild( elemPointLight );
  }
  elem.appendChild( elemPointLights );

  QDomElement elemDirectionalLights = doc.createElement( QStringLiteral( "directional-lights" ) );
  for ( const QgsDirectionalLightSettings &directionalLight : std::as_const( mDirectionalLights ) )
  {
    QDomElement elemDirectionalLight = directionalLight.writeXml( doc );
    elemDirectionalLights.appendChild( elemDirectionalLight );
  }
  elem.appendChild( elemDirectionalLights );

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

  QDomElement elemRenderers = doc.createElement( QStringLiteral( "renderers" ) );
  for ( const QgsAbstract3DRenderer *renderer : mRenderers )
  {
    QDomElement elemRenderer = doc.createElement( QStringLiteral( "renderer" ) );
    elemRenderer.setAttribute( QStringLiteral( "type" ), renderer->type() );
    renderer->writeXml( elemRenderer, context );
    elemRenderers.appendChild( elemRenderer );
  }
  elem.appendChild( elemRenderers );

  QDomElement elemSkybox = doc.createElement( QStringLiteral( "skybox" ) );
  elemSkybox.setAttribute( QStringLiteral( "skybox-enabled" ), mIsSkyboxEnabled );
  mSkyboxSettings.writeXml( elemSkybox, context );
  elem.appendChild( elemSkybox );

  QDomElement elemShadows = doc.createElement( QStringLiteral( "shadow-rendering" ) );
  mShadowSettings.writeXml( elemShadows, context );
  elem.appendChild( elemShadows );

  QDomElement elemDebug = doc.createElement( QStringLiteral( "debug" ) );
  elemDebug.setAttribute( QStringLiteral( "bounding-boxes" ), mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "terrain-tile-info" ), mShowTerrainTileInfo ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "camera-view-center" ), mShowCameraViewCenter ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "camera-rotation-center" ), mShowCameraRotationCenter ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "show-light-sources" ), mShowLightSources ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "show-fps-counter" ), mIsFpsCounterEnabled ? 1 : 0 );
  elem.appendChild( elemDebug );

  QDomElement elemEyeDomeLighting = doc.createElement( QStringLiteral( "eye-dome-lighting" ) );
  elemEyeDomeLighting.setAttribute( "enabled", mEyeDomeLightingEnabled ? 1 : 0 );
  elemEyeDomeLighting.setAttribute( "eye-dome-lighting-strength", mEyeDomeLightingStrength );
  elemEyeDomeLighting.setAttribute( "eye-dome-lighting-distance", mEyeDomeLightingDistance );
  elem.appendChild( elemEyeDomeLighting );


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

  for ( int i = 0; i < mRenderers.count(); ++i )
  {
    QgsAbstract3DRenderer *renderer = mRenderers[i];
    renderer->resolveReferences( project );
  }
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
  QList<QgsMapLayerRef> lst;
  lst.reserve( layers.count() );
  for ( QgsMapLayer *layer : layers )
  {
    lst.append( layer );
  }

  if ( mLayers == lst )
    return;

  mLayers = lst;
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
    disconnect( mTerrainGenerator.get(), &QgsTerrainGenerator::extentChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );
    disconnect( mTerrainGenerator.get(), &QgsTerrainGenerator::terrainChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );
  }

  mTerrainGenerator.reset( gen );
  connect( mTerrainGenerator.get(), &QgsTerrainGenerator::extentChanged, this, &Qgs3DMapSettings::terrainGeneratorChanged );
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

void Qgs3DMapSettings::setRenderers( const QList<QgsAbstract3DRenderer *> &renderers )
{
  qDeleteAll( mRenderers );

  mRenderers = renderers;

  emit renderersChanged();
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

void Qgs3DMapSettings::setPointLights( const QList<QgsPointLightSettings> &pointLights )
{
  if ( mPointLights == pointLights )
    return;

  mPointLights = pointLights;
  emit pointLightsChanged();
}

void Qgs3DMapSettings::setDirectionalLights( const QList<QgsDirectionalLightSettings> &directionalLights )
{
  if ( mDirectionalLights == directionalLights )
    return;

  mDirectionalLights = directionalLights;
  emit directionalLightsChanged();
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

void Qgs3DMapSettings::setCameraNavigationMode( QgsCameraController::NavigationMode navigationMode )
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
  connect( this, &Qgs3DMapSettings::pointLightsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::directionalLightsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::fieldOfViewChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::projectionTypeChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::cameraNavigationModeChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::cameraMovementSpeedChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::skyboxSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::shadowSettingsChanged, this, &Qgs3DMapSettings::settingsChanged );
  connect( this, &Qgs3DMapSettings::fpsCounterEnabledChanged, this, &Qgs3DMapSettings::settingsChanged );
}
