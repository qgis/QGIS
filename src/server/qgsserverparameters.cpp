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
                               QVariant()
                             };
  save( pService );

  const Parameter pRequest = { ParameterName::REQUEST,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant()
                             };
  save( pRequest );

  const Parameter pVersion = { ParameterName::VERSION_SERVICE,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant()
                             };
  save( pService );

  const Parameter pMap = { ParameterName::MAP,
                           QVariant::String,
                           QVariant( "" ),
                           QVariant()
                         };
  save( pMap );

  const Parameter pFile = { ParameterName::FILE_NAME,
                            QVariant::String,
                            QVariant( "" ),
                            QVariant()
                          };
  save( pFile );
}

QgsServerParameters::QgsServerParameters( const QgsServerRequest::Parameters &parameters )
  : QgsServerParameters()
{
  load( parameters );
}

void QgsServerParameters::save( const Parameter &parameter )
{
  mParameters[ parameter.mName ] = parameter;
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

QString QgsServerParameters::request() const
{
  return value( ParameterName::REQUEST ).toString();
}

QVariant QgsServerParameters::value( ParameterName name ) const
{
  return mParameters[name].mValue;
}

void QgsServerParameters::load( const QgsServerRequest::Parameters &parameters )
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

  for ( const QString &key : parameters.keys() )
  {
    const ParameterName name = ( ParameterName ) metaEnum.keyToValue( key.toStdString().c_str() );
    if ( name >= 0 )
    {
      const QVariant value( parameters[key] );
      mParameters[name].mValue = value;

      if ( !value.canConvert( mParameters[name].mType ) )
      {
        raiseError( name );
      }
    }
    else
    {
      mUnmanagedParameters[key] = parameters[key];
    }
  }
}

QString QgsServerParameters::name( ParameterName name ) const
{
  const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
  return metaEnum.valueToKey( name );
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
