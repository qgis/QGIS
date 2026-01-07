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

#include "qgsmeshlayerutils.h"
#include "qgsrasterbandstats.h"

#include <QThread>

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
  element.setAttribute( u"offset"_s, qgsDoubleToString( mOffset ) );
  element.setAttribute( u"scale"_s, qgsDoubleToString( mScale ) );
}

void QgsAbstractTerrainProvider::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mOffset = element.attribute( u"offset"_s ).toDouble();
  mScale = element.attribute( u"scale"_s ).toDouble();
}

//
// QgsFlatTerrainProvider
//

QString QgsFlatTerrainProvider::type() const
{
  return u"flat"_s;
}

bool QgsFlatTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( u"TerrainProvider"_s );
  if ( terrainElement.isNull() )
    return false;

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsFlatTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( u"TerrainProvider"_s );
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

void QgsFlatTerrainProvider::prepare()
{
  Q_ASSERT_X( QThread::currentThread() == QCoreApplication::instance()->thread(), "QgsFlatTerrainProvider::prepare", "prepare() must be called from the main thread" );

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
  return u"raster"_s;
}

void QgsRasterDemTerrainProvider::resolveReferences( const QgsProject *project )
{
  if ( mRasterLayer )
    return;  // already assigned

  mRasterLayer.resolve( project );
}

bool QgsRasterDemTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( u"TerrainProvider"_s );
  if ( terrainElement.isNull() )
    return false;

  QString layerId = terrainElement.attribute( u"layer"_s );
  QString layerName = terrainElement.attribute( u"layerName"_s );
  QString layerSource = terrainElement.attribute( u"layerSource"_s );
  QString layerProvider = terrainElement.attribute( u"layerProvider"_s );
  mRasterLayer = _LayerRef<QgsRasterLayer>( layerId, layerName, layerSource, layerProvider );

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsRasterDemTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( u"TerrainProvider"_s );
  if ( mRasterLayer )
  {
    element.setAttribute( u"layer"_s, mRasterLayer.layerId );
    element.setAttribute( u"layerName"_s, mRasterLayer.name );
    element.setAttribute( u"layerSource"_s, mRasterLayer.source );
    element.setAttribute( u"layerProvider"_s, mRasterLayer.provider );
  }

  writeCommonProperties( element, context );
  return element;
}

QgsCoordinateReferenceSystem QgsRasterDemTerrainProvider::crs() const
{
  return mRasterProvider ? mRasterProvider->crs()
         : ( mRasterLayer ? mRasterLayer->crs() : QgsCoordinateReferenceSystem() );
}

double QgsRasterDemTerrainProvider::heightAt( double x, double y ) const
{
  // TODO -- may want to use a more efficient approach here, i.e. requesting whole
  // blocks upfront instead of multiple sample calls
  bool ok = false;
  double res = std::numeric_limits<double>::quiet_NaN();
  if ( mRasterProvider )
  {
    res = mRasterProvider->sample( QgsPointXY( x, y ), 1, &ok );
  }
  else if ( QThread::currentThread() == QCoreApplication::instance()->thread() && mRasterLayer && mRasterLayer->isValid() )
  {
    res = mRasterLayer->dataProvider()->sample( QgsPointXY( x, y ), 1, &ok );
  }

  if ( ok )
    return res * mScale + mOffset;

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

void QgsRasterDemTerrainProvider::prepare()
{
  Q_ASSERT_X( QThread::currentThread() == QCoreApplication::instance()->thread(), "QgsRasterDemTerrainProvider::prepare", "prepare() must be called from the main thread" );

  if ( mRasterLayer && mRasterLayer->isValid() )
    mRasterProvider.reset( mRasterLayer->dataProvider()->clone() );
}

void QgsRasterDemTerrainProvider::setLayer( QgsRasterLayer *layer )
{
  mRasterLayer.setLayer( layer );
}

QgsRasterLayer *QgsRasterDemTerrainProvider::layer() const
{
  return mRasterLayer.get();
}

QgsRasterDemTerrainProvider::QgsRasterDemTerrainProvider( const QgsRasterDemTerrainProvider &other )
  : QgsAbstractTerrainProvider( other )
  , mRasterLayer( other.mRasterLayer )
  , mRasterProvider( nullptr )
{
}


//
// QgsMeshTerrainProvider
//

QString QgsMeshTerrainProvider::type() const
{
  return u"mesh"_s;
}

void QgsMeshTerrainProvider::resolveReferences( const QgsProject *project )
{
  if ( mMeshLayer )
    return;  // already assigned

  mMeshLayer.resolve( project );
}

bool QgsMeshTerrainProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement terrainElement = element.firstChildElement( u"TerrainProvider"_s );
  if ( terrainElement.isNull() )
    return false;

  QString layerId = terrainElement.attribute( u"layer"_s );
  QString layerName = terrainElement.attribute( u"layerName"_s );
  QString layerSource = terrainElement.attribute( u"layerSource"_s );
  QString layerProvider = terrainElement.attribute( u"layerProvider"_s );
  mMeshLayer = _LayerRef<QgsMeshLayer>( layerId, layerName, layerSource, layerProvider );

  readCommonProperties( terrainElement, context );
  return true;
}

QDomElement QgsMeshTerrainProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( u"TerrainProvider"_s );
  if ( mMeshLayer )
  {
    element.setAttribute( u"layer"_s, mMeshLayer.layerId );
    element.setAttribute( u"layerName"_s, mMeshLayer.name );
    element.setAttribute( u"layerSource"_s, mMeshLayer.source );
    element.setAttribute( u"layerProvider"_s, mMeshLayer.provider );
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
  if ( mTriangularMesh.vertices().empty() && mMeshLayer && QThread::currentThread() == QCoreApplication::instance()->thread() )
    const_cast< QgsMeshTerrainProvider * >( this )->prepare(); // auto prepare if we are on main thread and haven't already!

  return QgsMeshLayerUtils::interpolateZForPoint( mTriangularMesh, x, y ) * mScale + mOffset;
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

void QgsMeshTerrainProvider::prepare()
{
  Q_ASSERT_X( QThread::currentThread() == QCoreApplication::instance()->thread(), "QgsMeshTerrainProvider::prepare", "prepare() must be called from the main thread" );
  if ( mMeshLayer )
  {
    mMeshLayer->updateTriangularMesh();
    mTriangularMesh = *mMeshLayer->triangularMesh();
  }
}

void QgsMeshTerrainProvider::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer.setLayer( layer );
}

QgsMeshLayer *QgsMeshTerrainProvider::layer() const
{
  return mMeshLayer.get();
}

QgsMeshTerrainProvider::QgsMeshTerrainProvider( const QgsMeshTerrainProvider &other )
  : QgsAbstractTerrainProvider( other )
  , mMeshLayer( other.mMeshLayer )
  , mTriangularMesh( QgsTriangularMesh() )
{

}
