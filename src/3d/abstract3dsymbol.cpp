#include "abstract3dsymbol.h"
#include "qgsreadwritecontext.h"

#include "qgsxmlutils.h"


Polygon3DSymbol::Polygon3DSymbol()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
{
}

Abstract3DSymbol *Polygon3DSymbol::clone() const
{
  return new Polygon3DSymbol( *this );
}

void Polygon3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "alt-clamping", Utils::altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Utils::altBindingToString( altBinding ) );
  elemDataProperties.setAttribute( "height", height );
  elemDataProperties.setAttribute( "extrusion-height", extrusionHeight );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void Polygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  altClamping = Utils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = Utils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  height = elemDataProperties.attribute( "height" ).toFloat();
  extrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );
}


// ---------------

Point3DSymbol::Point3DSymbol()
{
}

Abstract3DSymbol *Point3DSymbol::clone() const
{
  return new Point3DSymbol( *this );
}

void Point3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QVariantMap shapePropertiesCopy(shapeProperties);
  shapePropertiesCopy["model"] = QVariant(context.pathResolver().writePath(shapePropertiesCopy["model"].toString()));

  QDomElement elemShapeProperties = doc.createElement( "shape-properties" );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( "transform" );
  elemTransform.setAttribute( "matrix", Utils::matrix4x4toString( transform ) );
  elem.appendChild( elemTransform );
}

void Point3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );

  QDomElement elemShapeProperties = elem.firstChildElement( "shape-properties" );
  shapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  shapeProperties["model"] = QVariant(context.pathResolver().readPath(shapeProperties["model"].toString()));

  QDomElement elemTransform = elem.firstChildElement( "transform" );
  transform = Utils::stringToMatrix4x4( elemTransform.attribute( "matrix" ) );
}


// ---------------

Line3DSymbol::Line3DSymbol()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
  , width( 2 )
{

}

Abstract3DSymbol *Line3DSymbol::clone() const
{
  return new Line3DSymbol( *this );
}

void Line3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( "data" );
  elemDataProperties.setAttribute( "alt-clamping", Utils::altClampingToString( altClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Utils::altBindingToString( altBinding ) );
  elemDataProperties.setAttribute( "height", height );
  elemDataProperties.setAttribute( "extrusion-height", extrusionHeight );
  elemDataProperties.setAttribute( "width", width );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void Line3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  altClamping = Utils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  altBinding = Utils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  height = elemDataProperties.attribute( "height" ).toFloat();
  extrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();
  width = elemDataProperties.attribute( "width" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );
}
