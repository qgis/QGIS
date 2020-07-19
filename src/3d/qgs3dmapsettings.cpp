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

#include <QDomDocument>
#include <QDomElement>

#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"

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
  , mTerrainShadingEnabled( other.mTerrainShadingEnabled )
  , mTerrainShadingMaterial( other.mTerrainShadingMaterial )
  , mTerrainMapTheme( other.mTerrainMapTheme )
  , mShowTerrainBoundingBoxes( other.mShowTerrainBoundingBoxes )
  , mShowTerrainTileInfo( other.mShowTerrainTileInfo )
  , mShowCameraViewCenter( other.mShowCameraViewCenter )
  , mShowLightSources( other.mShowLightSources )
  , mShowLabels( other.mShowLabels )
  , mPointLights( other.mPointLights )
  , mDirectionalLights( other.mDirectionalLights )
  , mFieldOfView( other.mFieldOfView )
  , mLayers( other.mLayers )
  , mRenderers() // initialized in body
  , mSkyboxEnabled( other.mSkyboxEnabled )
  , mSkyboxFileBase( other.mSkyboxFileBase )
  , mSkyboxFileExtension( other.mSkyboxFileExtension )
  , mTransformContext( other.mTransformContext )
  , mPathResolver( other.mPathResolver )
  , mMapThemes( other.mMapThemes )
{
  Q_FOREACH ( QgsAbstract3DRenderer *renderer, other.mRenderers )
  {
    mRenderers << renderer->clone();
  }
}

Qgs3DMapSettings::~Qgs3DMapSettings()
{
  qDeleteAll( mRenderers );
}

