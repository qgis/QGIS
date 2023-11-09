/***************************************************************************
                              qgswmtsparameters.cpp
                              --------------------
  begin                : Aug 10, 2018
  copyright            : (C) 2018 by René-Luc Dhont
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

#include "qgswmtsparameters.h"
#include "qgsmessagelog.h"

namespace QgsWmts
{
  //
  // QgsWmsParameterForWmts
  //
  QString QgsWmsParameterForWmts::name( const QgsWmsParameterForWmts::Name name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmsParameterForWmts::Name>() );
    return metaEnum.valueToKey( name );
  }

  QgsWmsParameterForWmts::Name QgsWmsParameterForWmts::name( const QString &name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmsParameterForWmts::Name>() );
    return ( QgsWmsParameterForWmts::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
  }

  //
  // QgsWmtsParameter
  //
  QgsWmtsParameter::QgsWmtsParameter( const QgsWmtsParameter::Name name,
                                      const QVariant::Type type,
                                      const QVariant defaultValue )
    : QgsServerParameterDefinition( type, defaultValue )
    , mName( name )
  {
  }

  int QgsWmtsParameter::toInt() const
  {
    bool ok = false;
    const int val = QgsServerParameterDefinition::toInt( ok );

    if ( !ok )
    {
      raiseError();
    }

    return val;
  }

  void QgsWmtsParameter::raiseError() const
  {
    const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( name( mName ), toString(), typeName() );
    QgsServerParameterDefinition::raiseError( msg );
  }

  QString QgsWmtsParameter::name( const QgsWmtsParameter::Name name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmtsParameter::Name>() );
    return metaEnum.valueToKey( name );
  }

  QgsWmtsParameter::Name QgsWmtsParameter::name( const QString &name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmtsParameter::Name>() );
    return ( QgsWmtsParameter::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
  }

  //
  // QgsWmtsParameters
  //
  QgsWmtsParameters::QgsWmtsParameters()
    : QgsServerParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 0, 0 ) );

    const QgsWmtsParameter pLayer = QgsWmtsParameter( QgsWmtsParameter::LAYER );
    save( pLayer );

    const QgsWmtsParameter pFormat = QgsWmtsParameter( QgsWmtsParameter::FORMAT );
    save( pFormat );

    const QgsWmtsParameter pTileMatrix = QgsWmtsParameter( QgsWmtsParameter::TILEMATRIX,
                                         QVariant::Int,
                                         QVariant( -1 ) );
    save( pTileMatrix );

    const QgsWmtsParameter pTileRow = QgsWmtsParameter( QgsWmtsParameter::TILEROW,
                                      QVariant::Int,
                                      QVariant( -1 ) );
    save( pTileRow );

    const QgsWmtsParameter pTileCol = QgsWmtsParameter( QgsWmtsParameter::TILECOL,
                                      QVariant::Int,
                                      QVariant( -1 ) );
    save( pTileCol );

    const QgsWmtsParameter pInfoFormat( QgsWmtsParameter::INFOFORMAT );
    save( pInfoFormat );

    const QgsWmtsParameter pI( QgsWmtsParameter::I,
                               QVariant::Int,
                               QVariant( -1 ) );
    save( pI );

    const QgsWmtsParameter pJ( QgsWmtsParameter::J,
                               QVariant::Int,
                               QVariant( -1 ) );
    save( pJ );
  }

  QgsWmtsParameters::QgsWmtsParameters( const QgsServerParameters &parameters )
    : QgsWmtsParameters()
  {
    load( parameters.urlQuery() );
  }

  bool QgsWmtsParameters::loadParameter( const QString &key, const QString &value )
  {
    bool loaded = false;

    const QgsWmtsParameter::Name name = QgsWmtsParameter::name( key );
    if ( name >= 0 )
    {
      mWmtsParameters[name].mValue = value;
      if ( ! mWmtsParameters[name].isValid() )
      {
        mWmtsParameters[name].raiseError();
      }

      loaded = true;
    }

    return loaded;
  }

  void QgsWmtsParameters::save( const QgsWmtsParameter &parameter )
  {
    mWmtsParameters[ parameter.mName ] = parameter;
  }

  void QgsWmtsParameters::dump() const
  {
    log( "WMTS Request parameters:" );
    const auto map = mWmtsParameters.toStdMap();
    for ( const auto &parameter : map )
    {
      const QString value = parameter.second.toString();

      if ( ! value.isEmpty() )
      {
        const QString name = QgsWmtsParameter::name( parameter.first );
        log( QStringLiteral( " - %1 : %2" ).arg( name, value ) );
      }
    }

    if ( !version().isEmpty() )
      log( QStringLiteral( " - VERSION : %1" ).arg( version() ) );
  }

  QString QgsWmtsParameters::layer() const
  {
    return mWmtsParameters[ QgsWmtsParameter::LAYER ].toString();
  }

  QString QgsWmtsParameters::formatAsString() const
  {
    return mWmtsParameters[ QgsWmtsParameter::FORMAT ].toString();
  }

  QgsWmtsParameters::Format QgsWmtsParameters::format() const
  {
    const QString fStr = formatAsString();

    if ( fStr.isEmpty() )
      return Format::NONE;

    Format f = Format::PNG;
    if ( fStr.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0
         || fStr.compare( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) == 0
         || fStr.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
      f = Format::JPG;

    return f;
  }

  QString QgsWmtsParameters::tileMatrixSet() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILEMATRIXSET ].toString();
  }

  QString QgsWmtsParameters::tileMatrix() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILEMATRIX ].toString();
  }

  int QgsWmtsParameters::tileMatrixAsInt() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILEMATRIX ].toInt();
  }

  QString QgsWmtsParameters::tileRow() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILEROW ].toString();
  }

  int QgsWmtsParameters::tileRowAsInt() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILEROW ].toInt();
  }

  QString QgsWmtsParameters::tileCol() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILECOL ].toString();
  }

  int QgsWmtsParameters::tileColAsInt() const
  {
    return mWmtsParameters[ QgsWmtsParameter::TILECOL ].toInt();
  }

  QString QgsWmtsParameters::infoFormatAsString() const
  {
    return mWmtsParameters[ QgsWmtsParameter::INFOFORMAT ].toString();
  }

  QgsWmtsParameters::Format QgsWmtsParameters::infoFormat() const
  {
    const QString fStr = infoFormatAsString();

    Format f = Format::TEXT;
    if ( fStr.isEmpty() )
      return f;

    if ( fStr.startsWith( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) )
      f = Format::XML;
    else if ( fStr.startsWith( QLatin1String( "text/html" ), Qt::CaseInsensitive ) )
      f = Format::HTML;
    else if ( fStr.startsWith( QLatin1String( "text/plain" ), Qt::CaseInsensitive ) )
      f = Format::TEXT;
    else if ( fStr.startsWith( QLatin1String( "application/vnd.ogc.gml" ), Qt::CaseInsensitive ) )
      f = Format::GML;
    else
      f = Format::NONE;

    return f;
  }

  int QgsWmtsParameters::infoFormatVersion() const
  {
    if ( infoFormat() != Format::GML )
      return -1;

    const QString fStr = infoFormatAsString();
    if ( fStr.startsWith( QLatin1String( "application/vnd.ogc.gml/3" ), Qt::CaseInsensitive ) )
      return 3;
    else
      return 2;
  }

  QString QgsWmtsParameters::i() const
  {
    return mWmtsParameters[ QgsWmtsParameter::I ].toString();
  }

  QString QgsWmtsParameters::j() const
  {
    return mWmtsParameters[ QgsWmtsParameter::J ].toString();
  }

  int QgsWmtsParameters::iAsInt() const
  {
    return mWmtsParameters[ QgsWmtsParameter::I ].toInt();
  }

  int QgsWmtsParameters::jAsInt() const
  {
    return mWmtsParameters[ QgsWmtsParameter::J ].toInt();
  }

  QgsProjectVersion QgsWmtsParameters::versionAsNumber() const
  {
    const QString vStr = version();
    QgsProjectVersion version;

    if ( vStr.isEmpty() )
      version = QgsProjectVersion( 1, 0, 0 ); // default value
    else if ( mVersions.contains( QgsProjectVersion( vStr ) ) )
      version = QgsProjectVersion( vStr );

    return version;
  }

  void QgsWmtsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, "Server", Qgis::MessageLevel::Info );
  }
}
