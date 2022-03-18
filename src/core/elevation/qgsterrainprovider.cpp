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

QgsAbstractTerrainProvider::QgsAbstractTerrainProvider( const QgsAbstractTerrainProvider &other )
  : mScale( other.mScale )
  , mOffset( other.mOffset )
{

}

void QgsAbstractTerrainProvider::writeCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "offset" ), qgsDoubleToString( mOffset ) );
  element.setAttribute( QStringLiteral( "scale" ), qgsDoubleToString( mScale ) );
}

void QgsAbstractTerrainProvider::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mOffset = element.attribute( QStringLiteral( "offset" ) ).toDouble();
  mScale = element.attribute( QStringLiteral( "scale" ) ).toDouble();
}

//
// QgsFlatTerrainProvider
//

QString QgsFlatTerrainProvider::type() const
{
  return QStringLiteral( "flat" );
}

bool QgsFlatTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( QStringLiteral( "TerrainProvider" ) );
  if ( terrainElement.isNull() )
    return false;

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsFlatTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  writeCommonProperties( element, context );
  return element;
}

QgsCoordinateReferenceSystem QgsFlatTerrainProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

double QgsFlatTerrainProvider::heightAt( double, double ) const
{
  return mOffset;
}

QgsFlatTerrainProvider *QgsFlatTerrainProvider::clone() const
{
  return new QgsFlatTerrainProvider( *this );
}

bool QgsFlatTerrainProvider::equals( const QgsAbstractTerrainProvider *other ) const
{
  if ( other->type() != type() )
    return false;

  const QgsFlatTerrainProvider *otherTerrain = qgis::down_cast< const QgsFlatTerrainProvider * >( other );

  return qgsDoubleNear( otherTerrain->offset(), mOffset );
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

bool QgsRasterDemTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( QStringLiteral( "TerrainProvider" ) );
  if ( terrainElement.isNull() )
    return false;

  QString layerId = terrainElement.attribute( QStringLiteral( "layer" ) );
  QString layerName = terrainElement.attribute( QStringLiteral( "layerName" ) );
  QString layerSource = terrainElement.attribute( QStringLiteral( "layerSource" ) );
  QString layerProvider = terrainElement.attribute( QStringLiteral( "layerProvider" ) );
  mRasterLayer = _LayerRef<QgsRasterLayer>( layerId, layerName, layerSource, layerProvider );

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsRasterDemTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  if ( mRasterLayer )
  {
    element.setAttribute( QStringLiteral( "layer" ), mRasterLayer.layerId );
    element.setAttribute( QStringLiteral( "layerName" ), mRasterLayer.name );
    element.setAttribute( QStringLiteral( "layerSource" ), mRasterLayer.source );
    element.setAttribute( QStringLiteral( "layerProvider" ), mRasterLayer.provider );
  }

  writeCommonProperties( element, context );
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
  if ( mRasterLayer && mRasterLayer->isValid() )
  {
    bool ok = false;
    const double res = mRasterLayer->dataProvider()->sample( QgsPointXY( x, y ), 1, &ok );

    if ( ok )
      return res * mScale + mOffset;
  }

  return std::numeric_limits<double>::quiet_NaN();
}

QgsRasterDemTerrainProvider *QgsRasterDemTerrainProvider::clone() const
{
  return new QgsRasterDemTerrainProvider( *this );
}

bool QgsRasterDemTerrainProvider::equals( const QgsAbstractTerrainProvider *other ) const
{
  if ( other->type() != type() )
    return false;

  const QgsRasterDemTerrainProvider *otherTerrain = qgis::down_cast< const QgsRasterDemTerrainProvider * >( other );
  if ( !qgsDoubleNear( otherTerrain->offset(), mOffset )
       || !qgsDoubleNear( otherTerrain->scale(), mScale )
       || mRasterLayer.get() != otherTerrain->layer() )
    return false;

  return true;
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

bool QgsMeshTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( QStringLiteral( "TerrainProvider" ) );
  if ( terrainElement.isNull() )
    return false;

  QString layerId = terrainElement.attribute( QStringLiteral( "layer" ) );
  QString layerName = terrainElement.attribute( QStringLiteral( "layerName" ) );
  QString layerSource = terrainElement.attribute( QStringLiteral( "layerSource" ) );
  QString layerProvider = terrainElement.attribute( QStringLiteral( "layerProvider" ) );
  mMeshLayer = _LayerRef<QgsMeshLayer>( layerId, layerName, layerSource, layerProvider );

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsMeshTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "TerrainProvider" ) );
  if ( mMeshLayer )
  {
    element.setAttribute( QStringLiteral( "layer" ), mMeshLayer.layerId );
    element.setAttribute( QStringLiteral( "layerName" ), mMeshLayer.name );
    element.setAttribute( QStringLiteral( "layerSource" ), mMeshLayer.source );
    element.setAttribute( QStringLiteral( "layerProvider" ), mMeshLayer.provider );
  }

  writeCommonProperties( element, context );
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
  return std::numeric_limits<double>::quiet_NaN();
}

QgsMeshTerrainProvider *QgsMeshTerrainProvider::clone() const
{
  return new QgsMeshTerrainProvider( *this );
}

bool QgsMeshTerrainProvider::equals( const QgsAbstractTerrainProvider *other ) const
{
  if ( other->type() != type() )
    return false;

  const QgsMeshTerrainProvider *otherTerrain = qgis::down_cast< const QgsMeshTerrainProvider * >( other );
  if ( !qgsDoubleNear( otherTerrain->offset(), mOffset )
       || !qgsDoubleNear( otherTerrain->scale(), mScale )
       || mMeshLayer.get() != otherTerrain->layer() )
    return false;

  return true;
}

void QgsMeshTerrainProvider::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer.setLayer( layer );
}

QgsMeshLayer *QgsMeshTerrainProvider::layer() const
{
  return mMeshLayer.get();
}
