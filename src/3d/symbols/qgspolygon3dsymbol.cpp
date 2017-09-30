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

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-binding" ), Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "extrusion-height" ), mExtrusionHeight );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( QStringLiteral( "alt-binding" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( QStringLiteral( "extrusion-height" ) ).toFloat();

  QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterial.readXml( elemMaterial );
}
