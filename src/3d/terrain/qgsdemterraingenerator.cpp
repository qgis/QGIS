#include "qgsdemterraingenerator.h"

#include "qgsdemterraintileloader_p.h"

#include "qgsrasterlayer.h"



QgsDemTerrainGenerator::QgsDemTerrainGenerator()
  : mResolution( 16 )
{
}

void QgsDemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  updateGenerator();
}

QgsRasterLayer *QgsDemTerrainGenerator::layer() const
{
  return qobject_cast<QgsRasterLayer *>( mLayer.layer.data() );
}

QgsTerrainGenerator *QgsDemTerrainGenerator::clone() const
{
  QgsDemTerrainGenerator *cloned = new QgsDemTerrainGenerator;
  cloned->mLayer = mLayer;
  cloned->mResolution = mResolution;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Dem;
}

QgsRectangle QgsDemTerrainGenerator::extent() const
{
  return terrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map );
  return mHeightMapGenerator->heightAt( x, y );
}

void QgsDemTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "layer", mLayer.layerId );
  elem.setAttribute( "resolution", mResolution );
}

void QgsDemTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( "layer" ) );
  mResolution = elem.attribute( "resolution" ).toInt();
}

void QgsDemTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  updateGenerator();
}

ChunkLoader *QgsDemTerrainGenerator::createChunkLoader( ChunkNode *node ) const
{
  return new QgsDemTerrainTileLoader( mTerrain, node );
}

void QgsDemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem )
  {
    terrainTilingScheme = QgsTilingScheme( dem->extent(), dem->crs() );
    mHeightMapGenerator.reset( new QgsDemHeightMapGenerator( dem, terrainTilingScheme, mResolution ) );
  }
  else
  {
    terrainTilingScheme = QgsTilingScheme();
    mHeightMapGenerator.reset();
  }
}
