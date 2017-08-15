#include "qgspolygon3dsymbol.h"

QgsPolygon3DSymbol::QgsPolygon3DSymbol()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
{
}

QgsAbstract3DSymbol *QgsPolygon3DSymbol::clone() const
{
  return new QgsPolygon3DSymbol( *this );
}

void QgsPolygon3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
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

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
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
