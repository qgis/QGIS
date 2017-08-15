#include "qgspoint3dsymbol.h"

#include "qgsreadwritecontext.h"
#include "qgsxmlutils.h"


QgsPoint3DSymbol::QgsPoint3DSymbol()
{
}

QgsAbstract3DSymbol *QgsPoint3DSymbol::clone() const
{
  return new QgsPoint3DSymbol( *this );
}

void QgsPoint3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemMaterial = doc.createElement( "material" );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QVariantMap shapePropertiesCopy( mShapeProperties );
  shapePropertiesCopy["model"] = QVariant( context.pathResolver().writePath( shapePropertiesCopy["model"].toString() ) );

  QDomElement elemShapeProperties = doc.createElement( "shape-properties" );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( "transform" );
  elemTransform.setAttribute( "matrix", Qgs3DUtils::matrix4x4toString( mTransform ) );
  elem.appendChild( elemTransform );
}

void QgsPoint3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement elemMaterial = elem.firstChildElement( "material" );
  mMaterial.readXml( elemMaterial );

  QDomElement elemShapeProperties = elem.firstChildElement( "shape-properties" );
  mShapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  mShapeProperties["model"] = QVariant( context.pathResolver().readPath( mShapeProperties["model"].toString() ) );

  QDomElement elemTransform = elem.firstChildElement( "transform" );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( "matrix" ) );
}