void Qgs3DMapSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement elemOrigin = elem.firstChildElement( QStringLiteral( "origin" ) );
  mOrigin = QgsVector3D(
              elemOrigin.attribute( QStringLiteral( "x" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "y" ) ).toDouble(),
              elemOrigin.attribute( QStringLiteral( "z" ) ).toDouble() );

  QDomElement elemCamera = elem.firstChildElement( QStringLiteral( "camera" ) );
  if ( !elemCamera.isNull() )
  {
    mFieldOfView = elemCamera.attribute( QStringLiteral( "field-of-view" ), QStringLiteral( "45" ) ).toFloat();
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
  mTerrainVerticalScale = elemTerrain.attribute( QStringLiteral( "exaggeration" ), QStringLiteral( "1" ) ).toFloat();
  mMapTileResolution = elemTerrain.attribute( QStringLiteral( "texture-size" ), QStringLiteral( "512" ) ).toInt();
  mMaxTerrainScreenError = elemTerrain.attribute( QStringLiteral( "max-terrain-error" ), QStringLiteral( "3" ) ).toFloat();
  mMaxTerrainGroundError = elemTerrain.attribute( QStringLiteral( "max-ground-error" ), QStringLiteral( "1" ) ).toFloat();
  mTerrainShadingEnabled = elemTerrain.attribute( QStringLiteral( "shading-enabled" ), QStringLiteral( "0" ) ).toInt();
  QDomElement elemTerrainShadingMaterial = elemTerrain.firstChildElement( QStringLiteral( "shading-material" ) );
  if ( !elemTerrainShadingMaterial.isNull() )
    mTerrainShadingMaterial.readXml( elemTerrainShadingMaterial );
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
    mTerrainGenerator.reset( demTerrainGenerator );
  }
  else if ( terrainGenType == QLatin1String( "online" ) )
  {
    QgsOnlineTerrainGenerator *onlineTerrainGenerator = new QgsOnlineTerrainGenerator;
    onlineTerrainGenerator->setCrs( mCrs, mTransformContext );
    mTerrainGenerator.reset( onlineTerrainGenerator );
  }
  else if ( terrainGenType == QLatin1String( "mesh" ) )
  {
    QgsMeshTerrainGenerator *meshTerrainGenerator = new QgsMeshTerrainGenerator;
    meshTerrainGenerator->setCrs( mCrs, mTransformContext );
    mTerrainGenerator.reset( meshTerrainGenerator );
  }
  else // "flat"
  {
    QgsFlatTerrainGenerator *flatGen = new QgsFlatTerrainGenerator;
    flatGen->setCrs( mCrs );
    mTerrainGenerator.reset( flatGen );
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

    if ( renderer )
    {
      renderer->readXml( elemRenderer, context );
      mRenderers.append( renderer );
    }
    elemRenderer = elemRenderer.nextSiblingElement( QStringLiteral( "renderer" ) );
  }

  QDomElement elemSkybox = elem.firstChildElement( QStringLiteral( "skybox" ) );
  mSkyboxEnabled = elemSkybox.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mSkyboxFileBase = elemSkybox.attribute( QStringLiteral( "file-base" ) );
  mSkyboxFileExtension = elemSkybox.attribute( QStringLiteral( "file-ext" ) );

  QDomElement elemDebug = elem.firstChildElement( QStringLiteral( "debug" ) );
  mShowTerrainBoundingBoxes = elemDebug.attribute( QStringLiteral( "bounding-boxes" ), QStringLiteral( "0" ) ).toInt();
  mShowTerrainTileInfo = elemDebug.attribute( QStringLiteral( "terrain-tile-info" ), QStringLiteral( "0" ) ).toInt();
  mShowCameraViewCenter = elemDebug.attribute( QStringLiteral( "camera-view-center" ), QStringLiteral( "0" ) ).toInt();
  mShowLightSources = elemDebug.attribute( QStringLiteral( "show-light-sources" ), QStringLiteral( "0" ) ).toInt();

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
  elem.appendChild( elemCamera );

  QDomElement elemColor = doc.createElement( QStringLiteral( "color" ) );
  elemColor.setAttribute( QStringLiteral( "background" ), QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );
  elemColor.setAttribute( QStringLiteral( "selection" ), QgsSymbolLayerUtils::encodeColor( mSelectionColor ) );
  elem.appendChild( elemColor );

  QDomElement elemCrs = doc.createElement( QStringLiteral( "crs" ) );
  mCrs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( QStringLiteral( "terrain" ) );
  elemTerrain.setAttribute( QStringLiteral( "exaggeration" ), QString::number( mTerrainVerticalScale ) );
  elemTerrain.setAttribute( QStringLiteral( "texture-size" ), mMapTileResolution );
  elemTerrain.setAttribute( QStringLiteral( "max-terrain-error" ), QString::number( mMaxTerrainScreenError ) );
  elemTerrain.setAttribute( QStringLiteral( "max-ground-error" ), QString::number( mMaxTerrainGroundError ) );
  elemTerrain.setAttribute( QStringLiteral( "shading-enabled" ), mTerrainShadingEnabled ? 1 : 0 );
  QDomElement elemTerrainShadingMaterial = doc.createElement( QStringLiteral( "shading-material" ) );
  mTerrainShadingMaterial.writeXml( elemTerrainShadingMaterial );
  elemTerrain.appendChild( elemTerrainShadingMaterial );
  elemTerrain.setAttribute( QStringLiteral( "map-theme" ), mTerrainMapTheme );
  elemTerrain.setAttribute( QStringLiteral( "show-labels" ), mShowLabels ? 1 : 0 );

  QDomElement elemPointLights = doc.createElement( QStringLiteral( "point-lights" ) );
  for ( const QgsPointLightSettings &pointLight : qgis::as_const( mPointLights ) )
  {
    QDomElement elemPointLight = pointLight.writeXml( doc );
    elemPointLights.appendChild( elemPointLight );
  }
  elem.appendChild( elemPointLights );

  QDomElement elemDirectionalLights = doc.createElement( QStringLiteral( "directional-lights" ) );
  for ( const QgsDirectionalLightSettings &directionalLight : qgis::as_const( mDirectionalLights ) )
  {
    QDomElement elemDirectionalLight = directionalLight.writeXml( doc );
    elemDirectionalLights.appendChild( elemDirectionalLight );
  }
  elem.appendChild( elemDirectionalLights );

  QDomElement elemMapLayers = doc.createElement( QStringLiteral( "layers" ) );
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
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
  Q_FOREACH ( const QgsAbstract3DRenderer *renderer, mRenderers )
  {
    QDomElement elemRenderer = doc.createElement( QStringLiteral( "renderer" ) );
    elemRenderer.setAttribute( QStringLiteral( "type" ), renderer->type() );
    renderer->writeXml( elemRenderer, context );
    elemRenderers.appendChild( elemRenderer );
  }
  elem.appendChild( elemRenderers );

  QDomElement elemSkybox = doc.createElement( QStringLiteral( "skybox" ) );
  elemSkybox.setAttribute( QStringLiteral( "enabled" ), mSkyboxEnabled ? 1 : 0 );
  // TODO: use context for relative paths, maybe explicitly list all files(?)
  elemSkybox.setAttribute( QStringLiteral( "file-base" ), mSkyboxFileBase );
  elemSkybox.setAttribute( QStringLiteral( "file-ext" ), mSkyboxFileExtension );
  elem.appendChild( elemSkybox );

  QDomElement elemDebug = doc.createElement( QStringLiteral( "debug" ) );
  elemDebug.setAttribute( QStringLiteral( "bounding-boxes" ), mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "terrain-tile-info" ), mShowTerrainTileInfo ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "camera-view-center" ), mShowCameraViewCenter ? 1 : 0 );
  elemDebug.setAttribute( QStringLiteral( "show-light-sources" ), mShowLightSources ? 1 : 0 );
  elem.appendChild( elemDebug );

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
  Q_FOREACH ( QgsMapLayer *layer, layers )
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
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
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

float Qgs3DMapSettings::maxTerrainGroundError() const
{
  return mMaxTerrainGroundError;
}

void Qgs3DMapSettings::setTerrainGenerator( QgsTerrainGenerator *gen )
{
  mTerrainGenerator.reset( gen );
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

void Qgs3DMapSettings::setSkybox( bool enabled, const QString &fileBase, const QString &fileExtension )
{
  mSkyboxEnabled = enabled;
  mSkyboxFileBase = fileBase;
  mSkyboxFileExtension = fileExtension;
}
