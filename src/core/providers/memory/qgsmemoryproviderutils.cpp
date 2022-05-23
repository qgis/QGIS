/***************************************************************************
                         qgsmemoryproviderutils.cpp
                         --------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmemoryproviderutils.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"
#include <QUrl>

QString memoryLayerFieldType( QVariant::Type type )
{
  switch ( type )
  {
    case QVariant::Int:
      return QStringLiteral( "integer" );

    case QVariant::LongLong:
      return QStringLiteral( "long" );

    case QVariant::Double:
      return QStringLiteral( "double" );

    case QVariant::String:
      return QStringLiteral( "string" );

    case QVariant::Date:
      return QStringLiteral( "date" );

    case QVariant::Time:
      return QStringLiteral( "time" );

    case QVariant::DateTime:
      return QStringLiteral( "datetime" );

    case QVariant::ByteArray:
      return QStringLiteral( "binary" );

    case QVariant::Bool:
      return QStringLiteral( "boolean" );

    case QVariant::Map:
      return QStringLiteral( "map" );

    default:
      break;
  }
  return QStringLiteral( "string" );
}

QgsVectorLayer *QgsMemoryProviderUtils::createMemoryLayer( const QString &name, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs )
{
  QString geomType = QgsWkbTypes::displayString( geometryType );
  if ( geomType.isNull() )
    geomType = QStringLiteral( "none" );

  QStringList parts;
  if ( crs.isValid() )
  {
    if ( !crs.authid().isEmpty() )
      parts << QStringLiteral( "crs=%1" ).arg( crs.authid() );
    else
      parts << QStringLiteral( "crs=wkt:%1" ).arg( crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );
  }
  for ( const auto &field : fields )
  {
    const QString lengthPrecision = QStringLiteral( "(%1,%2)" ).arg( field.length() ).arg( field.precision() );
    parts << QStringLiteral( "field=%1:%2%3%4" ).arg( QString( QUrl::toPercentEncoding( field.name() ) ),
          memoryLayerFieldType( field.type() == QVariant::List || field.type() == QVariant::StringList ? field.subType() : field.type() ),
          lengthPrecision,
          field.type() == QVariant::List || field.type() == QVariant::StringList ? QStringLiteral( "[]" ) : QString() );
  }

  const QString uri = geomType + '?' + parts.join( '&' );
  QgsVectorLayer::LayerOptions options{ QgsCoordinateTransformContext() };
  options.skipCrsValidation = true;
  return new QgsVectorLayer( uri, name, QStringLiteral( "memory" ), options );
}
