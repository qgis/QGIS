/***************************************************************************
                              qgswfsparameters.cpp
                              --------------------
  begin                : Sept 14, 2017
  copyright            : (C) 2017 by René-Luc Dhont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsparameters.h"
#include "qgsmessagelog.h"
#include <iostream>

namespace QgsWfs
{
  QgsWfsParameters::QgsWfsParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 0, 0 ) );
    mVersions.append( QgsProjectVersion( 1, 1, 0 ) );

    const Parameter pOutputFormat = { ParameterName::OUTPUTFORMAT,
                                      QVariant::String,
                                      QVariant( "" ),
                                      QVariant()
                                    };
    save( pOutputFormat );

    const Parameter pSrsName = { ParameterName::SRSNAME,
                                 QVariant::String,
                                 QVariant( "" ),
                                 QVariant()
                               };
    save( pSrsName );
  }

  QgsWfsParameters::QgsWfsParameters( const QgsServerRequest::Parameters &parameters )
  {
    load( parameters );
  }

  void QgsWfsParameters::load( const QgsServerRequest::Parameters &parameters )
  {
    mRequestParameters = parameters;

    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    foreach ( QString key, parameters.keys() )
    {
      const ParameterName name = ( ParameterName ) metaEnum.keyToValue( key.toStdString().c_str() );
      if ( name >= 0 )
      {
        QVariant value( parameters[key] );
        if ( value.canConvert( mParameters[name].mType ) )
        {
          mParameters[name].mValue = value;
        }
        else
        {
          raiseError( name );
        }
      }
    }
  }

  void QgsWfsParameters::dump() const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

    log( "WFS Request parameters:" );
    for ( auto parameter : mParameters.toStdMap() )
    {
      const QString value = parameter.second.mValue.toString();

      if ( ! value.isEmpty() )
      {
        const QString name = metaEnum.valueToKey( parameter.first );
        log( " - " + name + " : " + value );
      }
    }

    if ( !version().isEmpty() )
      log( " - VERSION : " + version() );
  }

  void QgsWfsParameters::save( const Parameter &parameter )
  {
    mParameters[ parameter.mName ] = parameter;
  }

  QVariant QgsWfsParameters::value( ParameterName name ) const
  {
    return mParameters[name].mValue;
  }

  QVariant QgsWfsParameters::defaultValue( ParameterName name ) const
  {
    return mParameters[name].mDefaultValue;
  }

  QString QgsWfsParameters::outputFormatAsString() const
  {
    return value( ParameterName::OUTPUTFORMAT ).toString();
  }

  QgsWfsParameters::Format QgsWfsParameters::outputFormat() const
  {
    QString fStr = outputFormatAsString();

    if ( fStr.isEmpty() )
    {
      if ( versionAsNumber() >= QgsProjectVersion( 1, 1, 0 ) )
        return Format::GML3;
      else
        return Format::GML2;
    }

    Format f = Format::NONE;
    if ( fStr.compare( QLatin1String( "text/xml; subtype=gml/2.1.2" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;
    else if ( fStr.compare( QLatin1String( "text/xml; subtype=gml/3.1.1" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML3;
    else if ( fStr.compare( QLatin1String( "application/vnd.geo+json" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GeoJSON;
    else if ( fStr.compare( QLatin1String( "gml2" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;
    else if ( fStr.compare( QLatin1String( "gml3" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML3;
    else if ( fStr.compare( QLatin1String( "geojson" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GeoJSON;

    return f;
  }

  QString QgsWfsParameters::srsName() const
  {
    return value( ParameterName::SRSNAME ).toString();
  }

  QString QgsWfsParameters::version() const
  {
    // VERSION parameter is not managed with other parameters because
    // there's a conflict with qgis VERSION defined in qgsconfig.h
    if ( mRequestParameters.contains( "VERSION" ) )
      return mRequestParameters["VERSION"];
    else
      return QString();
  }

  QgsProjectVersion QgsWfsParameters::versionAsNumber() const
  {
    QString vStr = version();
    QgsProjectVersion version;

    if ( vStr.isEmpty() )
      version = QgsProjectVersion( 1, 1, 0 ); // default value
    else if ( mVersions.contains( QgsProjectVersion( vStr ) ) )
      version = QgsProjectVersion( vStr );

    return version;
  }

  QString QgsWfsParameters::name( ParameterName name ) const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    return metaEnum.valueToKey( name );
  }

  void QgsWfsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, "Server", QgsMessageLog::INFO );
  }

  void QgsWfsParameters::raiseError( ParameterName paramName ) const
  {
    const QString value = mParameters[paramName].mValue.toString();
    const QString param = name( paramName );
    const QString type = QVariant::typeToName( mParameters[paramName].mType );
    raiseError( param + " ('" + value + "') cannot be converted into " + type );
  }

  void QgsWfsParameters::raiseError( const QString &msg ) const
  {
    throw QgsBadRequestException( QStringLiteral( "Invalid WFS Parameter" ), msg );
  }
}