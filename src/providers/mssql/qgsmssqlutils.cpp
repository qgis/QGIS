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
#include "qgsvariantutils.h"
#include "qgslogger.h"
#include "qgsfield.h"
#include "qgswkbtypes.h"

QString QgsMssqlUtils::quotedValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return QStringLiteral( "NULL" );

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
      v.replace( '\'', QLatin1String( "''" ) );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "N'" ).append( '\'' );
      else
        return v.prepend( "N'" ).append( '\'' );
  }
}

QString QgsMssqlUtils::quotedIdentifier( const QString &value )
{
  return QStringLiteral( "[%1]" ).arg( value );
}

QMetaType::Type QgsMssqlUtils::convertSqlFieldType( const QString &systemTypeName )
{
  QMetaType::Type type = QMetaType::Type::UnknownType;
  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( systemTypeName.startsWith( QLatin1String( "decimal" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "numeric" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "real" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "float" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Double;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "char" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "nchar" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "varchar" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "nvarchar" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "text" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "ntext" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "uniqueidentifier" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "smallint" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "int" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "bit" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "tinyint" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Int;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "bigint" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::LongLong;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "binary" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "varbinary" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "image" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QByteArray;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "datetime" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "smalldatetime" ), Qt::CaseInsensitive ) || systemTypeName.startsWith( QLatin1String( "datetime2" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDateTime;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "date" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDate;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( systemTypeName.startsWith( QLatin1String( "time" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QTime;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Unknown field type: %1" ).arg( systemTypeName ) );
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
      int stringLength = systemTypeName.startsWith( QStringLiteral( "uniqueidentifier" ), Qt::CaseInsensitive ) ? precision : length;
      if ( systemTypeName.startsWith( QLatin1Char( 'n' ) ) )
      {
        stringLength = stringLength / 2;
      }
      field = QgsField( name, sqlType, systemTypeName, stringLength );
      break;
    }

    case QMetaType::Type::Double:
    {
      field = QgsField( name, sqlType, systemTypeName, precision, systemTypeName == QLatin1String( "decimal" ) || systemTypeName == QLatin1String( "numeric" ) ? scale : -1 );
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
