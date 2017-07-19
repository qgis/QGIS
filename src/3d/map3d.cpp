#include "map3d.h"

#include "flatterraingenerator.h"
#include "demterraingenerator.h"
//#include "quantizedmeshterraingenerator.h"

#include <QDomDocument>
#include <QDomElement>

#include "qgssymbollayerutils.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


static QString _matrix4x4toString( const QMatrix4x4 &m )
{
  const float *d = m.constData();
  QStringList elems;
  for ( int i = 0; i < 16; ++i )
    elems << QString::number( d[i] );
  return elems.join( ' ' );
}

static QMatrix4x4 _stringToMatrix4x4( const QString &str )
{
  QMatrix4x4 m;
  float *d = m.data();
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < 16; ++i )
    d[i] = elems[i].toFloat();
  return m;
}

QString altClampingToString( AltitudeClamping altClamp )
{
  switch ( altClamp )
  {
    case AltClampAbsolute: return QStringLiteral( "absolute" );
    case AltClampRelative: return QStringLiteral( "relative" );
    case AltClampTerrain: return QStringLiteral( "terrain" );
    default: Q_ASSERT( false ); return QString();
  }
}

AltitudeClamping altClampingFromString( const QString &str )
{
  if ( str == "absolute" )
    return AltClampAbsolute;
  else if ( str == "terrain" )
    return AltClampTerrain;
  else   // "relative"  (default)
    return AltClampRelative;
}

QString altBindingToString( AltitudeBinding altBind )
{
  switch ( altBind )
  {
    case AltBindVertex: return QStringLiteral( "vertex" );
    case AltBindCentroid: return QStringLiteral( "centroid" );
    default: Q_ASSERT( false ); return QString();
  }
}

AltitudeBinding altBindingFromString( const QString &str )
{
  if ( str == "vertex" )
    return AltBindVertex;
  else  // "centroid"  (default)
    return AltBindCentroid;
}


Map3D::Map3D()
  : originX( 0 )
  , originY( 0 )
  , originZ( 0 )
  , backgroundColor( Qt::black )
  , zExaggeration( 1 )
  , tileTextureSize( 512 )
  , maxTerrainError( 3.f )
  , skybox( false )
  , showBoundingBoxes( false )
  , drawTerrainTileInfo( false )
{
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
    terrainGenerator.reset( new DemTerrainGenerator );
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
    terrainGenerator.reset( flatGen );
  }
  terrainGenerator->readXml( elemTerrainGenerator );

  polygonRenderers.clear();
  pointRenderers.clear();

  QDomElement elemRenderers = elem.firstChildElement( "renderers" );
  QDomElement elemRenderer = elemRenderers.firstChildElement( "renderer" );
  while ( !elemRenderer.isNull() )
  {
    QString type = elemRenderer.attribute( "type" );
    if ( type == "polygon" )
    {
      PolygonRenderer r;
      r.readXml( elemRenderer );
      polygonRenderers.append( r );
    }
    else if ( type == "point" )
    {
      PointRenderer r;
      r.readXml( elemRenderer );
      pointRenderers.append( r );
    }
    else if ( type == "line" )
    {
      LineRenderer r;
      r.readXml( elemRenderer );
      lineRenderers.append( r );
    }
    elemRenderer = elemRenderer.nextSiblingElement( "renderer" );
  }

  QDomElement elemSkybox = elem.firstChildElement( "skybox" );
  skybox = elemSkybox.attribute( "enabled", "0" ).toInt();
  skyboxFileBase = elemSkybox.attribute( "file-base" );
  skyboxFileExtension = elemSkybox.attribute( "file-ext" );

  QDomElement elemDebug = elem.firstChildElement( "debug" );
  showBoundingBoxes = elemDebug.attribute( "bounding-boxes", "0" ).toInt();
  drawTerrainTileInfo = elemDebug.attribute( "terrain-tile-info", "0" ).toInt();
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
  elemTerrainGenerator.setAttribute( "type", TerrainGenerator::typeToString( terrainGenerator->type() ) );
  terrainGenerator->writeXml( elemTerrainGenerator );
  elemTerrain.appendChild( elemTerrainGenerator );
  elem.appendChild( elemTerrain );

  QDomElement elemRenderers = doc.createElement( "renderers" );
  Q_FOREACH ( const PointRenderer &r, pointRenderers )
  {
    QDomElement elemRenderer = doc.createElement( "renderer" );
    elemRenderer.setAttribute( "type", "point" );
    r.writeXml( elemRenderer );
    elemRenderers.appendChild( elemRenderer );
  }
  Q_FOREACH ( const PolygonRenderer &r, polygonRenderers )
  {
    QDomElement elemRenderer = doc.createElement( "renderer" );
    elemRenderer.setAttribute( "type", "polygon" );
    r.writeXml( elemRenderer );
    elemRenderers.appendChild( elemRenderer );
  }
  Q_FOREACH ( const LineRenderer &r, lineRenderers )
  {
    QDomElement elemRenderer = doc.createElement( "renderer" );
    elemRenderer.setAttribute( "type", "line" );
    r.writeXml( elemRenderer );
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
  elemDebug.setAttribute( "bounding-boxes", showBoundingBoxes ? 1 : 0 );
  elemDebug.setAttribute( "terrain-tile-info", drawTerrainTileInfo ? 1 : 0 );
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

  terrainGenerator->resolveReferences( project );

  for ( int i = 0; i < polygonRenderers.count(); ++i )
  {
    PolygonRenderer &r = polygonRenderers[i];
    r.resolveReferences( project );
  }

  for ( int i = 0; i < pointRenderers.count(); ++i )
  {
    PointRenderer &r = pointRenderers[i];
    r.resolveReferences( project );
  }

  for ( int i = 0; i < lineRenderers.count(); ++i )
  {
    LineRenderer &r = lineRenderers[i];
    r.resolveReferences( project );
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
  mLayers = lst;
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

// ---------------

PolygonRenderer::PolygonRenderer()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
{
}

void PolygonRenderer::setLayer( QgsVectorLayer *layer )
{
  layerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *PolygonRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( layerRef.layer );
}

void PolygonRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "layer", layerRef.layerId );
  elemDataProperties.setAttribute( "alt-clamping", altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", altBindingToString( altBinding ) );
  elemDataProperties.setAttribute( "height", height );
  elemDataProperties.setAttribute( "extrusion-height", extrusionHeight );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void PolygonRenderer::readXml( const QDomElement &elem )
{
  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  layerRef = QgsMapLayerRef( elemDataProperties.attribute( "layer" ) );
  altClamping = altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  height = elemDataProperties.attribute( "height" ).toFloat();
  extrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );
}

void PolygonRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}

