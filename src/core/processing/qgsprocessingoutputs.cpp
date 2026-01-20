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

#include <QDir>
#include <QUrl>

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

QColor QgsProcessingOutputDefinition::modelColor() const
{
  return QColor( 128, 128, 128 ); /* mid  gray */
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

QColor QgsProcessingOutputVectorLayer::modelColor() const
{
  return QColor( 122, 0, 47 ); /* burgundy */
}

QgsProcessingOutputRasterLayer::QgsProcessingOutputRasterLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QColor QgsProcessingOutputRasterLayer::modelColor() const
{
  return QColor( 0, 180, 180 ); /* turquoise */
}

QgsProcessingOutputPointCloudLayer::QgsProcessingOutputPointCloudLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputVectorTileLayer::QgsProcessingOutputVectorTileLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QColor QgsProcessingOutputVectorTileLayer::modelColor() const
{
  return QColor( 137, 150, 171 ); /* cold gray */
}

QgsProcessingOutputHtml::QgsProcessingOutputHtml( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputHtml::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return u"<a href=\"%1\">%2</a>"_s.arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QColor QgsProcessingOutputHtml::modelColor() const
{
  return QColor( 255, 131, 23 ); /* orange */
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

QColor QgsProcessingOutputNumber::modelColor() const
{
  return QColor( 34, 157, 214 ); /* blue */
}

QgsProcessingOutputString::QgsProcessingOutputString( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QColor QgsProcessingOutputString::modelColor() const
{
  return QColor( 255, 131, 23 ); /* orange */
}

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

QColor QgsProcessingOutputBoolean::modelColor() const
{
  return QColor( 51, 201, 28 ); /* green */
}

QgsProcessingOutputFolder::QgsProcessingOutputFolder( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputFolder::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return u"<a href=\"%1\">%2</a>"_s.arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QColor QgsProcessingOutputFolder::modelColor() const
{
  return QColor( 80, 80, 80 ); /* dark gray */
}

QgsProcessingOutputFile::QgsProcessingOutputFile( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputFile::valueAsFormattedString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.userType() == QMetaType::Type::QString && !value.toString().isEmpty() )
  {
    ok = true;
    return u"<a href=\"%1\">%2</a>"_s.arg( QUrl::fromLocalFile( value.toString() ).toString(), QDir::toNativeSeparators( value.toString() ) );
  }

  return valueAsString( value, context, ok );
}

QColor QgsProcessingOutputFile::modelColor() const
{
  return QColor( 80, 80, 80 ); /* dark gray */
}

QgsProcessingOutputMapLayer::QgsProcessingOutputMapLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputMapLayer::type() const
{
  return typeName();
}

QColor QgsProcessingOutputMapLayer::modelColor() const
{
  return QColor( 137, 150, 171 ); /* cold gray */
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
      return layerNames.join( ", "_L1 );
    }

    case QMetaType::Type::QStringList:
    {
      ok = true;
      const QStringList list = value.toStringList();
      return list.join( ", "_L1 );
    }

    default:
      break;
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}

QColor QgsProcessingOutputMultipleLayers::modelColor() const
{
  return QColor( 137, 150, 171 ); /* cold gray */
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
      return names.join( ", "_L1 );
    }

    case QMetaType::Type::QStringList:
    {
      ok = true;
      const QStringList list = value.toStringList();
      return list.join( ", "_L1 );
    }

    default:
      break;
  }
  return QgsProcessingOutputDefinition::valueAsString( value, context, ok );
}

