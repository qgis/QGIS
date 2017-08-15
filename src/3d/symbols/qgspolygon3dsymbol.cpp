#include "qgspolygon3dsymbol.h"

QgsPolygon3DSymbol::QgsPolygon3DSymbol()
  : mAltClamping( AltClampRelative )
  , mAltBinding( AltBindCentroid )
  , mHeight( 0 )
  , mExtrusionHeight( 0 )
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
  elemDataProperties.setAttribute( "alt-clamping", Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( "alt-binding", Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( "height", mHeight );
  elemDataProperties.setAttribute( "extrusion-height", mExtrusionHeight );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( "material" );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( "data" );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( "alt-clamping" ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( "alt-binding" ) );
  mHeight = elemDataProperties.attribute( "height" ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( "extrusion-height" ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( "material" );
  mMaterial.readXml( elemMaterial );
}
