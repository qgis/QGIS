#include "qgsline3dsymbol.h"

QgsLine3DSymbol::QgsLine3DSymbol()
  : mAltClamping( AltClampRelative )
  , mAltBinding( AltBindCentroid )
  , mWidth( 2 )
  , mHeight( 0 )
  , mExtrusionHeight( 0 )
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
  elemDataProperties.setAttribute( "alt-clamping", Utils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Utils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( "height", mHeight );
  elemDataProperties.setAttribute( "extrusion-height", mExtrusionHeight );
  elemDataProperties.setAttribute( "width", mWidth );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void QgsLine3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  mAltClamping = Utils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  mAltBinding = Utils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  mHeight = elemDataProperties.attribute( "height" ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();
  mWidth = elemDataProperties.attribute( "width" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  mMaterial.readXml( elemMaterial );
}
