#include "abstract3drenderer.h"

#include "lineentity.h"
#include "pointentity.h"
#include "polygonentity.h"

#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"


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

Abstract3DRenderer *PolygonRenderer::clone() const
{
  return new PolygonRenderer( *this );
}

Qt3DCore::QEntity *PolygonRenderer::createEntity( const Map3D &map ) const
{
  return new PolygonEntity( map, *this );
}

void PolygonRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "layer", layerRef.layerId );
  elemDataProperties.setAttribute( "alt-clamping", Utils::altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Utils::altBindingToString( altBinding ) );
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
  altClamping = Utils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = Utils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
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

Abstract3DRenderer *PointRenderer::clone() const
{
  return new PointRenderer( *this );
}

Qt3DCore::QEntity *PointRenderer::createEntity( const Map3D &map ) const
{
  return new PointEntity( map, *this );
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
  elemTransform.setAttribute( "matrix", Utils::matrix4x4toString( transform ) );
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
  transform = Utils::stringToMatrix4x4( elemTransform.attribute( "matrix" ) );
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

Abstract3DRenderer *LineRenderer::clone() const
{
  return new LineRenderer( *this );
}

Qt3DCore::QEntity *LineRenderer::createEntity( const Map3D &map ) const
{
  return new LineEntity( map, *this );
}

void LineRenderer::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "layer", layerRef.layerId );
  elemDataProperties.setAttribute( "alt-clamping", Utils::altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Utils::altBindingToString( altBinding ) );
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
  altClamping = Utils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = Utils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
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
