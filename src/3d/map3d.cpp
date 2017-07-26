#include "map3d.h"

#include "flatterraingenerator.h"
#include "demterraingenerator.h"
//#include "quantizedmeshterraingenerator.h"
#include "vectorlayer3drenderer.h"

#include <QDomDocument>
#include <QDomElement>

#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"


Map3D::Map3D()
  : originX( 0 )
  , originY( 0 )
  , originZ( 0 )
  , backgroundColor( Qt::black )
  , zExaggeration( 1 )
  , tileTextureSize( 512 )
  , maxTerrainError( 3.f )
  , skybox( false )
  , mShowTerrainBoundingBoxes( false )
  , mShowTerrainTileInfo( false )
{
}

Map3D::Map3D( const Map3D &other )
  : QObject()
  , originX( other.originX )
  , originY( other.originY )
  , originZ( other.originZ )
  , crs( other.crs )
  , backgroundColor( other.backgroundColor )
  , zExaggeration( other.zExaggeration )
  , tileTextureSize( other.tileTextureSize )
  , maxTerrainError( other.maxTerrainError )
  , skybox( other.skybox )
  , skyboxFileBase( other.skyboxFileBase )
  , skyboxFileExtension( other.skyboxFileExtension )
  , mTerrainGenerator( other.mTerrainGenerator ? other.mTerrainGenerator->clone() : nullptr )
  , mShowTerrainBoundingBoxes( other.mShowTerrainBoundingBoxes )
  , mShowTerrainTileInfo( other.mShowTerrainTileInfo )
  , mLayers( other.mLayers )
{
  Q_FOREACH ( QgsAbstract3DRenderer *renderer, other.renderers )
  {
    renderers << renderer->clone();
  }
}

Map3D::~Map3D()
{
  qDeleteAll( renderers );
}

void Map3D::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemOrigin = elem.firstChildElement( "origin" );
  originX = elemOrigin.attribute( "x" ).toDouble();
  originY = elemOrigin.attribute( "y" ).toDouble();
  originZ = elemOrigin.attribute( "z" ).toDouble();

  QDomElement elemCrs = elem.firstChildElement( "crs" );
  crs.readXml( elemCrs );

  QDomElement elemTerrain = elem.firstChildElement( "terrain" );
  zExaggeration = elemTerrain.attribute( "exaggeration", "1" ).toFloat();
  tileTextureSize = elemTerrain.attribute( "texture-size", "512" ).toInt();
  maxTerrainError = elemTerrain.attribute( "max-terrain-error", "3" ).toFloat();
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
    mTerrainGenerator.reset( new DemTerrainGenerator );
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
    FlatTerrainGenerator *flatGen = new FlatTerrainGenerator;
    flatGen->setCrs( crs );
    mTerrainGenerator.reset( flatGen );
  }
  mTerrainGenerator->readXml( elemTerrainGenerator );

  qDeleteAll( renderers );
  renderers.clear();;

  QDomElement elemRenderers = elem.firstChildElement( "renderers" );
  QDomElement elemRenderer = elemRenderers.firstChildElement( "renderer" );
  while ( !elemRenderer.isNull() )
  {
    QgsAbstract3DRenderer *renderer = nullptr;
    QString type = elemRenderer.attribute( "type" );
    if ( type == "vector" )
    {
      renderer = new VectorLayer3DRenderer;
    }

    if ( renderer )
    {
      renderer->readXml( elemRenderer );
      renderers.append( renderer );
    }
    elemRenderer = elemRenderer.nextSiblingElement( "renderer" );
  }

  QDomElement elemSkybox = elem.firstChildElement( "skybox" );
  skybox = elemSkybox.attribute( "enabled", "0" ).toInt();
  skyboxFileBase = elemSkybox.attribute( "file-base" );
  skyboxFileExtension = elemSkybox.attribute( "file-ext" );

  QDomElement elemDebug = elem.firstChildElement( "debug" );
  mShowTerrainBoundingBoxes = elemDebug.attribute( "bounding-boxes", "0" ).toInt();
  mShowTerrainTileInfo = elemDebug.attribute( "terrain-tile-info", "0" ).toInt();
}

