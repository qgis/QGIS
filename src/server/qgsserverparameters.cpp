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

QgsServerParameters::QgsServerParameters()
{
  const Parameter pService = { ParameterName::SERVICE,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant(),
                               false
                             };
  save( pService );

  const Parameter pRequest = { ParameterName::REQUEST,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant(),
                               false
                             };
  save( pRequest );

  const Parameter pVersion = { ParameterName::VERSION_SERVICE,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant(),
                               false
                             };
  save( pVersion );

  const Parameter pMap = { ParameterName::MAP,
                           QVariant::String,
                           QVariant( "" ),
                           QVariant(),
                           false
                         };
  save( pMap );

  const Parameter pFile = { ParameterName::FILE_NAME,
                            QVariant::String,
                            QVariant( "" ),
                            QVariant(),
                            false
                          };
  save( pFile );
}

QgsServerParameters::QgsServerParameters( const QUrlQuery &query )
  : QgsServerParameters()
{
  load( query );
}

void QgsServerParameters::save( const Parameter &parameter )
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
    ParameterName paramName = name( key );
    if ( mParameters.contains( paramName ) )
    {
      mParameters.take( paramName );
    }
  }
}

QString QgsServerParameters::map() const
{
  return value( ParameterName::MAP ).toString();
}

QString QgsServerParameters::version() const
{
  return value( ParameterName::VERSION_SERVICE ).toString();
}

QString QgsServerParameters::fileName() const
{
  return value( ParameterName::FILE_NAME ).toString();
}

QString QgsServerParameters::service() const
{
  QString serviceValue = value( ParameterName::SERVICE ).toString();

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

  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

  for ( auto parameter : mParameters.toStdMap() )
  {
    if ( ! parameter.second.mDefined )
      continue;

    const QString paramName = name( parameter.first );
    params[paramName] = parameter.second.mValue.toString();
  }

  return params;
}

QString QgsServerParameters::request() const
{
  return value( ParameterName::REQUEST ).toString();
}

QString QgsServerParameters::value( const QString &key ) const
{
  return value( name( key ) ).toString();
}

QVariant QgsServerParameters::value( ParameterName name ) const
{
  return mParameters[name].mValue;
}

void QgsServerParameters::load( const QUrlQuery &query )
{
  // clean query string first
  QUrlQuery cleanQuery( query );
  cleanQuery.setQuery( query.query().replace( '+', QStringLiteral( "%20" ) ) );

  // load parameters
  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

  for ( const auto &item : cleanQuery.queryItems( QUrl::FullyDecoded ) )
  {
    const ParameterName paramName = name( item.first );
    if ( paramName >= 0 )
    {
      const QVariant value( item.second );
      mParameters[paramName].mValue = value;
      mParameters[paramName].mDefined = true;
      if ( !value.canConvert( mParameters[paramName].mType ) )
      {
        raiseError( paramName );
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

QString QgsServerParameters::name( ParameterName name ) const
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
  return metaEnum.valueToKey( name );
}

QgsServerParameters::ParameterName QgsServerParameters::name( const QString &key ) const
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
  return ( ParameterName ) metaEnum.keyToValue( key.toUpper().toStdString().c_str() );
}

void QgsServerParameters::raiseError( ParameterName paramName ) const
{
  const QString value = mParameters[paramName].mValue.toString();
  const QString param = name( paramName );
  const QString type = QVariant::typeToName( mParameters[paramName].mType );
  const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( param, value, type );
  raiseError( msg );
}

void QgsServerParameters::raiseError( const QString &msg ) const
{
  throw QgsBadRequestException( QStringLiteral( "Invalid WMS Parameter" ), msg );
}
