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
  material.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QVariantMap shapePropertiesCopy( shapeProperties );
  shapePropertiesCopy["model"] = QVariant( context.pathResolver().writePath( shapePropertiesCopy["model"].toString() ) );

  QDomElement elemShapeProperties = doc.createElement( "shape-properties" );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( "transform" );
  elemTransform.setAttribute( "matrix", Utils::matrix4x4toString( transform ) );
  elem.appendChild( elemTransform );
}

void QgsPoint3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement elemMaterial = elem.firstChildElement( "material" );
  material.readXml( elemMaterial );

  QDomElement elemShapeProperties = elem.firstChildElement( "shape-properties" );
  shapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  shapeProperties["model"] = QVariant( context.pathResolver().readPath( shapeProperties["model"].toString() ) );

  QDomElement elemTransform = elem.firstChildElement( "transform" );
  transform = Utils::stringToMatrix4x4( elemTransform.attribute( "matrix" ) );
}
