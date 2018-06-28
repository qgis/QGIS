/***************************************************************************
                              qgsserverparameters.cpp
                              --------------------
  begin                : Jun 27, 2018
  copyright            : (C) 2018 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverparameters.h"
#include "qgsserverexception.h"

//
// QgsServerParameterDefinition
//
QgsServerParameterDefinition::QgsServerParameterDefinition( const QVariant::Type type,
    const QVariant defaultValue )
  : mType( type )
  , mDefaultValue( defaultValue )
{
}

QString QgsServerParameterDefinition::typeName() const
{
  return  QVariant::typeToName( mType );
}

bool QgsServerParameterDefinition::isValid() const
{
  return mValue.canConvert( mType );
}

void QgsServerParameterDefinition::raiseError( const QString &msg )
{
  throw QgsBadRequestException( QStringLiteral( "Invalid Parameter" ), msg );
}

//
// QgsServerParameter
//
QgsServerParameter::QgsServerParameter( const QgsServerParameter::Name name,
                                        const QVariant::Type type, const QVariant defaultValue )
  : QgsServerParameterDefinition( type, defaultValue )
  , mName( name )
{
}

QString QgsServerParameter::name( QgsServerParameter::Name name )
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerParameter::Name>() );
  return metaEnum.valueToKey( name );
}

QgsServerParameter::Name QgsServerParameter::name( const QString &name )
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerParameter::Name>() );
  return ( QgsServerParameter::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
}

void QgsServerParameter::raiseError() const
{
  const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( name( mName ), mValue.toString(), typeName() );
  QgsServerParameterDefinition::raiseError( msg );
}

//
// QgsServerParameters
//
QgsServerParameters::QgsServerParameters()
{
  save( QgsServerParameter( QgsServerParameter::SERVICE ) );
  save( QgsServerParameter( QgsServerParameter::REQUEST ) );
  save( QgsServerParameter( QgsServerParameter::VERSION_SERVICE ) );
  save( QgsServerParameter( QgsServerParameter::MAP ) );
  save( QgsServerParameter( QgsServerParameter::FILE_NAME ) );
}

QgsServerParameters::QgsServerParameters( const QUrlQuery &query )
  : QgsServerParameters()
{
  load( query );
}

void QgsServerParameters::save( const QgsServerParameter &parameter )
{
  mParameters[ parameter.mName ] = parameter;
}

void QgsServerParameters::add( const QString &key, const QString &value )
{
  QUrlQuery query;
  query.addQueryItem( key, value );
  load( query );
}

QUrlQuery QgsServerParameters::urlQuery() const
{
  QUrlQuery query;

  for ( auto param : toMap().toStdMap() )
  {
    query.addQueryItem( param.first, param.second );
  }

  return query;
}

void QgsServerParameters::remove( const QString &key )
{
  if ( mUnmanagedParameters.contains( key ) )
  {
    mUnmanagedParameters.take( key );
  }
  else
  {
    QgsServerParameter::Name paramName = QgsServerParameter::name( key );
    if ( mParameters.contains( paramName ) )
    {
      mParameters.take( paramName );
    }
  }
}

QString QgsServerParameters::map() const
{
  return value( QgsServerParameter::MAP ).toString();
}

QString QgsServerParameters::version() const
{
  return value( QgsServerParameter::VERSION_SERVICE ).toString();
}

QString QgsServerParameters::fileName() const
{
  return value( QgsServerParameter::FILE_NAME ).toString();
}

QString QgsServerParameters::service() const
{
  QString serviceValue = value( QgsServerParameter::SERVICE ).toString();

  if ( serviceValue.isEmpty() )
  {
    // SERVICE not mandatory for WMS 1.3.0 GetMap & GetFeatureInfo
    if ( request() == QLatin1String( "GetMap" ) \
         || request() == QLatin1String( "GetFeatureInfo" ) )
    {
      serviceValue = "WMS";
    }
  }

  return serviceValue;
}

QMap<QString, QString> QgsServerParameters::toMap() const
{
  QMap<QString, QString> params = mUnmanagedParameters;

  for ( auto parameter : mParameters.toStdMap() )
  {
    if ( parameter.second.mValue.isNull() )
      continue;

    const QString paramName = QgsServerParameter::name( parameter.first );
    params[paramName] = parameter.second.mValue.toString();
  }

  return params;
}

QString QgsServerParameters::request() const
{
  return value( QgsServerParameter::REQUEST ).toString();
}

QString QgsServerParameters::value( const QString &key ) const
{
  return value( QgsServerParameter::name( key ) ).toString();
}

QVariant QgsServerParameters::value( QgsServerParameter::Name name ) const
{
  return mParameters[name].mValue;
}

void QgsServerParameters::load( const QUrlQuery &query )
{
  // clean query string first
  QUrlQuery cleanQuery( query );
  cleanQuery.setQuery( query.query().replace( '+', QStringLiteral( "%20" ) ) );

  // load parameters
  for ( const auto &item : cleanQuery.queryItems( QUrl::FullyDecoded ) )
  {
    const QgsServerParameter::Name name = QgsServerParameter::name( item.first );
    if ( name >= 0 )
    {
      mParameters[name].mValue = item.second;
      if ( ! mParameters[name].isValid() )
      {
        mParameters[name].raiseError();
      }
    }
    else
    {
      mUnmanagedParameters[item.first.toUpper()] = item.second;
    }
  }
}

void QgsServerParameters::clear()
{
  mParameters.clear();
  mUnmanagedParameters.clear();
}
