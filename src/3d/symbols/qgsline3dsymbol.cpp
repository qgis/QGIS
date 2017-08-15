#include "qgsline3dsymbol.h"

QgsLine3DSymbol::QgsLine3DSymbol()
  : altClamping( AltClampRelative )
  , altBinding( AltBindCentroid )
  , height( 0 )
  , extrusionHeight( 0 )
  , width( 2 )
{

}

QgsAbstract3DSymbol *QgsLine3DSymbol::clone() const
{
  return new QgsLine3DSymbol( *this );
}

void QgsLine3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
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

void QgsLine3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
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