QDomElement Map3D::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  QDomElement elem = doc.createElement( "qgis3d" );

  QDomElement elemOrigin = doc.createElement( "origin" );
  elemOrigin.setAttribute( "x", QString::number( originX ) );
  elemOrigin.setAttribute( "y", QString::number( originY ) );
  elemOrigin.setAttribute( "z", QString::number( originZ ) );
  elem.appendChild( elemOrigin );

  QDomElement elemCrs = doc.createElement( "crs" );
  crs.writeXml( elemCrs, doc );
  elem.appendChild( elemCrs );

  QDomElement elemTerrain = doc.createElement( "terrain" );
  elemTerrain.setAttribute( "exaggeration", QString::number( zExaggeration ) );
  elemTerrain.setAttribute( "texture-size", tileTextureSize );
  elemTerrain.setAttribute( "max-terrain-error", QString::number( maxTerrainError ) );
  QDomElement elemMapLayers = doc.createElement( "layers" );
  Q_FOREACH ( const QgsMapLayerRef &layerRef, mLayers )
  {
    QDomElement elemMapLayer = doc.createElement( "layer" );
    elemMapLayer.setAttribute( "id", layerRef.layerId );
    elemMapLayers.appendChild( elemMapLayer );
  }
  elemTerrain.appendChild( elemMapLayers );
  QDomElement elemTerrainGenerator = doc.createElement( "generator" );
  elemTerrainGenerator.setAttribute( "type", TerrainGenerator::typeToString( mTerrainGenerator->type() ) );
  mTerrainGenerator->writeXml( elemTerrainGenerator );
  elemTerrain.appendChild( elemTerrainGenerator );
  elem.appendChild( elemTerrain );

  QDomElement elemRenderers = doc.createElement( "renderers" );
  Q_FOREACH ( const QgsAbstract3DRenderer *renderer, renderers )
  {
    QDomElement elemRenderer = doc.createElement( "renderer" );
    elemRenderer.setAttribute( "type", renderer->type() );
    renderer->writeXml( elemRenderer );
    elemRenderers.appendChild( elemRenderer );
  }
  elem.appendChild( elemRenderers );

  QDomElement elemSkybox = doc.createElement( "skybox" );
  elemSkybox.setAttribute( "enabled", skybox ? 1 : 0 );
  // TODO: use context for relative paths, maybe explicitly list all files(?)
  elemSkybox.setAttribute( "file-base", skyboxFileBase );
  elemSkybox.setAttribute( "file-ext", skyboxFileExtension );
  elem.appendChild( elemSkybox );

  QDomElement elemDebug = doc.createElement( "debug" );
  elemDebug.setAttribute( "bounding-boxes", mShowTerrainBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( "terrain-tile-info", mShowTerrainTileInfo ? 1 : 0 );
  elem.appendChild( elemDebug );

  return elem;
}

void Map3D::resolveReferences( const QgsProject &project )
{
  for ( int i = 0; i < mLayers.count(); ++i )
  {
    QgsMapLayerRef &layerRef = mLayers[i];
    layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
  }

  mTerrainGenerator->resolveReferences( project );

  for ( int i = 0; i < renderers.count(); ++i )
  {
    QgsAbstract3DRenderer *renderer = renderers[i];
    renderer->resolveReferences( project );
  }
}

void Map3D::setLayers( const QList<QgsMapLayer *> &layers )
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

QList<QgsMapLayer *> Map3D::layers() const
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

void Map3D::setTerrainGenerator( TerrainGenerator *gen )
{
  mTerrainGenerator.reset( gen );
  emit terrainGeneratorChanged();
}

void Map3D::setShowTerrainBoundingBoxes( bool enabled )
{
  if ( mShowTerrainBoundingBoxes == enabled )
    return;

  mShowTerrainBoundingBoxes = enabled;
  emit showTerrainBoundingBoxesChanged();
}

void Map3D::setShowTerrainTilesInfo( bool enabled )
{
  if ( mShowTerrainTileInfo == enabled )
    return;

  mShowTerrainTileInfo = enabled;
  emit showTerrainTilesInfoChanged();
}
