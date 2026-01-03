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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsdoublespinbox.h"
#include "qgsfilterlineedit.h"
#include "qgsgdalutils.h"
#include "qgslogger.h"
#include "qgsspinbox.h"

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
  if ( connectionType == "ESRI Personal GeoDatabase"_L1 )
  {
    uri = "PGeo:" + database;
  }
  else if ( connectionType == "ESRI ArcSDE"_L1 )
  {
    if ( port.isEmpty() )
      port = u"5151"_s;

    uri = "SDE:" + host + ",PORT:" + port + ',' + database + ',' + username + ',' + password;
  }
  else if ( connectionType == "Informix DataBlade"_L1 )
  {
    //not tested
    uri = "IDB:dbname=" + database;

    if ( !host.isEmpty() )
      uri += u" server=%1"_s.arg( host );

    if ( !username.isEmpty() )
    {
      uri += u" user=%1"_s.arg( username );

      if ( !password.isEmpty() )
        uri += u" pass=%1"_s.arg( password );
    }
  }
  else if ( connectionType == "Ingres"_L1 )
  {
    //not tested
    uri = "@driver=ingres,dbname=" + database;
    if ( !username.isEmpty() )
    {
      uri += u",userid=%1"_s.arg( username );

      if ( !password.isEmpty() )
        uri += u",password=%1"_s.arg( password );
    }
  }
  else if ( connectionType == "MySQL"_L1 )
  {
    uri = "MySQL:" + database;

    if ( !host.isEmpty() )
    {
      uri += u",host=%1"_s.arg( host );

      if ( !port.isEmpty() )
        uri += u",port=%1"_s.arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += u",user=%1"_s.arg( username );

      if ( !password.isEmpty() )
        uri += u",password=%1"_s.arg( password );
    }
  }
  else if ( connectionType == "MSSQL"_L1 )
  {
    uri = u"MSSQL:"_s;

    if ( !host.isEmpty() )
    {
      uri += u";server=%1"_s.arg( host );

      if ( !port.isEmpty() )
        uri += u",%1"_s.arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += u";uid=%1"_s.arg( username );

      if ( !password.isEmpty() )
        uri += u";pwd=%1"_s.arg( password );
    }
    else
      uri += ";trusted_connection=yes"_L1;

    if ( !database.isEmpty() )
      uri += u";database=%1"_s.arg( database );
  }
  else if ( connectionType == "Oracle Spatial"_L1 )
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
  else if ( connectionType == "ODBC"_L1 )
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
  else if ( connectionType == "OGDI Vectors"_L1 )
  {
  }
  else if ( connectionType == "PostgreSQL"_L1 )
  {
    uri = "PG:dbname='" + database + '\'';

    if ( !host.isEmpty() )
    {
      uri += u" host='%1'"_s.arg( host );

      if ( !port.isEmpty() )
        uri += u" port='%1'"_s.arg( port );
    }

    if ( !username.isEmpty() )
    {
      uri += u" user='%1'"_s.arg( username );

      if ( !password.isEmpty() )
        uri += u" password='%1'"_s.arg( password );
    }

    uri += ' ';
  }
  // Append authentication configuration to the URI
  if ( !( configId.isEmpty() ) )
  {
    if ( !expandAuthConfig )
    {
      uri += u" authcfg='%1'"_s.arg( configId );
    }
    else
    {
      QStringList connectionItems;
      connectionItems << uri;
      if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, u"ogr"_s ) )
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
  QString uri = url;
  QString prefix;
  if ( type == "vsicurl"_L1 )
  {
    prefix = u"/vsicurl/"_s;
    if ( !uri.startsWith( prefix ) )
    {
      // If no protocol is provided in the URL, default to HTTP
      if ( !uri.startsWith( "http://"_L1 ) && !uri.startsWith( "https://"_L1 ) && !uri.startsWith( "ftp://"_L1 ) )
      {
        uri.prepend( u"http://"_s );
      }
      uri.prepend( prefix );
    }
  }
  else if ( type == "vsis3"_L1
            || type == "vsigs"_L1
            || type == "vsiaz"_L1
            || type == "vsiadls"_L1
            || type == "vsioss"_L1
            || type == "vsiswift"_L1
            || type == "vsihdfs"_L1 )
  {
    prefix = u"/%1/"_s.arg( type );
    if ( !uri.startsWith( prefix ) )
    {
      uri.prepend( prefix );
    }
  }
  // catching both GeoJSON and GeoJSONSeq
  else if ( type.startsWith( "GeoJSON"_L1 ) )
  {
    // no change needed for now
  }
  else if ( type == "CouchDB"_L1 )
  {
    prefix = u"couchdb:"_s;
    if ( !uri.startsWith( prefix ) )
    {
      uri.prepend( prefix );
    }
  }
  else if ( type == "DODS/OPeNDAP"_L1 )
  {
    prefix = u"DODS:"_s;
    if ( !uri.startsWith( prefix ) )
    {
      uri.prepend( prefix );
    }
  }
  else if ( type == "WFS3"_L1 )
  {
    prefix = u"WFS3:"_s;
    if ( !uri.startsWith( prefix ) )
    {
      uri.prepend( prefix );
    }
  }
  QgsDebugMsgLevel( "Connection type is=" + type + " and uri=" + uri, 2 );
  // Update URI with authentication information
  if ( !configId.isEmpty() )
  {
    if ( expandAuthConfig )
    {
      QStringList connectionItems;
      connectionItems << uri;
      if ( QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, configId, u"ogr"_s ) )
      {
        uri = connectionItems.join( QString() );
      }
    }
    else
    {
      uri += u" authcfg='%1'"_s.arg( configId );
    }
  }
  else if ( !( username.isEmpty() || password.isEmpty() ) )
  {
    uri.replace( "://"_L1, u"://%1:%2@"_s.arg( username, password ) );
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
