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
      return QStringLiteral( "string" );
  }
  return QStringLiteral( "string" ); // no warnings
}

QgsVectorLayer *QgsMemoryProviderUtils::createMemoryLayer( const QString &name, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs )
{
  QString geomType = QgsWkbTypes::displayString( geometryType );
  if ( geomType.isNull() )
    geomType = QStringLiteral( "none" );

  QString uri = geomType + '?';

  bool first = true;
  if ( crs.isValid() )
  {
    uri += QStringLiteral( "crs=" ) + crs.authid();
    first = false;
  }

  QStringList fieldsStrings;
  Q_FOREACH ( const QgsField &field, fields )
  {
    fieldsStrings << QStringLiteral( "field=%1:%2" ).arg( field.name(), memoryLayerFieldType( field.type() ) );
  }

  if ( !fieldsStrings.isEmpty() )
  {
    if ( !first )
      uri += '&';
    first = false;
    uri += fieldsStrings.join( '&' );
  }

  return new QgsVectorLayer( uri, name, QStringLiteral( "memory" ) );
}
