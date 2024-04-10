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

QString memoryLayerFieldType( QMetaType::Type type, const QString &typeString )
{
  switch ( type )
  {
    case QMetaType::Type::Int:
      return QStringLiteral( "integer" );

    case QMetaType::Type::LongLong:
      return QStringLiteral( "long" );

    case QMetaType::Type::Double:
      return QStringLiteral( "double" );

    case QMetaType::Type::QString:
      return QStringLiteral( "string" );

    case QMetaType::Type::QDate:
      return QStringLiteral( "date" );

    case QMetaType::Type::QTime:
      return QStringLiteral( "time" );

    case QMetaType::Type::QDateTime:
      return QStringLiteral( "datetime" );

    case QMetaType::Type::QByteArray:
      return QStringLiteral( "binary" );

    case QMetaType::Type::Bool:
      return QStringLiteral( "boolean" );

    case QMetaType::Type::QVariantMap:
      return QStringLiteral( "map" );

    case QMetaType::Type::User:
      if ( typeString.compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 )
      {
        return QStringLiteral( "geometry" );
      }
      break;

    default:
      break;
  }
  return QStringLiteral( "string" );
}

QgsVectorLayer *QgsMemoryProviderUtils::createMemoryLayer( const QString &name, const QgsFields &fields, Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs, bool loadDefaultStyle )
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
      parts << QStringLiteral( "crs=wkt:%1" ).arg( crs.toWkt( Qgis::CrsWktVariant::Preferred ) );
  }
  else
  {
    parts << QStringLiteral( "crs=" );
  }
  for ( const QgsField &field : fields )
  {
    const QString lengthPrecision = QStringLiteral( "(%1,%2)" ).arg( field.length() ).arg( field.precision() );
    parts << QStringLiteral( "field=%1:%2%3%4" ).arg( QString( QUrl::toPercentEncoding( field.name() ) ),
          memoryLayerFieldType( field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList ? field.subType() : field.type(), field.typeName() ),
          lengthPrecision,
          field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList ? QStringLiteral( "[]" ) : QString() );
  }

  const QString uri = geomType + '?' + parts.join( '&' );
  QgsVectorLayer::LayerOptions options{ QgsCoordinateTransformContext() };
  options.skipCrsValidation = true;
  options.loadDefaultStyle = loadDefaultStyle;
  return new QgsVectorLayer( uri, name, QStringLiteral( "memory" ), options );
}
