/***************************************************************************
                         qgsprocessingoutputs.cpp
                         -------------------------
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

#include "qgsprocessingoutputs.h"
#include "qgsvariantutils.h"

#include <QUrl>
#include <QDir>

QgsProcessingOutputDefinition::QgsProcessingOutputDefinition( const QString &name, const QString &description )
  : mName( name )
  , mDescription( description )
{}

QString QgsProcessingOutputDefinition::valueAsString( const QVariant &value, QgsProcessingContext &, bool &ok ) const
{
  if ( QgsVariantUtils::isNull( value ) )
  {
    ok = true;
    return QObject::tr( "NULL" );
  }

  if ( value.userType() == QMetaType::Type::QString )
  {
    ok = true;
    return value.toString();
  }
  ok = false;
  return QString();
}

QString QgsProcessingOutputDefinition::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsString( value, context, ok );
}

QgsProcessingOutputVectorLayer::QgsProcessingOutputVectorLayer( const QString &name, const QString &description, Qgis::ProcessingSourceType type )
  : QgsProcessingOutputDefinition( name, description )
  , mDataType( type )
{}

Qgis::ProcessingSourceType QgsProcessingOutputVectorLayer::dataType() const
{
  return mDataType;
}

void QgsProcessingOutputVectorLayer::setDataType( Qgis::ProcessingSourceType type )
{
  mDataType = type;
}

QgsProcessingOutputRasterLayer::QgsProcessingOutputRasterLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputPointCloudLayer::QgsProcessingOutputPointCloudLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputVectorTileLayer::QgsProcessingOutputVectorTileLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputHtml::QgsProcessingOutputHtml( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputHtml::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QgsProcessingOutputNumber::QgsProcessingOutputNumber( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputNumber::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = false;
  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
      ok = true;
      return value.toString();
    default:
      break;
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}

QgsProcessingOutputString::QgsProcessingOutputString( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputBoolean::QgsProcessingOutputBoolean( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputBoolean::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = false;
  if ( value.userType() == QMetaType::Type::Bool )
  {
    ok = true;
    return value.toBool() ? QObject::tr( "True" ) : QObject::tr( "False" );
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}

QgsProcessingOutputFolder::QgsProcessingOutputFolder( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputFolder::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QgsProcessingOutputFile::QgsProcessingOutputFile( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputFile::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QgsProcessingOutputMapLayer::QgsProcessingOutputMapLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputMapLayer::type() const
{
  return typeName();
}

QgsProcessingOutputMultipleLayers::QgsProcessingOutputMultipleLayers( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputMultipleLayers::type() const
{
  return typeName();
}

QString QgsProcessingOutputMultipleLayers::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = false;
  switch ( value.userType() )
  {
    case QMetaType::Type::QVariantList:
    {
      ok = true;
      const QVariantList list = value.toList();

      QStringList layerNames;
      for ( const QVariant &v : list )
      {
        layerNames << v.toString();
      }
      return layerNames.join( QLatin1String( ", " ) );
    }

    case QMetaType::Type::QStringList:
    {
      ok = true;
      const QStringList list = value.toStringList();
      return list.join( QLatin1String( ", " ) );
    }

    default:
      break;
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}

QgsProcessingOutputConditionalBranch::QgsProcessingOutputConditionalBranch( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputVariant::QgsProcessingOutputVariant( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{

}

QString QgsProcessingOutputVariant::type() const
{
  return typeName();
}

QString QgsProcessingOutputVariant::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = false;
  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
      ok = true;
      return value.toString();
    case QMetaType::Type::Bool:
      ok = true;
      return value.toBool() ? QObject::tr( "True" ) : QObject::tr( "False" );

    case QMetaType::Type::QVariantList:
    {
      ok = true;
      const QVariantList list = value.toList();

      QStringList names;
      for ( const QVariant &v : list )
      {
        names << v.toString();
      }
      return names.join( QLatin1String( ", " ) );
    }

    case QMetaType::Type::QStringList:
    {
      ok = true;
      const QStringList list = value.toStringList();
      return list.join( QLatin1String( ", " ) );
    }

    default:
      break;
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}
