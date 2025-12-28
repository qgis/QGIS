/***************************************************************************
  qgsmssqlutils.cpp
  --------------------------------------
  Date                 : February 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlutils.h"

#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsvariantutils.h"
#include "qgswkbtypes.h"

QString QgsMssqlUtils::quotedValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"NULL"_s;

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();

    case QMetaType::Type::Bool:
      return QString( value.toBool() ? '1' : '0' );

    default:
    case QMetaType::Type::QString:
      QString v = value.toString();
      v.replace( '\'', "''"_L1 );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', "\\\\"_L1 ).prepend( "N'" ).append( '\'' );
      else
        return v.prepend( "N'" ).append( '\'' );
  }
}

QString QgsMssqlUtils::quotedIdentifier( const QString &value )
{
  return u"[%1]"_s.arg( value );
}

QMetaType::Type QgsMssqlUtils::convertSqlFieldType( const QString &systemTypeName )
{
  QMetaType::Type type = QMetaType::Type::UnknownType;
  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( systemTypeName.startsWith( "decimal"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "numeric"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "real"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "float"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Double;
  }
  else if ( systemTypeName.startsWith( "char"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "nchar"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "varchar"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "nvarchar"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "text"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "ntext"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "uniqueidentifier"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( systemTypeName.startsWith( "smallint"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "int"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "bit"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "tinyint"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Int;
  }
  else if ( systemTypeName.startsWith( "bigint"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::LongLong;
  }
  else if ( systemTypeName.startsWith( "binary"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "varbinary"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "image"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QByteArray;
  }
  else if ( systemTypeName.startsWith( "datetime"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "smalldatetime"_L1, Qt::CaseInsensitive ) || systemTypeName.startsWith( "datetime2"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDateTime;
  }
  else if ( systemTypeName.startsWith( "date"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDate;
  }
  else if ( systemTypeName.startsWith( "timestamp"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( systemTypeName.startsWith( "time"_L1, Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QTime;
  }
  else
  {
    QgsDebugError( u"Unknown field type: %1"_s.arg( systemTypeName ) );
    // Everything else just dumped as a string.
    type = QMetaType::Type::QString;
  }
  // NOLINTEND(bugprone-branch-clone)

  return type;
}

QgsField QgsMssqlUtils::createField( const QString &name, const QString &systemTypeName, int length, int precision, int scale, bool nullable, bool unique, bool readOnly )
{
  QgsField field;
  const QMetaType::Type sqlType = convertSqlFieldType( systemTypeName );
  switch ( sqlType )
  {
    case QMetaType::Type::QString:
    {
      // Field length in chars is "Length" of the sp_columns output,
      // except for uniqueidentifiers which must use "Precision".
      int stringLength = systemTypeName.startsWith( u"uniqueidentifier"_s, Qt::CaseInsensitive ) ? precision : length;
      if ( systemTypeName.startsWith( QLatin1Char( 'n' ) ) )
      {
        stringLength = stringLength / 2;
      }
      field = QgsField( name, sqlType, systemTypeName, stringLength );
      break;
    }

    case QMetaType::Type::Double:
    {
      field = QgsField( name, sqlType, systemTypeName, precision, systemTypeName == "decimal"_L1 || systemTypeName == "numeric"_L1 ? scale : -1 );
      break;
    }

    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QTime:
    {
      field = QgsField( name, sqlType, systemTypeName, -1, -1 );
      break;
    }

    default:
    {
      field = QgsField( name, sqlType, systemTypeName );
      break;
    }
  }

  // Set constraints
  QgsFieldConstraints constraints;
  if ( !nullable )
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  if ( unique )
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
  field.setConstraints( constraints );

  if ( readOnly )
  {
    field.setReadOnly( true );
  }
  return field;
}

Qgis::WkbType QgsMssqlUtils::wkbTypeFromGeometryType( const QString &type )
{
  return QgsWkbTypes::parseType( type.toUpper() );
}
