#include "qgspointcloud3dsymbol.h"

// TODO: For some reason whwn I define function on a .cpp file they don't get included in the qgis_app target

//QgsPointCloud3DSymbol::QgsPointCloud3DSymbol()
//: QgsAbstract3DSymbol()
//{

//}

//QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {  }

//QgsAbstract3DSymbol *QgsPointCloud3DSymbol::clone() const
//{
//  // TODO: fix memory leak
//  QgsPointCloud3DSymbol *result = new QgsPointCloud3DSymbol;
//  result->mEnabled = mEnabled;
//  result->mPointSize = mPointSize;
//  copyBaseSettings( result );
//  return result;
//}

//void QgsPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
//{
//  Q_UNUSED( context )

//  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled );
//  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
//}

//void QgsPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
//{
//  Q_UNUSED( context )

//  mEnabled = elem.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
//  mPointSize = elem.attribute( "point-size", QStringLiteral( "5.0" ) ).toFloat();
//}

//void QgsPointCloud3DSymbol::setIsEnabled( bool enabled )
//{
//  mEnabled = enabled;
//}

//void QgsPointCloud3DSymbol::setPointSize( float size )
//{
//  mPointSize = size * 1.0f;
//}
