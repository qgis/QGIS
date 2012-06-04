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
#include <QSettings>
#include <QTextStream>

#include "qgsmanageconnectionsdialog.h"

QgsManageConnectionsDialog::QgsManageConnectionsDialog( QWidget *parent, Mode mode, Type type, QString fileName )
    : QDialog( parent )
    , mFileName( fileName )
    , mDialogMode( mode )
    , mConnectionType( type )
{
  setupUi( this );

  // additional buttons
  QPushButton *pb;
  pb = new QPushButton( tr( "Select all" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( selectAll() ) );

  pb = new QPushButton( tr( "Clear selection" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( clearSelection() ) );

  if ( mDialogMode == Import )
  {
    label->setText( tr( "Select connections to import" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Import" ) );
  }
  else
  {
    //label->setText( tr( "Select connections to export" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Export" ) );
  }

  if ( !populateConnections() )
  {
    QApplication::postEvent( this, new QCloseEvent() );
  }

  // use Ok button for starting import and export operations
  disconnect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( doExportImport() ) );
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
  for ( int i = 0; i < selection.size(); ++i )
  {
    items.append( selection.at( i )->text() );
  }

  if ( mDialogMode == Export )
  {
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save connections" ), ".",
                       tr( "XML files (*.xml *.XML)" ) );
    if ( fileName.isEmpty() )
    {
      return;
    }

    // ensure the user never ommited the extension from the file name
    if ( !fileName.toLower().endsWith( ".xml" ) )
    {
      fileName += ".xml";
    }

    mFileName = fileName;

    QDomDocument doc;
    switch ( mConnectionType )
    {
      case WMS:
        doc = saveWMSConnections( items );
        break;
      case WFS:
        doc = saveWFSConnections( items );
        break;
      case PostGIS:
        doc = savePgConnections( items );
        break;
      case MSSQL:
        doc = saveMssqlConnections( items );
        break;
    }

    QFile file( mFileName );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QMessageBox::warning( this, tr( "Saving connections" ),
                            tr( "Cannot write file %1:\n%2." )
                            .arg( mFileName )
                            .arg( file.errorString() ) );
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
                            .arg( mFileName )
                            .arg( file.errorString() ) );
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
        loadWMSConnections( doc, items );
        break;
      case WFS:
        loadWFSConnections( doc, items );
        break;
      case PostGIS:
        loadPgConnections( doc, items );
        break;
      case MSSQL:
        loadMssqlConnections( doc, items );
        break;
    }
    // clear connections list and close window
    listConnections->clear();
    accept();
  }

  mFileName = "";
}

bool QgsManageConnectionsDialog::populateConnections()
{
  // Export mode. Populate connections list from settings
  if ( mDialogMode == Export )
  {
    QSettings settings;
    switch ( mConnectionType )
    {
      case WMS:
        settings.beginGroup( "/Qgis/connections-wms" );
        break;
      case WFS:
        settings.beginGroup( "/Qgis/connections-wfs" );
        break;
      case PostGIS:
        settings.beginGroup( "/PostgreSQL/connections" );
        break;
      case MSSQL:
        settings.beginGroup( "/MSSQL/connections" );
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
                            .arg( mFileName )
                            .arg( file.errorString() ) );
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
        if ( root.tagName() != "qgsWMSConnections" )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not an WMS connections exchange file." ) );
          return false;
        }
        break;

      case WFS:
        if ( root.tagName() != "qgsWFSConnections" )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not an WFS connections exchange file." ) );
          return false;
        }
        break;

      case PostGIS:
        if ( root.tagName() != "qgsPgConnections" )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not an PostGIS connections exchange file." ) );
          return false;
        }
        break;

      case MSSQL:
        if ( root.tagName() != "qgsMssqlConnections" )
        {
          QMessageBox::information( this, tr( "Loading connections" ),
                                    tr( "The file is not an MSSQL connections exchange file." ) );
          return false;
        }
        break;
    }

    QDomElement child = root.firstChildElement();
    while ( !child.isNull() )
    {
      QListWidgetItem *item = new QListWidgetItem();
      item->setText( child.attribute( "name" ) );
      listConnections->addItem( item );
      child = child.nextSiblingElement();
    }
  }
  return true;
}

