/***************************************************************************
                         qgsterrainprovider.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsterrainprovider.h"

QgsAbstractTerrainProvider::~QgsAbstractTerrainProvider() = default;

void QgsAbstractTerrainProvider::resolveReferences( const QgsProject * )
{

}

//
// QgsFlatTerrainProvider
//

QString QgsFlatTerrainProvider::type() const
{
  return QStringLiteral( "flat" );
}

bool QgsFlatTerrainProvider::readXml( const QDomElement &, const QgsReadWriteContext & )
{
  return true;
}

QDomElement QgsFlatTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  return element;
}

QgsCoordinateReferenceSystem QgsFlatTerrainProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

double QgsFlatTerrainProvider::heightAt( double, double ) const
{
  return 0;
}

QgsFlatTerrainProvider *QgsFlatTerrainProvider::clone() const
{
  return new QgsFlatTerrainProvider( *this );
}


//
//  QgsRasterDemTerrainProvider
//

QString QgsRasterDemTerrainProvider::type() const
{
  return QStringLiteral( "raster" );
}

void QgsRasterDemTerrainProvider::resolveReferences( const QgsProject *project )
{
  if ( mRasterLayer )
    return;  // already assigned

  mRasterLayer.resolve( project );
}

bool QgsRasterDemTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement terrainElement = element.firstChildElement( QStringLiteral( "TerrainProvider" ) );

  QString layerId = terrainElement.attribute( QStringLiteral( "layer" ) );
  QString layerName = terrainElement.attribute( QStringLiteral( "layerName" ) );
  QString layerSource = terrainElement.attribute( QStringLiteral( "layerSource" ) );
  QString layerProvider = terrainElement.attribute( QStringLiteral( "layerProvider" ) );
  mRasterLayer = _LayerRef<QgsRasterLayer>( layerId, layerName, layerSource, layerProvider );

  return true;
}

QDomElement QgsRasterDemTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  if ( mRasterLayer )
  {
    element.setAttribute( QStringLiteral( "layer" ), mRasterLayer.layerId );
    element.setAttribute( QStringLiteral( "layerName" ), mRasterLayer.name );
    element.setAttribute( QStringLiteral( "layerSource" ), mRasterLayer.source );
    element.setAttribute( QStringLiteral( "layerProvider" ), mRasterLayer.provider );
  }
  return element;
}

QgsCoordinateReferenceSystem QgsRasterDemTerrainProvider::crs() const
{
  return mRasterLayer ? mRasterLayer->crs() : QgsCoordinateReferenceSystem();
}

double QgsRasterDemTerrainProvider::heightAt( double x, double y ) const
{
  // TODO -- may want to use a more efficient approach here, i.e. requesting whole
  // blocks upfront instead of multiple sample calls
  if ( mRasterLayer->isValid() )
    return mRasterLayer->dataProvider()->sample( QgsPointXY( x, y ), 1 );
  else
    return 0;
}

QgsRasterDemTerrainProvider *QgsRasterDemTerrainProvider::clone() const
{
  return new QgsRasterDemTerrainProvider( *this );
}

void QgsRasterDemTerrainProvider::setLayer( QgsRasterLayer *layer )
{
  mRasterLayer.setLayer( layer );
}

QgsRasterLayer *QgsRasterDemTerrainProvider::layer() const
{
  return mRasterLayer.get();
}


//
// QgsMeshTerrainProvider
//

QString QgsMeshTerrainProvider::type() const
{
  return QStringLiteral( "mesh" );
}

void QgsMeshTerrainProvider::resolveReferences( const QgsProject *project )
{
  if ( mMeshLayer )
    return;  // already assigned

  mMeshLayer.resolve( project );
}

bool QgsMeshTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement terrainElement = element.firstChildElement( QStringLiteral( "TerrainProvider" ) );

  QString layerId = terrainElement.attribute( QStringLiteral( "layer" ) );
  QString layerName = terrainElement.attribute( QStringLiteral( "layerName" ) );
  QString layerSource = terrainElement.attribute( QStringLiteral( "layerSource" ) );
  QString layerProvider = terrainElement.attribute( QStringLiteral( "layerProvider" ) );
  mMeshLayer = _LayerRef<QgsMeshLayer>( layerId, layerName, layerSource, layerProvider );

  return true;
}

QDomElement QgsMeshTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  if ( mMeshLayer )
  {
    element.setAttribute( QStringLiteral( "layer" ), mMeshLayer.layerId );
    element.setAttribute( QStringLiteral( "layerName" ), mMeshLayer.name );
    element.setAttribute( QStringLiteral( "layerSource" ), mMeshLayer.source );
    element.setAttribute( QStringLiteral( "layerProvider" ), mMeshLayer.provider );
  }
  return element;
}

QgsCoordinateReferenceSystem QgsMeshTerrainProvider::crs() const
{
  return mMeshLayer ? mMeshLayer->crs() : QgsCoordinateReferenceSystem();
}

double QgsMeshTerrainProvider::heightAt( double x, double y ) const
{
  // TODO
  Q_UNUSED( x )
  Q_UNUSED( y )
  return 0;
}

QgsMeshTerrainProvider *QgsMeshTerrainProvider::clone() const
{
  return new QgsMeshTerrainProvider( *this );
}

void QgsMeshTerrainProvider::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer.setLayer( layer );
}

QgsMeshLayer *QgsMeshTerrainProvider::layer() const
{
  return mMeshLayer.get();
}
