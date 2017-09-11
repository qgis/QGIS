/***************************************************************************
    qgsmanageconnectionsdialog.cpp
    ---------------------
    begin                : Dec 2009
    copyright            : (C) 2009 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>

#include "qgssettings.h"
#include "qgsmanageconnectionsdialog.h"


QgsManageConnectionsDialog::QgsManageConnectionsDialog( QWidget *parent, Mode mode, Type type, const QString &fileName )
  : QDialog( parent )
  , mFileName( fileName )
  , mDialogMode( mode )
  , mConnectionType( type )
{
  setupUi( this );

  // additional buttons
  QPushButton *pb = nullptr;
  pb = new QPushButton( tr( "Select all" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, &QAbstractButton::clicked, this, &QgsManageConnectionsDialog::selectAll );

  pb = new QPushButton( tr( "Clear selection" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, &QAbstractButton::clicked, this, &QgsManageConnectionsDialog::clearSelection );

  if ( mDialogMode == Import )
  {
    label->setText( tr( "Select connections to import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  }
  else
  {
    //label->setText( tr( "Select connections to export" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Export" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  }

  if ( !populateConnections() )
  {
    QApplication::postEvent( this, new QCloseEvent() );
  }

  // use OK button for starting import and export operations
  disconnect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsManageConnectionsDialog::doExportImport );

  connect( listConnections, &QListWidget::itemSelectionChanged, this, &QgsManageConnectionsDialog::selectionChanged );
}

void QgsManageConnectionsDialog::selectionChanged()
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !listConnections->selectedItems().isEmpty() );
}

void QgsManageConnectionsDialog::doExportImport()
{
  QList<QListWidgetItem *> selection = listConnections->selectedItems();
  if ( selection.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Export/import error" ),
                          tr( "You should select at least one connection from list." ) );
    return;
  }

  QStringList items;
  items.reserve( selection.size() );
  for ( int i = 0; i < selection.size(); ++i )
  {
    items.append( selection.at( i )->text() );
  }

  if ( mDialogMode == Export )
  {
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save connections" ), QDir::homePath(),
                       tr( "XML files (*.xml *.XML)" ) );
    if ( fileName.isEmpty() )
    {
      return;
    }

    // ensure the user never omitted the extension from the file name
    if ( !fileName.endsWith( QLatin1String( ".xml" ), Qt::CaseInsensitive ) )
    {
      fileName += QLatin1String( ".xml" );
    }

    mFileName = fileName;

    QDomDocument doc;
    switch ( mConnectionType )
    {
      case WMS:
        doc = saveOWSConnections( items, QStringLiteral( "WMS" ) );
        break;
      case WFS:
        doc = saveWfsConnections( items );
        break;
      case PostGIS:
        doc = savePgConnections( items );
        break;
      case MSSQL:
        doc = saveMssqlConnections( items );
        break;
      case WCS:
        doc = saveOWSConnections( items, QStringLiteral( "WCS" ) );
        break;
      case Oracle:
        doc = saveOracleConnections( items );
        break;
      case DB2:
        doc = saveDb2Connections( items );
        break;
      case GeoNode:
        doc = saveGeonodeConnections( items );
        break;
    }

    QFile file( mFileName );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      QMessageBox::warning( this, tr( "Saving connections" ),
                            tr( "Cannot write file %1:\n%2." )
                            .arg( mFileName,
                                  file.errorString() ) );
      return;
    }

    QTextStream out( &file );
    doc.save( out, 4 );
  }
  else // import connections
  {
    QFile file( mFileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QMessageBox::warning( this, tr( "Loading connections" ),
                            tr( "Cannot read file %1:\n%2." )
                            .arg( mFileName,
                                  file.errorString() ) );
      return;
    }

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
      QMessageBox::warning( this, tr( "Loading connections" ),
                            tr( "Parse error at line %1, column %2:\n%3" )
                            .arg( errorLine )
                            .arg( errorColumn )
                            .arg( errorStr ) );
      return;
    }

    switch ( mConnectionType )
    {
      case WMS:
        loadOWSConnections( doc, items, QStringLiteral( "WMS" ) );
        break;
      case WFS:
        loadWfsConnections( doc, items );
        break;
      case PostGIS:
        loadPgConnections( doc, items );
        break;
      case MSSQL:
        loadMssqlConnections( doc, items );
        break;
      case WCS:
        loadOWSConnections( doc, items, QStringLiteral( "WCS" ) );
        break;
      case Oracle:
        loadOracleConnections( doc, items );
        break;
      case DB2:
        loadDb2Connections( doc, items );
        break;
      case GeoNode:
        loadGeonodeConnections( doc, items );
        break;
    }
    // clear connections list and close window
    listConnections->clear();
    accept();
  }

  mFileName = QLatin1String( "" );
}

bool QgsManageConnectionsDialog::populateConnections()
{
  // Export mode. Populate connections list from settings
  if ( mDialogMode == Export )
  {
    QgsSettings settings;
    switch ( mConnectionType )
    {
      case WMS:
        settings.beginGroup( QStringLiteral( "/qgis/connections-wms" ) );
        break;
      case WFS:
        settings.beginGroup( QStringLiteral( "/qgis/connections-wfs" ) );
        break;
      case WCS:
        settings.beginGroup( QStringLiteral( "/qgis/connections-wcs" ) );
        break;
      case PostGIS:
        settings.beginGroup( QStringLiteral( "/PostgreSQL/connections" ) );
        break;
      case MSSQL:
        settings.beginGroup( QStringLiteral( "/MSSQL/connections" ) );
        break;
      case Oracle:
        settings.beginGroup( QStringLiteral( "/Oracle/connections" ) );
        break;
      case DB2:
        settings.beginGroup( QStringLiteral( "/DB2/connections" ) );
        break;
      case GeoNode:
        settings.beginGroup( QStringLiteral( "/qgis/connections-geonode" ) );
        break;
    }
    QStringList keys = settings.childGroups();
    QStringList::Iterator it = keys.begin();
    while ( it != keys.end() )
    {
      QListWidgetItem *item = new QListWidgetItem();
      item->setText( *it );
      listConnections->addItem( item );
      ++it;
    }
    settings.endGroup();
  }
  // Import mode. Populate connections list from file
  else
  {
    QFile file( mFileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QMessageBox::warning( this, tr( "Loading connections" ),
                            tr( "Cannot read file %1:\n%2." )
                            .arg( mFileName,
                                  file.errorString() ) );
      return false;
    }

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
      QMessageBox::warning( this, tr( "Loading connections" ),
                            tr( "Parse error at line %1, column %2:\n%3" )
                            .arg( errorLine )
                            .arg( errorColumn )
                            .arg( errorStr ) );
      return false;
    }

    QDomElement root = doc.documentElement();
    switch ( mConnectionType )
    {
      case WMS:
        if ( root.tagName() != QLatin1String( "qgsWMSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a WMS connections exchange file." ) );
          return false;
        }
        break;

      case WFS:
        if ( root.tagName() != QLatin1String( "qgsWFSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a WFS connections exchange file." ) );
          return false;
        }
        break;

      case WCS:
        if ( root.tagName() != QLatin1String( "qgsWCSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a WCS connections exchange file." ) );
          return false;
        }
        break;

      case PostGIS:
        if ( root.tagName() != QLatin1String( "qgsPgConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a PostGIS connections exchange file." ) );
          return false;
        }
        break;

      case MSSQL:
        if ( root.tagName() != QLatin1String( "qgsMssqlConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a MSSQL connections exchange file." ) );
          return false;
        }
        break;
      case Oracle:
        if ( root.tagName() != QLatin1String( "qgsOracleConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not an Oracle connections exchange file." ) );
          return false;
        }
        break;
      case DB2:
        if ( root.tagName() != QLatin1String( "qgsDb2Connections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a DB2 connections exchange file." ) );
          return false;
        }
        break;
      case GeoNode:
        if ( root.tagName() != QLatin1String( "qgsGeoNodeConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not a GeoNode connections exchange file." ) );
          return false;
        }
        break;
    }

    QDomElement child = root.firstChildElement();
    while ( !child.isNull() )
    {
      QListWidgetItem *item = new QListWidgetItem();
      item->setText( child.attribute( QStringLiteral( "name" ) ) );
      listConnections->addItem( item );
      child = child.nextSiblingElement();
    }
  }
  return true;
}

QDomDocument QgsManageConnectionsDialog::saveOWSConnections( const QStringList &connections, const QString &service )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( "qgs" + service.toUpper() + "Connections" );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/qgis/connections-" + service.toLower() + '/';
    QDomElement el = doc.createElement( service.toLower() );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "url" ), settings.value( path + connections[ i ] + "/url", "" ).toString() );

    if ( service == QLatin1String( "WMS" ) )
    {
      el.setAttribute( QStringLiteral( "ignoreGetMapURI" ), settings.value( path + connections[i] + "/ignoreGetMapURI", false ).toBool() ? "true" : "false" );
      el.setAttribute( QStringLiteral( "ignoreGetFeatureInfoURI" ), settings.value( path + connections[i] + "/ignoreGetFeatureInfoURI", false ).toBool() ? "true" : "false" );
      el.setAttribute( QStringLiteral( "ignoreAxisOrientation" ), settings.value( path + connections[i] + "/ignoreAxisOrientation", false ).toBool() ? "true" : "false" );
      el.setAttribute( QStringLiteral( "invertAxisOrientation" ), settings.value( path + connections[i] + "/invertAxisOrientation", false ).toBool() ? "true" : "false" );
      el.setAttribute( QStringLiteral( "referer" ), settings.value( path + connections[ i ] + "/referer", "" ).toString() );
      el.setAttribute( QStringLiteral( "smoothPixmapTransform" ), settings.value( path + connections[i] + "/smoothPixmapTransform", false ).toBool() ? "true" : "false" );
      el.setAttribute( QStringLiteral( "dpiMode" ), settings.value( path + connections[i] + "/dpiMode", "7" ).toInt() );
    }

    path = "/qgis/" + service.toUpper() + '/';
    el.setAttribute( QStringLiteral( "username" ), settings.value( path + connections[ i ] + "/username", "" ).toString() );
    el.setAttribute( QStringLiteral( "password" ), settings.value( path + connections[ i ] + "/password", "" ).toString() );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveWfsConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsWFSConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = QStringLiteral( "/qgis/connections-wfs/" );
    QDomElement el = doc.createElement( QStringLiteral( "wfs" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "url" ), settings.value( path + connections[ i ] + "/url", "" ).toString() );

    el.setAttribute( QStringLiteral( "referer" ), settings.value( path + connections[ i ] + "/referer", "" ).toString() );

    path = QStringLiteral( "/qgis/WFS/" );
    el.setAttribute( QStringLiteral( "username" ), settings.value( path + connections[ i ] + "/username", "" ).toString() );
    el.setAttribute( QStringLiteral( "password" ), settings.value( path + connections[ i ] + "/password", "" ).toString() );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::savePgConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsPgConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/PostgreSQL/connections/" + connections[ i ];
    QDomElement el = doc.createElement( QStringLiteral( "postgis" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host", "" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port", "" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database", "" ).toString() );
    el.setAttribute( QStringLiteral( "service" ), settings.value( path + "/service", "" ).toString() );
    el.setAttribute( QStringLiteral( "sslmode" ), settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveMssqlConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsMssqlConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/MSSQL/connections/" + connections[ i ];
    QDomElement el = doc.createElement( QStringLiteral( "mssql" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host", "" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port", "" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database", "" ).toString() );
    el.setAttribute( QStringLiteral( "service" ), settings.value( path + "/service", "" ).toString() );
    el.setAttribute( QStringLiteral( "sslmode" ), settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveOracleConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsOracleConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/Oracle/connections/" + connections[ i ];
    QDomElement el = doc.createElement( QStringLiteral( "oracle" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host", "" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port", "" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database", "" ).toString() );
    el.setAttribute( QStringLiteral( "dboptions" ), settings.value( path + "/dboptions", "" ).toString() );
    el.setAttribute( QStringLiteral( "dbworkspace" ), settings.value( path + "/dbworkspace", "" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );
    el.setAttribute( QStringLiteral( "userTablesOnly" ), settings.value( path + "/userTablesOnly", "0" ).toString() );
    el.setAttribute( QStringLiteral( "geometryColumnsOnly" ), settings.value( path + "/geometryColumnsOnly", "0" ).toString() );
    el.setAttribute( QStringLiteral( "allowGeometrylessTables" ), settings.value( path + "/allowGeometrylessTables", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveDb2Connections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsDb2Connections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/DB2/connections/" + connections[ i ];
    QDomElement el = doc.createElement( QStringLiteral( "db2" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host", "" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port", "" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database", "" ).toString() );
    el.setAttribute( QStringLiteral( "service" ), settings.value( path + "/service", "" ).toString() );
    el.setAttribute( QStringLiteral( "sslmode" ), settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveGeonodeConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsGeoNodeConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = QStringLiteral( "/qgis/connections-geonode/" );
    QDomElement el = doc.createElement( QStringLiteral( "geonode" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[ i ] );
    el.setAttribute( QStringLiteral( "url" ), settings.value( path + connections[ i ] + "/url", "" ).toString() );

    path = QStringLiteral( "/qgis/GeoNode/" );
    el.setAttribute( QStringLiteral( "username" ), settings.value( path + connections[ i ] + "/username", "" ).toString() );
    el.setAttribute( QStringLiteral( "password" ), settings.value( path + connections[ i ] + "/password", "" ).toString() );
    root.appendChild( el );
  }

  return doc;
}

void QgsManageConnectionsDialog::loadOWSConnections( const QDomDocument &doc, const QStringList &items, const QString &service )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgs" + service.toUpper() + "Connections" )
  {
    QMessageBox::information( this, tr( "Loading connections" ),
                              tr( "The file is not a %1 connections exchange file." ).arg( service ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( "/qgis/connections-" + service.toLower() );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // no dups detected or overwrite is allowed
    settings.beginGroup( "/qgis/connections-" + service.toLower() );
    settings.setValue( QString( '/' + connectionName + "/url" ), child.attribute( QStringLiteral( "url" ) ) );
    settings.setValue( QString( '/' + connectionName + "/ignoreGetMapURI" ), child.attribute( QStringLiteral( "ignoreGetMapURI" ) ) == QLatin1String( "true" ) );
    settings.setValue( QString( '/' + connectionName + "/ignoreGetFeatureInfoURI" ), child.attribute( QStringLiteral( "ignoreGetFeatureInfoURI" ) ) == QLatin1String( "true" ) );
    settings.setValue( QString( '/' + connectionName + "/ignoreAxisOrientation" ), child.attribute( QStringLiteral( "ignoreAxisOrientation" ) ) == QLatin1String( "true" ) );
    settings.setValue( QString( '/' + connectionName + "/invertAxisOrientation" ), child.attribute( QStringLiteral( "invertAxisOrientation" ) ) == QLatin1String( "true" ) );
    settings.setValue( QString( '/' + connectionName + "/referer" ), child.attribute( QStringLiteral( "referer" ) ) );
    settings.setValue( QString( '/' + connectionName + "/smoothPixmapTransform" ), child.attribute( QStringLiteral( "smoothPixmapTransform" ) ) == QLatin1String( "true" ) );
    settings.setValue( QString( '/' + connectionName + "/dpiMode" ), child.attribute( QStringLiteral( "dpiMode" ), QStringLiteral( "7" ) ).toInt() );
    settings.endGroup();

    if ( !child.attribute( QStringLiteral( "username" ) ).isEmpty() )
    {
      settings.beginGroup( "/qgis/" + service.toUpper() + '/' + connectionName );
      settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
      settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
      settings.endGroup();
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadWfsConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsWFSConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading connections" ),
                              tr( "The file is not a WFS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/qgis/connections-wfs" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // no dups detected or overwrite is allowed
    settings.beginGroup( QStringLiteral( "/qgis/connections-wfs" ) );
    settings.setValue( QString( '/' + connectionName + "/url" ), child.attribute( QStringLiteral( "url" ) ) );
    settings.endGroup();

    if ( !child.attribute( QStringLiteral( "username" ) ).isEmpty() )
    {
      settings.beginGroup( "/qgis/WFS/" + connectionName );
      settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
      settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
      settings.endGroup();
    }
    child = child.nextSiblingElement();
  }
}


void QgsManageConnectionsDialog::loadPgConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsPgConnections" ) )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not a PostGIS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/PostgreSQL/connections" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/PostgreSQL/connections/" + connectionName );

    settings.setValue( QStringLiteral( "/host" ), child.attribute( QStringLiteral( "host" ) ) );
    settings.setValue( QStringLiteral( "/port" ), child.attribute( QStringLiteral( "port" ) ) );
    settings.setValue( QStringLiteral( "/database" ), child.attribute( QStringLiteral( "database" ) ) );
    if ( child.hasAttribute( QStringLiteral( "service" ) ) )
    {
      settings.setValue( QStringLiteral( "/service" ), child.attribute( QStringLiteral( "service" ) ) );
    }
    else
    {
      settings.setValue( QStringLiteral( "/service" ), "" );
    }
    settings.setValue( QStringLiteral( "/sslmode" ), child.attribute( QStringLiteral( "sslmode" ) ) );
    settings.setValue( QStringLiteral( "/estimatedMetadata" ), child.attribute( QStringLiteral( "estimatedMetadata" ) ) );
    settings.setValue( QStringLiteral( "/saveUsername" ), child.attribute( QStringLiteral( "saveUsername" ) ) );
    settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
    settings.setValue( QStringLiteral( "/savePassword" ), child.attribute( QStringLiteral( "savePassword" ) ) );
    settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadMssqlConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsMssqlConnections" ) )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not a MSSQL connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/MSSQL/connections" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/MSSQL/connections/" + connectionName );

    settings.setValue( QStringLiteral( "/host" ), child.attribute( QStringLiteral( "host" ) ) );
    settings.setValue( QStringLiteral( "/port" ), child.attribute( QStringLiteral( "port" ) ) );
    settings.setValue( QStringLiteral( "/database" ), child.attribute( QStringLiteral( "database" ) ) );
    if ( child.hasAttribute( QStringLiteral( "service" ) ) )
    {
      settings.setValue( QStringLiteral( "/service" ), child.attribute( QStringLiteral( "service" ) ) );
    }
    else
    {
      settings.setValue( QStringLiteral( "/service" ), "" );
    }
    settings.setValue( QStringLiteral( "/sslmode" ), child.attribute( QStringLiteral( "sslmode" ) ) );
    settings.setValue( QStringLiteral( "/estimatedMetadata" ), child.attribute( QStringLiteral( "estimatedMetadata" ) ) );
    settings.setValue( QStringLiteral( "/saveUsername" ), child.attribute( QStringLiteral( "saveUsername" ) ) );
    settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
    settings.setValue( QStringLiteral( "/savePassword" ), child.attribute( QStringLiteral( "savePassword" ) ) );
    settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadOracleConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsOracleConnections" ) )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not an Oracle connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/Oracle/connections" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/Oracle/connections/" + connectionName );

    settings.setValue( QStringLiteral( "/host" ), child.attribute( QStringLiteral( "host" ) ) );
    settings.setValue( QStringLiteral( "/port" ), child.attribute( QStringLiteral( "port" ) ) );
    settings.setValue( QStringLiteral( "/database" ), child.attribute( QStringLiteral( "database" ) ) );
    settings.setValue( QStringLiteral( "/dboptions" ), child.attribute( QStringLiteral( "dboptions" ) ) );
    settings.setValue( QStringLiteral( "/dbworkspace" ), child.attribute( QStringLiteral( "dbworkspace" ) ) );
    settings.setValue( QStringLiteral( "/estimatedMetadata" ), child.attribute( QStringLiteral( "estimatedMetadata" ) ) );
    settings.setValue( QStringLiteral( "/userTablesOnly" ), child.attribute( QStringLiteral( "userTablesOnly" ) ) );
    settings.setValue( QStringLiteral( "/geometryColumnsOnly" ), child.attribute( QStringLiteral( "geometryColumnsOnly" ) ) );
    settings.setValue( QStringLiteral( "/allowGeometrylessTables" ), child.attribute( QStringLiteral( "allowGeometrylessTables" ) ) );
    settings.setValue( QStringLiteral( "/saveUsername" ), child.attribute( QStringLiteral( "saveUsername" ) ) );
    settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
    settings.setValue( QStringLiteral( "/savePassword" ), child.attribute( QStringLiteral( "savePassword" ) ) );
    settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadDb2Connections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsDb2Connections" ) )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not a DB2 connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/DB2/connections" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/DB2/connections/" + connectionName );

    settings.setValue( QStringLiteral( "/host" ), child.attribute( QStringLiteral( "host" ) ) );
    settings.setValue( QStringLiteral( "/port" ), child.attribute( QStringLiteral( "port" ) ) );
    settings.setValue( QStringLiteral( "/database" ), child.attribute( QStringLiteral( "database" ) ) );
    if ( child.hasAttribute( QStringLiteral( "service" ) ) )
    {
      settings.setValue( QStringLiteral( "/service" ), child.attribute( QStringLiteral( "service" ) ) );
    }
    else
    {
      settings.setValue( QStringLiteral( "/service" ), "" );
    }
    settings.setValue( QStringLiteral( "/sslmode" ), child.attribute( QStringLiteral( "sslmode" ) ) );
    settings.setValue( QStringLiteral( "/estimatedMetadata" ), child.attribute( QStringLiteral( "estimatedMetadata" ) ) );
    settings.setValue( QStringLiteral( "/saveUsername" ), child.attribute( QStringLiteral( "saveUsername" ) ) );
    settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
    settings.setValue( QStringLiteral( "/savePassword" ), child.attribute( QStringLiteral( "savePassword" ) ) );
    settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadGeonodeConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsGeoNodeConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading connections" ),
                              tr( "The file is not a GeoNode connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/qgis/connections-geonode" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name '%1' already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

      switch ( res )
      {
        case QMessageBox::Cancel:
          return;
        case QMessageBox::No:
          child = child.nextSiblingElement();
          continue;
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
      }
    }

    if ( keys.contains( connectionName ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // no dups detected or overwrite is allowed
    settings.beginGroup( QStringLiteral( "/qgis/connections-geonode" ) );
    settings.setValue( QString( '/' + connectionName + "/url" ), child.attribute( QStringLiteral( "url" ) ) );
    settings.endGroup();

    if ( !child.attribute( QStringLiteral( "username" ) ).isEmpty() )
    {
      settings.beginGroup( "/qgis/GeoNode/" + connectionName );
      settings.setValue( QStringLiteral( "/username" ), child.attribute( QStringLiteral( "username" ) ) );
      settings.setValue( QStringLiteral( "/password" ), child.attribute( QStringLiteral( "password" ) ) );
      settings.endGroup();
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::selectAll()
{
  listConnections->selectAll();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !listConnections->selectedItems().isEmpty() );
}

void QgsManageConnectionsDialog::clearSelection()
{
  listConnections->clearSelection();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}
