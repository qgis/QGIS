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

/* $Id$ */

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QDebug>

#include "qgsmanageconnectionsdialog.h"

QgsManageConnectionsDialog::QgsManageConnectionsDialog( QWidget *parent, Mode mode, Type type ) : QDialog( parent ), mDialogMode( mode ), mConnectionType( type )
{
  setupUi( this );

  if ( mDialogMode == Load )
  {
    label->setText( tr( "Load from file" ) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Load" ) );
  }
  else
  {
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Save" ) );
    populateConnections();
  }

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsManageConnectionsDialog::on_btnBrowse_clicked()
{
  QString fileName;
  if ( mDialogMode == Save )
  {
    fileName = QFileDialog::getSaveFileName( this, tr( "Save connections" ), ".", tr( "XML files (*.xml *.XML)" ) );
  }
  else
  {
    fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".", tr( "XML files (*.xml *XML)" ) );
  }

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
  leFileName->setText( mFileName );
  //buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

  if ( mDialogMode == Load )
  {
    populateConnections();
  }

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
}

void QgsManageConnectionsDialog::on_buttonBox_accepted()
{
  QList<QListWidgetItem *> selection = listConnections->selectedItems();
  if ( selection.isEmpty() )
  {
    return;
  }
  QStringList items;
  for ( int i = 0; i < selection.size(); ++i )
  {
    items.append( selection.at( i )->text() );
  }

  if ( mDialogMode == Save )
  {
    QDomDocument doc;
    if ( mConnectionType == WMS )
    {
      doc = saveWMSConnections( items );
    }
    else
    {
      doc = savePgConnections( items );
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
  else // load connections
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

    if ( mConnectionType == WMS )
    {
      loadWMSConnections( doc, items );
    }
    else
    {
      loadPgConnections( doc, items );
    }
  }

  mFileName = "";
  leFileName->clear();
  listConnections->clear();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsManageConnectionsDialog::populateConnections()
{
  // Save mode. Populate connections list from settings
  if ( mDialogMode == 0 )
  {
    QSettings settings;
    if ( mConnectionType == 0 )
    {
      settings.beginGroup( "/Qgis/connections-wms" );
    }
    else
    {
      settings.beginGroup( "/PostgreSQL/connections" );
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
  // Load mode. Populate connections list from file
  else
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

    QDomElement root = doc.documentElement();
    if ( mConnectionType == 0 )
    {
      if ( root.tagName() != "qgsWMSConnections" )
      {
        QMessageBox::information( this, tr( "Loading connections" ),
                                  tr( "The file is not an WMS connections exchange file." ) );
        mFileName = "";
        leFileName->clear();
        listConnections->clear();
        return;
      }
    }
    else
    {
      if ( root.tagName() != "qgsPgConnections" )
      {
        QMessageBox::information( this, tr( "Loading connections" ),
                                  tr( "The file is not an PostGIS connections exchange file." ) );
        mFileName = "";
        leFileName->clear();
        listConnections->clear();
        return;
      }
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

    path = "/Qgis/WMS/";
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
    el.setAttribute( "sslmode", settings.value( path + "/sslmode", "1" ).toString() );

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
  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) )
    {
      int res = QMessageBox::warning( this, tr( "Loading connections" ),
                                      tr( "Connection with name %1 already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }

    // no dups detected or overwrite is allowed
    settings.beginGroup( "/Qgis/connections-wms" );
    settings.setValue( QString( "/" + connectionName + "/url" ) , child.attribute( "url" ) );
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
  while ( !child.isNull() )
  {
    connectionName = child.attribute( "name" );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) )
    {
      int res = QMessageBox::warning( this,
                                      tr( "Loading connections" ),
                                      tr( "Connection with name %1 already exists. Overwrite?" )
                                      .arg( connectionName ),
                                      QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/PostgreSQL/connections/" + connectionName );

    settings.setValue( "/host", child.attribute( "host" ) );
    settings.setValue( "/port", child.attribute( "port" ) );
    settings.setValue( "/database", child.attribute( "database" ) );
    settings.setValue( "/sslmode", child.attribute( "sslmode" ) );
		settings.setValue( "/saveUsername", child.attribute( "saveUsername" ) );
    settings.setValue( "/username", child.attribute( "username" ) );
		settings.setValue( "/savePassword", child.attribute( "savePassword" ) );
    settings.setValue( "/password", child.attribute( "password" ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}
