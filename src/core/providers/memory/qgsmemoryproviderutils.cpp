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
    parts << QStringLiteral( "crs=" ) + crs.authid();
  }
  for ( const auto &field : fields )
  {
    QString lengthPrecision;
    if ( field.length() > 0 && field.precision() > 0 )
    {
      lengthPrecision = QStringLiteral( "(%1,%2)" ).arg( field.length() ).arg( field.precision() );
    }
    else if ( field.length() > 0 )
    {
      lengthPrecision = QStringLiteral( "(%1)" ).arg( field.length() );
    }
    parts << QStringLiteral( "field=%1:%2%3" ).arg( field.name(), memoryLayerFieldType( field.type() ), lengthPrecision );
  }

  QString uri = geomType + '?' + parts.join( '&' );

  return new QgsVectorLayer( uri, name, QStringLiteral( "memory" ) );
}
