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

#include "qgsflatterraingenerator.h"
#include "qgsdemterraingenerator.h"
//#include "quantizedmeshterraingenerator.h"
#include "qgsvectorlayer3drenderer.h"

#include <QDomDocument>
#include <QDomElement>

#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"

Qgs3DMapSettings::Qgs3DMapSettings( const Qgs3DMapSettings &other )
  : QObject( nullptr )
  , mOrigin( other.mOrigin )
  , mCrs( other.mCrs )
  , mBackgroundColor( other.mBackgroundColor )
  , mTerrainVerticalScale( other.mTerrainVerticalScale )
  , mTerrainGenerator( other.mTerrainGenerator ? other.mTerrainGenerator->clone() : nullptr )
  , mMapTileResolution( other.mMapTileResolution )
  , mMaxTerrainScreenError( other.mMaxTerrainScreenError )
  , mMaxTerrainGroundError( other.mMaxTerrainGroundError )
  , mShowTerrainBoundingBoxes( other.mShowTerrainBoundingBoxes )
  , mShowTerrainTileInfo( other.mShowTerrainTileInfo )
  , mShowCameraViewCenter( other.mShowCameraViewCenter )
  , mLayers( other.mLayers )
  , mSkyboxEnabled( other.mSkyboxEnabled )
  , mSkyboxFileBase( other.mSkyboxFileBase )
  , mSkyboxFileExtension( other.mSkyboxFileExtension )
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
  QDomElement elemOrigin = elem.firstChildElement( "origin" );
  mOrigin = QgsVector3D(
              elemOrigin.attribute( "x" ).toDouble(),
              elemOrigin.attribute( "y" ).toDouble(),
              elemOrigin.attribute( "z" ).toDouble() );

  QDomElement elemColor = elem.firstChildElement( "color" );
  if ( !elemColor.isNull() )
  {
    mBackgroundColor = QgsSymbolLayerUtils::decodeColor( elemColor.attribute( "background" ) );
    mSelectionColor = QgsSymbolLayerUtils::decodeColor( elemColor.attribute( "selection" ) );
  }

  QDomElement elemCrs = elem.firstChildElement( "crs" );
  mCrs.readXml( elemCrs );

  QDomElement elemTerrain = elem.firstChildElement( "terrain" );
  mTerrainVerticalScale = elemTerrain.attribute( "exaggeration", "1" ).toFloat();
  mMapTileResolution = elemTerrain.attribute( "texture-size", "512" ).toInt();
  mMaxTerrainScreenError = elemTerrain.attribute( "max-terrain-error", "3" ).toFloat();
  mMaxTerrainGroundError = elemTerrain.attribute( "max-ground-error", "1" ).toFloat();
  mShowLabels = elemTerrain.attribute( "show-labels", "0" ).toInt();
  QDomElement elemMapLayers = elemTerrain.firstChildElement( "layers" );
  QDomElement elemMapLayer = elemMapLayers.firstChildElement( "layer" );
  QList<QgsMapLayerRef> mapLayers;
  while ( !elemMapLayer.isNull() )
  {
    mapLayers << QgsMapLayerRef( elemMapLayer.attribute( "id" ) );
    elemMapLayer = elemMapLayer.nextSiblingElement( "layer" );
  }
  mLayers = mapLayers;  // needs to resolve refs afterwards
  QDomElement elemTerrainGenerator = elemTerrain.firstChildElement( "generator" );
  QString terrainGenType = elemTerrainGenerator.attribute( "type" );
  if ( terrainGenType == "dem" )
  {
    QgsDemTerrainGenerator *demTerrainGenerator = new QgsDemTerrainGenerator;
    demTerrainGenerator->setCrs( mCrs, mTransformContext );
    mTerrainGenerator.reset( demTerrainGenerator );
  }
  else if ( terrainGenType == "quantized-mesh" )
  {
#if 0
    terrainGenerator.reset( new QuantizedMeshTerrainGenerator );
#endif
    Q_ASSERT( false ); // currently disabled
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

  QDomElement elemRenderers = elem.firstChildElement( "renderers" );
  QDomElement elemRenderer = elemRenderers.firstChildElement( "renderer" );
  while ( !elemRenderer.isNull() )
  {
    QgsAbstract3DRenderer *renderer = nullptr;
    QString type = elemRenderer.attribute( "type" );
    if ( type == "vector" )
    {
      renderer = new QgsVectorLayer3DRenderer;
    }

    if ( renderer )
    {
      renderer->readXml( elemRenderer, context );
      mRenderers.append( renderer );
    }
    elemRenderer = elemRenderer.nextSiblingElement( "renderer" );
  }

  QDomElement elemSkybox = elem.firstChildElement( "skybox" );
  mSkyboxEnabled = elemSkybox.attribute( "enabled", "0" ).toInt();
  mSkyboxFileBase = elemSkybox.attribute( "file-base" );
  mSkyboxFileExtension = elemSkybox.attribute( "file-ext" );

  QDomElement elemDebug = elem.firstChildElement( "debug" );
  mShowTerrainBoundingBoxes = elemDebug.attribute( "bounding-boxes", "0" ).toInt();
  mShowTerrainTileInfo = elemDebug.attribute( "terrain-tile-info", "0" ).toInt();
  mShowCameraViewCenter = elemDebug.attribute( "camera-view-center", "0" ).toInt();
}

