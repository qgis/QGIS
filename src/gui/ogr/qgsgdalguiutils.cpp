/***************************************************************************
                          qgsgdalguiutils.cpp
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalguiutils.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsgdalutils.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgsfilterlineedit.h"

#include <QComboBox>

QString QgsGdalGuiUtils::createDatabaseURI( const QString &connectionType, const QString &host, const QString &database, QString port, const QString &configId, QString username, QString password, bool expandAuthConfig )
{
  QString uri;

  // If an auth configuration is set, override username and password
  // Note that only Basic auth (username/password) is for now supported for OGR connections
  if ( !configId.isEmpty() )
  {
    // Blank credentials: we are using authcfg!
    username = QString();
    password = QString();
    // append authcfg is at the end, because we want to append the authcfg as last argument
  }

  //todo:add default ports for all kind of databases
  if ( connectionType == QLatin1String( "ESRI Personal GeoDatabase" ) )
  {
    uri = "PGeo:" + database;
  }
  else if ( connectionType == QLatin1String( "ESRI ArcSDE" ) )
  {
    if ( port.isEmpty() )
      port = QStringLiteral( "5151" );

    uri = "SDE:" + host + ",PORT:" + port + ',' + database + ',' + username + ',' + password;
  }
  else if ( connectionType == QLatin1String( "Informix DataBlade" ) )
  {
    //not tested
    uri = "IDB:dbname=" + database;

    if ( !host.isEmpty() )
      uri += QStringLiteral( " server=%1" ).arg( host );

    if ( !username.isEmpty() )
    {
      uri += QStringLiteral( " user=%1" ).arg( username );

      if ( !password.isEmpty() )
        uri += QStringLiteral( " pass=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "Ingres" ) )
  {
    //not tested
    uri = "@driver=ingres,dbname=" + database;
    if ( !username.isEmpty() )
    {
      uri += QStringLiteral( ",userid=%1" ).arg( username );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "MySQL" ) )
  {
    uri = "MySQL:" + database;

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( ",host=%1" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( ",port=%1" ).arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += QStringLiteral( ",user=%1" ).arg( username );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "MSSQL" ) )
  {
    uri = QStringLiteral( "MSSQL:" );

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( ";server=%1" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( ",%1" ).arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += QStringLiteral( ";uid=%1" ).arg( username );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ";pwd=%1" ).arg( password );
    }
    else
      uri += QLatin1String( ";trusted_connection=yes" );

    if ( !database.isEmpty() )
      uri += QStringLiteral( ";database=%1" ).arg( database );
  }
  else if ( connectionType == QLatin1String( "Oracle Spatial" ) )
  {
    uri = "OCI:" + username;

    if ( ( !username.isEmpty() && !password.isEmpty() ) || ( username.isEmpty() && password.isEmpty() ) )
    {
      uri += '/';
      if ( !password.isEmpty() )
        uri += password;
    }

    if ( !host.isEmpty() || !database.isEmpty() )
    {
      uri += '@';

      if ( !host.isEmpty() )
      {
        uri += host;
        if ( !port.isEmpty() )
          uri += ':' + port;
      }

      if ( !database.isEmpty() )
      {
        if ( !host.isEmpty() )
          uri += '/';
        uri += database;
      }
    }
  }
  else if ( connectionType == QLatin1String( "ODBC" ) )
  {
    if ( !username.isEmpty() )
    {
      if ( password.isEmpty() )
      {
        uri = "ODBC:" + username + '@' + database;
      }
      else
      {
        uri = "ODBC:" + username + '/' + password + '@' + database;
      }
    }
    else
    {
      uri = "ODBC:" + database;
    }
  }
  else if ( connectionType == QLatin1String( "OGDI Vectors" ) )
  {
  }
  else if ( connectionType == QLatin1String( "PostgreSQL" ) )
  {
    uri = "PG:dbname='" + database + '\'';

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( " host='%1'" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( " port='%1'" ).arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += QStringLiteral( " user='%1'" ).arg( username );

      if ( !password.isEmpty() )
        uri += QStringLiteral( " password='%1'" ).arg( password );
    }

    uri += ' ';
  }
  // Append authentication configuration to the URI
  if ( !( configId.isEmpty() ) )
  {
    if ( !expandAuthConfig )
    {
      uri += QStringLiteral( " authcfg='%1'" ).arg( configId );
    }
    else
    {
      QStringList connectionItems;
      connectionItems << uri;
      if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, QStringLiteral( "ogr" ) ) )
      {
        uri = connectionItems.join( QString() );
      }
    }
  }
  QgsDebugMsgLevel( "Connection type is=" + connectionType + " and uri=" + uri, 2 );
  return uri;
}


QString QgsGdalGuiUtils::createProtocolURI( const QString &type, const QString &url, const QString &configId, const QString &username, const QString &password, bool expandAuthConfig )
{
  QString uri;
  if ( type == QLatin1String( "vsicurl" ) )
  {
    uri = url;
    // If no protocol is provided in the URL, default to HTTP
    if ( !uri.startsWith( "http://" ) && !uri.startsWith( "https://" ) && !uri.startsWith( "ftp://" ) )
    {
      uri.prepend( QStringLiteral( "http://" ) );
    }
    uri.prepend( QStringLiteral( "/vsicurl/" ) );
  }
  else if ( type == QLatin1String( "vsis3" )
            || type == QLatin1String( "vsigs" )
            || type == QLatin1String( "vsiaz" )
            || type == QLatin1String( "vsiadls" )
            || type == QLatin1String( "vsioss" )
            || type == QLatin1String( "vsiswift" )
            || type == QLatin1String( "vsihdfs" ) )
  {
    uri = url;
    uri.prepend( QStringLiteral( "/%1/" ).arg( type ) );
  }
  // catching both GeoJSON and GeoJSONSeq
  else if ( type.startsWith( QLatin1String( "GeoJSON" ) ) )
  {
    uri = url;
  }
  else if ( type == QLatin1String( "CouchDB" ) )
  {
    uri = QStringLiteral( "couchdb:%1" ).arg( url );
  }
  else if ( type == QLatin1String( "DODS/OPeNDAP" ) )
  {
    uri = QStringLiteral( "DODS:%1" ).arg( url );
  }
  else if ( type == QLatin1String( "WFS3" ) )
  {
    uri = QStringLiteral( "WFS3:%1" ).arg( url );
  }
  QgsDebugMsgLevel( "Connection type is=" + type + " and uri=" + uri, 2 );
  // Update URI with authentication information
  if ( !configId.isEmpty() )
  {
    if ( expandAuthConfig )
    {
      QStringList connectionItems;
      connectionItems << uri;
      if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, QStringLiteral( "ogr" ) ) )
      {
        uri = connectionItems.join( QString() );
      }
    }
    else
    {
      uri += QStringLiteral( " authcfg='%1'" ).arg( configId );
    }
  }
  else if ( !( username.isEmpty() || password.isEmpty() ) )
  {
    uri.replace( QLatin1String( "://" ), QStringLiteral( "://%1:%2@" ).arg( username, password ) );
  }
  return uri;
}

QWidget *QgsGdalGuiUtils::createWidgetForOption( const QgsGdalOption &option, QWidget *parent, bool includeDefaultChoices )
{
  switch ( option.type )
  {
    case QgsGdalOption::Type::Select:
    {
      QComboBox *cb = new QComboBox( parent );
      if ( includeDefaultChoices )
      {
        cb->addItem( QObject::tr( "<Default>" ), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      }
      for ( const QString &val : std::as_const( option.options ) )
      {
        cb->addItem( val, val );
      }
      cb->setCurrentIndex( 0 );
      cb->setToolTip( option.description );
      return cb;
    }

    case QgsGdalOption::Type::Boolean:
    {
      QComboBox *cb = new QComboBox( parent );
      if ( includeDefaultChoices )
      {
        cb->addItem( QObject::tr( "<Default>" ), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
      }
      cb->addItem( QObject::tr( "Yes" ), "YES" );
      cb->addItem( QObject::tr( "No" ), "NO" );
      cb->setCurrentIndex( 0 );
      cb->setToolTip( option.description );
      return cb;
    }

    case QgsGdalOption::Type::Text:
    {
      QgsFilterLineEdit *res = new QgsFilterLineEdit( parent );
      res->setToolTip( option.description );
      res->setShowClearButton( true );
      if ( includeDefaultChoices )
      {
        res->setPlaceholderText( QObject::tr( "Default" ) );
      }
      return res;
    }

    case QgsGdalOption::Type::Int:
    {
      QgsSpinBox *res = new QgsSpinBox( parent );
      res->setToolTip( option.description );
      if ( option.minimum.isValid() )
        res->setMinimum( option.minimum.toInt() );
      else
        res->setMinimum( 0 );
      if ( option.maximum.isValid() )
        res->setMaximum( option.maximum.toInt() );
      else
        res->setMaximum( std::numeric_limits<int>::max() - 1 );
      if ( includeDefaultChoices )
      {
        res->setMinimum( res->minimum() - 1 );
        res->setClearValueMode( QgsSpinBox::ClearValueMode::MinimumValue, QObject::tr( "Default" ) );
      }
      else if ( option.defaultValue.isValid() )
      {
        res->setClearValue( option.defaultValue.toInt() );
      }
      res->clear();
      return res;
    }

    case QgsGdalOption::Type::Double:
    {
      QgsDoubleSpinBox *res = new QgsDoubleSpinBox( parent );
      res->setToolTip( option.description );
      if ( option.minimum.isValid() )
        res->setMinimum( option.minimum.toDouble() );
      else
        res->setMinimum( 0 );
      if ( option.maximum.isValid() )
        res->setMaximum( option.maximum.toDouble() );
      else
        res->setMaximum( std::numeric_limits<double>::max() - 1 );
      if ( option.defaultValue.isValid() )
        res->setClearValue( option.defaultValue.toDouble() );
      if ( includeDefaultChoices )
      {
        res->setMinimum( res->minimum() - 1 );
        res->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, QObject::tr( "Default" ) );
      }
      else if ( option.defaultValue.isValid() )
      {
        res->setClearValue( option.defaultValue.toDouble() );
      }
      res->clear();
      return res;
    }

    case QgsGdalOption::Type::Invalid:
      break;
  }
  return nullptr;
}