QDomDocument QgsManageConnectionsDialog::saveWMSConnections( const QStringList &connections )
{
  QDomDocument doc( "connections" );
  QDomElement root = doc.createElement( "qgsWMSConnections" );
  root.setAttribute( "version", "1.0" );
  doc.appendChild( root );

  QSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/Qgis/connections-wms/";
    QDomElement el = doc.createElement( "wms" );
    el.setAttribute( "name", connections[ i ] );
    el.setAttribute( "url", settings.value( path + connections[ i ] + "/url", "" ).toString() );
    el.setAttribute( "ignoreGetMapURI", settings.value( path + connections[i] + "/ignoreGetMapURI", false ).toBool() ? "true" : "false" );
    el.setAttribute( "ignoreGetFeatureInfoURI", settings.value( path + connections[i] + "/ignoreGetFeatureInfoURI", false ).toBool() ? "true" : "false" );
    el.setAttribute( "ignoreAxisOrientation", settings.value( path + connections[i] + "/ignoreAxisOrientation", false ).toBool() ? "true" : "false" );
    el.setAttribute( "invertAxisOrientation", settings.value( path + connections[i] + "/invertAxisOrientation", false ).toBool() ? "true" : "false" );

    path = "/Qgis/WMS/";
    el.setAttribute( "username", settings.value( path + connections[ i ] + "/username", "" ).toString() );
    el.setAttribute( "password", settings.value( path + connections[ i ] + "/password", "" ).toString() );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveWFSConnections( const QStringList &connections )
{
  QDomDocument doc( "connections" );
  QDomElement root = doc.createElement( "qgsWFSConnections" );
  root.setAttribute( "version", "1.0" );
  doc.appendChild( root );

  QSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/Qgis/connections-wfs/";
    QDomElement el = doc.createElement( "wfs" );
    el.setAttribute( "name", connections[ i ] );
    el.setAttribute( "url", settings.value( path + connections[ i ] + "/url", "" ).toString() );

    path = "/Qgis/WFS/";
    el.setAttribute( "username", settings.value( path + connections[ i ] + "/username", "" ).toString() );
    el.setAttribute( "password", settings.value( path + connections[ i ] + "/password", "" ).toString() );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::savePgConnections( const QStringList &connections )
{
  QDomDocument doc( "connections" );
  QDomElement root = doc.createElement( "qgsPgConnections" );
  root.setAttribute( "version", "1.0" );
  doc.appendChild( root );

  QSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/PostgreSQL/connections/" + connections[ i ];
    QDomElement el = doc.createElement( "postgis" );
    el.setAttribute( "name", connections[ i ] );
    el.setAttribute( "host", settings.value( path + "/host", "" ).toString() );
    el.setAttribute( "port", settings.value( path + "/port", "" ).toString() );
    el.setAttribute( "database", settings.value( path + "/database", "" ).toString() );
    el.setAttribute( "service", settings.value( path + "/service", "" ).toString() );
    el.setAttribute( "sslmode", settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( "estimatedMetadata", settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( "saveUsername", settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true" )
    {
      el.setAttribute( "username", settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( "savePassword", settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == "true" )
    {
      el.setAttribute( "password", settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveMssqlConnections( const QStringList &connections )
{
  QDomDocument doc( "connections" );
  QDomElement root = doc.createElement( "qgsMssqlConnections" );
  root.setAttribute( "version", "1.0" );
  doc.appendChild( root );

  QSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/MSSQL/connections/" + connections[ i ];
    QDomElement el = doc.createElement( "mssql" );
    el.setAttribute( "name", connections[ i ] );
    el.setAttribute( "host", settings.value( path + "/host", "" ).toString() );
    el.setAttribute( "port", settings.value( path + "/port", "" ).toString() );
    el.setAttribute( "database", settings.value( path + "/database", "" ).toString() );
    el.setAttribute( "service", settings.value( path + "/service", "" ).toString() );
    el.setAttribute( "sslmode", settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( "estimatedMetadata", settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( "saveUsername", settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true" )
    {
      el.setAttribute( "username", settings.value( path + "/username", "" ).toString() );
    }

    el.setAttribute( "savePassword", settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == "true" )
    {
      el.setAttribute( "password", settings.value( path + "/password", "" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

void QgsManageConnectionsDialog::loadWMSConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsWMSConnections" )
  {
    QMessageBox::information( this, tr( "Loading connections" ),
                              tr( "The file is not an WMS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wms" );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
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
    settings.beginGroup( "/Qgis/connections-wms" );
    settings.setValue( QString( "/" + connectionName + "/url" ) , child.attribute( "url" ) );
    settings.setValue( QString( "/" + connectionName + "/ignoreGetMapURI" ), child.attribute( "ignoreGetMapURI" ) == "true" );
    settings.setValue( QString( "/" + connectionName + "/ignoreGetFeatureInfoURI" ), child.attribute( "ignoreGetFeatureInfoURI" ) == "true" );
    settings.setValue( QString( "/" + connectionName + "/ignoreAxisOrientation" ), child.attribute( "ignoreAxisOrientation" ) == "true" );
    settings.setValue( QString( "/" + connectionName + "/invertAxisOrientation" ), child.attribute( "invertAxisOrientation" ) == "true" );
    settings.endGroup();

    if ( !child.attribute( "username" ).isEmpty() )
    {
      settings.beginGroup( "/Qgis/WMS/" + connectionName );
      settings.setValue( "/username", child.attribute( "username" ) );
      settings.setValue( "/password", child.attribute( "password" ) );
      settings.endGroup();
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadWFSConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsWFSConnections" )
  {
    QMessageBox::information( this, tr( "Loading connections" ),
                              tr( "The file is not an WFS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wfs" );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
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
    settings.beginGroup( "/Qgis/connections-wfs" );
    settings.setValue( QString( "/" + connectionName + "/url" ) , child.attribute( "url" ) );
    settings.endGroup();

    if ( !child.attribute( "username" ).isEmpty() )
    {
      settings.beginGroup( "/Qgis/WFS/" + connectionName );
      settings.setValue( "/username", child.attribute( "username" ) );
      settings.setValue( "/password", child.attribute( "password" ) );
      settings.endGroup();
    }
    child = child.nextSiblingElement();
  }
}


void QgsManageConnectionsDialog::loadPgConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsPgConnections" )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not an PostGIS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QSettings settings;
  settings.beginGroup( "/PostgreSQL/connections" );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
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

    settings.setValue( "/host", child.attribute( "host" ) );
    settings.setValue( "/port", child.attribute( "port" ) );
    settings.setValue( "/database", child.attribute( "database" ) );
    if ( child.hasAttribute( "service" ) )
    {
      settings.setValue( "/service", child.attribute( "service" ) );
    }
    else
    {
      settings.setValue( "/service", "" );
    }
    settings.setValue( "/sslmode", child.attribute( "sslmode" ) );
    settings.setValue( "/estimatedMetadata", child.attribute( "estimatedMetadata" ) );
    settings.setValue( "/saveUsername", child.attribute( "saveUsername" ) );
    settings.setValue( "/username", child.attribute( "username" ) );
    settings.setValue( "/savePassword", child.attribute( "savePassword" ) );
    settings.setValue( "/password", child.attribute( "password" ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadMssqlConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsMssqlConnections" )
  {
    QMessageBox::information( this,
                              tr( "Loading connections" ),
                              tr( "The file is not an PostGIS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QSettings settings;
  settings.beginGroup( "/MSSQL/connections" );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
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

    settings.setValue( "/host", child.attribute( "host" ) );
    settings.setValue( "/port", child.attribute( "port" ) );
    settings.setValue( "/database", child.attribute( "database" ) );
    if ( child.hasAttribute( "service" ) )
    {
      settings.setValue( "/service", child.attribute( "service" ) );
    }
    else
    {
      settings.setValue( "/service", "" );
    }
    settings.setValue( "/sslmode", child.attribute( "sslmode" ) );
    settings.setValue( "/estimatedMetadata", child.attribute( "estimatedMetadata" ) );
    settings.setValue( "/saveUsername", child.attribute( "saveUsername" ) );
    settings.setValue( "/username", child.attribute( "username" ) );
    settings.setValue( "/savePassword", child.attribute( "savePassword" ) );
    settings.setValue( "/password", child.attribute( "password" ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::selectAll()
{
  listConnections->selectAll();
}

void QgsManageConnectionsDialog::clearSelection()
{
  listConnections->clearSelection();
}