QDomElement Qgs3DMapSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( "qgis3d" );

  QDomElement elemOrigin = doc.createElement( "origin" );
  elemOrigin.setAttribute( "x", QString::number( mOrigin.x() ) );
  elemOrigin.setAttribute( "y", QString::number( mOrigin.y() ) );
  elemOrigin.setAttribute( "z", QString::number( mOrigin.z() ) );
  elem.appendChild( elemOrigin );

  QDomElement elemColor = doc.createElement( "color" );
  elemColor.setAttribute( "background", QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );
  elemColor.setAttribute( "selection", QgsSymbolLayerUtils::encodeColor( mSelectionColor ) );
  elem.appendChild( elemColor );

  QDomElement elemCrs = doc.createElement( "crs" );
  mCrs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( "terrain" );
  elemTerrain.setAttribute( "exaggeration", QString::number( mTerrainVerticalScale ) );
  elemTerrain.setAttribute( "texture-size", mMapTileResolution );
  elemTerrain.setAttribute( "max-terrain-error", QString::number( mMaxTerrainScreenError ) );
  elemTerrain.setAttribute( "max-ground-error", QString::number( mMaxTerrainGroundError ) );
  elemTerrain.setAttribute( "show-labels", mShowLabels ? 1 : 0 );
  QDomElement elemMapLayers = doc.createElement( "layers" );
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
  {
    QDomElement elemMapLayer = doc.createElement( "layer" );
    elemMapLayer.setAttribute( "id", layerRef.layerId );
    elemMapLayers.appendChild( elemMapLayer );
  }
  elemTerrain.appendChild( elemMapLayers );
  QDomElement elemTerrainGenerator = doc.createElement( "generator" );
  elemTerrainGenerator.setAttribute( "type", QgsTerrainGenerator::typeToString( mTerrainGenerator->type() ) );
  mTerrainGenerator->writeXml( elemTerrainGenerator );
  elemTerrain.appendChild( elemTerrainGenerator );
  elem.appendChild( elemTerrain );

  QDomElement elemRenderers = doc.createElement( "renderers" );
  Q_FOREACH ( const QgsAbstract3DRenderer *renderer, mRenderers )
  {
    QDomElement elemRenderer = doc.createElement( "renderer" );
    elemRenderer.setAttribute( "type", renderer->type() );
    renderer->writeXml( elemRenderer, context );
    elemRenderers.appendChild( elemRenderer );
  }
  elem.appendChild( elemRenderers );

  QDomElement elemSkybox = doc.createElement( "skybox" );
  elemSkybox.setAttribute( "enabled", mSkyboxEnabled ? 1 : 0 );
  // TODO: use context for relative paths, maybe explicitly list all files(?)
  elemSkybox.setAttribute( "file-base", mSkyboxFileBase );
  elemSkybox.setAttribute( "file-ext", mSkyboxFileExtension );
  elem.appendChild( elemSkybox );

  QDomElement elemDebug = doc.createElement( "debug" );
  elemDebug.setAttribute( "bounding-boxes", mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( "terrain-tile-info", mShowTerrainTileInfo ? 1 : 0 );
  elemDebug.setAttribute( "camera-view-center", mShowCameraViewCenter ? 1 : 0 );
  elem.appendChild( elemDebug );

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

void Qgs3DMapSettings::setRenderers( const QList<QgsAbstract3DRenderer *> &renderers )
{
  mRenderers = renderers;
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

void Qgs3DMapSettings::setShowLabels( bool enabled )
{
  if ( mShowLabels == enabled )
    return;

  mShowLabels = enabled;
  emit showLabelsChanged();
}