// ---------------

PointRenderer::PointRenderer()
  : height( 0 )
{
}

void PointRenderer::setLayer( QgsVectorLayer *layer )
{
  layerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *PointRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( layerRef.layer );
}

void PointRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "layer", layerRef.layerId );
  elemDataProperties.setAttribute( "height", height );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QDomElement elemShapeProperties = doc.createElement( "shape-properties" );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapeProperties, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( "transform" );
  elemTransform.setAttribute( "matrix", _matrix4x4toString( transform ) );
  elem.appendChild( elemTransform );
}

void PointRenderer::readXml( const QDomElement &elem )
{
  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  layerRef = QgsMapLayerRef( elemDataProperties.attribute( "layer" ) );
  height = elemDataProperties.attribute( "height" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );

  QDomElement elemShapeProperties = elem.firstChildElement( "shape-properties" );
  shapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();

  QDomElement elemTransform = elem.firstChildElement( "transform" );
  transform = _stringToMatrix4x4( elemTransform.attribute( "matrix" ) );
}

void PointRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}

// ---------------

LineRenderer::LineRenderer()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
  , distance( 1 )
{

}

void LineRenderer::setLayer( QgsVectorLayer *layer )
{
  layerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *LineRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( layerRef.layer );
}

void LineRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "layer", layerRef.layerId );
  elemDataProperties.setAttribute( "alt-clamping", altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", altBindingToString( altBinding ) );
  elemDataProperties.setAttribute( "height", height );
  elemDataProperties.setAttribute( "extrusion-height", extrusionHeight );
  elemDataProperties.setAttribute( "distance", distance );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void LineRenderer::readXml( const QDomElement &elem )
{
  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  layerRef = QgsMapLayerRef( elemDataProperties.attribute( "layer" ) );
  altClamping = altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  height = elemDataProperties.attribute( "height" ).toFloat();
  extrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();
  distance = elemDataProperties.attribute( "distance" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );
}

void LineRenderer::resolveReferences( const QgsProject &project )
{
  layerRef.setLayer( project.mapLayer( layerRef.layerId ) );
}

// ---------------

void PhongMaterialSettings::readXml( const QDomElement &elem )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( "ambient" ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( "diffuse" ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( "specular" ) );
  mShininess = elem.attribute( "shininess" ).toFloat();
}

void PhongMaterialSettings::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "ambient", QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( "diffuse", QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( "specular", QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( "shininess", mShininess );
}
