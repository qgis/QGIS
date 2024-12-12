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
#include "moc_qgsmanageconnectionsdialog.cpp"
#include "qgshttpheaders.h"
#include "qgsowsconnection.h"
#include "qgsvectortileconnection.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgstiledsceneconnection.h"
#include "qgssensorthingsconnection.h"
#include "qgsgdalcloudconnection.h"
#include "qgsstacconnection.h"

QgsManageConnectionsDialog::QgsManageConnectionsDialog( QWidget *parent, Mode mode, Type type, const QString &fileName )
  : QDialog( parent )
  , mFileName( fileName )
  , mDialogMode( mode )
  , mConnectionType( type )
{
  setupUi( this );

  // additional buttons
  QPushButton *pb = nullptr;
  pb = new QPushButton( tr( "Select All" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, &QAbstractButton::clicked, this, &QgsManageConnectionsDialog::selectAll );

  pb = new QPushButton( tr( "Clear Selection" ) );
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
  const QList<QListWidgetItem *> selection = listConnections->selectedItems();
  if ( selection.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Export/Import Error" ), tr( "You should select at least one connection from list." ) );
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
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
    // return dialog focus on Mac
    activateWindow();
    raise();
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
      case HANA:
        doc = saveHanaConnections( items );
        break;
      case XyzTiles:
        doc = saveXyzTilesConnections( items );
        break;
      case ArcgisMapServer:
      case ArcgisFeatureServer:
        doc = saveArcgisConnections( items );
        break;
      case VectorTile:
        doc = saveVectorTileConnections( items );
        break;
      case TiledScene:
        doc = saveTiledSceneConnections( items );
        break;
      case SensorThings:
        doc = saveSensorThingsConnections( items );
        break;
      case CloudStorage:
        doc = saveCloudStorageConnections( items );
        break;
      case STAC:
        doc = saveStacConnections( items );
        break;
    }

    QFile file( mFileName );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      QMessageBox::warning( this, tr( "Saving Connections" ), tr( "Cannot write file %1:\n%2." ).arg( mFileName, file.errorString() ) );
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
      QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Cannot read file %1:\n%2." ).arg( mFileName, file.errorString() ) );
      return;
    }

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
      QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Parse error at line %1, column %2:\n%3" ).arg( errorLine ).arg( errorColumn ).arg( errorStr ) );
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
      case HANA:
        loadHanaConnections( doc, items );
        break;
      case XyzTiles:
        loadXyzTilesConnections( doc, items );
        break;
      case ArcgisMapServer:
        loadArcgisConnections( doc, items, QStringLiteral( "ARCGISMAPSERVER" ) );
        break;
      case ArcgisFeatureServer:
        loadArcgisConnections( doc, items, QStringLiteral( "ARCGISFEATURESERVER" ) );
        break;
      case VectorTile:
        loadVectorTileConnections( doc, items );
        break;
      case TiledScene:
        loadTiledSceneConnections( doc, items );
        break;
      case SensorThings:
        loadSensorThingsConnections( doc, items );
        break;
      case CloudStorage:
        loadCloudStorageConnections( doc, items );
        break;
      case STAC:
        loadStacConnections( doc, items );
        break;
    }
    // clear connections list and close window
    listConnections->clear();
    accept();
  }

  mFileName.clear();
}

bool QgsManageConnectionsDialog::populateConnections()
{
  // Export mode. Populate connections list from settings
  if ( mDialogMode == Export )
  {
    QStringList connections;
    QgsSettings settings;
    switch ( mConnectionType )
    {
      case WMS:
        connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wms" ) } );
        break;
      case WFS:
        connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wfs" ) } );
        break;
      case WCS:
        connections = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wcs" ) } );
        break;
      case PostGIS:
        settings.beginGroup( QStringLiteral( "/PostgreSQL/connections" ) );
        connections = settings.childGroups();
        break;
      case MSSQL:
        settings.beginGroup( QStringLiteral( "/MSSQL/connections" ) );
        connections = settings.childGroups();
        break;
      case Oracle:
        settings.beginGroup( QStringLiteral( "/Oracle/connections" ) );
        connections = settings.childGroups();
        break;
      case HANA:
        settings.beginGroup( QStringLiteral( "/HANA/connections" ) );
        connections = settings.childGroups();
        break;
      case XyzTiles:
        connections = QgsXyzConnectionSettings::sTreeXyzConnections->items();
        break;
      case ArcgisMapServer:
      case ArcgisFeatureServer:
        connections = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
        break;
      case VectorTile:
        connections = QgsVectorTileProviderConnection::sTreeConnectionVectorTile->items();
        break;
      case TiledScene:
        connections = QgsTiledSceneProviderConnection::sTreeConnectionTiledScene->items();
        break;
      case SensorThings:
        connections = QgsSensorThingsProviderConnection::sTreeSensorThingsConnections->items();
        break;
      case CloudStorage:
        connections = QgsGdalCloudProviderConnection::sTreeConnectionCloud->items();
        break;
      case STAC:
        connections = QgsStacConnection::sTreeConnectionStac->items();
        break;
    }
    for ( const QString &connection : std::as_const( connections ) )
    {
      QListWidgetItem *item = new QListWidgetItem();
      item->setText( connection );
      listConnections->addItem( item );
    }
  }
  // Import mode. Populate connections list from file
  else
  {
    QFile file( mFileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Cannot read file %1:\n%2." ).arg( mFileName, file.errorString() ) );
      return false;
    }

    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
      QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Parse error at line %1, column %2:\n%3" ).arg( errorLine ).arg( errorColumn ).arg( errorStr ) );
      return false;
    }

    const QDomElement root = doc.documentElement();
    switch ( mConnectionType )
    {
      case WMS:
        if ( root.tagName() != QLatin1String( "qgsWMSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WMS connections exchange file." ) );
          return false;
        }
        break;

      case WFS:
        if ( root.tagName() != QLatin1String( "qgsWFSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WFS connections exchange file." ) );
          return false;
        }
        break;

      case WCS:
        if ( root.tagName() != QLatin1String( "qgsWCSConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WCS connections exchange file." ) );
          return false;
        }
        break;

      case PostGIS:
        if ( root.tagName() != QLatin1String( "qgsPgConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a PostGIS connections exchange file." ) );
          return false;
        }
        break;

      case MSSQL:
        if ( root.tagName() != QLatin1String( "qgsMssqlConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a MS SQL Server connections exchange file." ) );
          return false;
        }
        break;
      case Oracle:
        if ( root.tagName() != QLatin1String( "qgsOracleConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not an Oracle connections exchange file." ) );
          return false;
        }
        break;
      case HANA:
        if ( root.tagName() != QLatin1String( "qgsHanaConnections" ) )
        {
          QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The file is not a HANA connections exchange file." ) );
          return false;
        }
        break;
      case XyzTiles:
        if ( root.tagName() != QLatin1String( "qgsXYZTilesConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a XYZ Tiles connections exchange file." ) );
          return false;
        }
        break;
      case ArcgisMapServer:
        if ( root.tagName() != QLatin1String( "qgsARCGISMAPSERVERConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a ArcGIS Map Service connections exchange file." ) );
          return false;
        }
        break;
      case ArcgisFeatureServer:
        if ( root.tagName() != QLatin1String( "qgsARCGISFEATURESERVERConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a ArcGIS Feature Service connections exchange file." ) );
          return false;
        }
        break;
      case VectorTile:
        if ( root.tagName() != QLatin1String( "qgsVectorTileConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a Vector Tile connections exchange file." ) );
          return false;
        }
        break;
      case TiledScene:
        if ( root.tagName() != QLatin1String( "qgsTiledSceneConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a tiled scene connections exchange file." ) );
          return false;
        }
        break;
      case SensorThings:
        if ( root.tagName() != QLatin1String( "qgsSensorThingsConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a SensorThings connections exchange file." ) );
          return false;
        }
        break;
      case CloudStorage:
        if ( root.tagName() != QLatin1String( "qgsCloudStorageConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a cloud storage connections exchange file." ) );
          return false;
        }
        break;
      case STAC:
        if ( root.tagName() != QLatin1String( "qgsStacConnections" ) )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a STAC connections exchange file." ) );
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

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( service.toLower() );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsOwsConnection::settingsUrl->value( { service.toLower(), connections[i] } ) );

    if ( service == QLatin1String( "WMS" ) )
    {
      el.setAttribute( QStringLiteral( "ignoreGetMapURI" ), QgsOwsConnection::settingsIgnoreGetMapURI->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( QStringLiteral( "ignoreGetFeatureInfoURI" ), QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( QStringLiteral( "ignoreAxisOrientation" ), QgsOwsConnection::settingsIgnoreAxisOrientation->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( QStringLiteral( "invertAxisOrientation" ), QgsOwsConnection::settingsInvertAxisOrientation->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( QStringLiteral( "smoothPixmapTransform" ), QgsOwsConnection::settingsSmoothPixmapTransform->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( QStringLiteral( "dpiMode" ), static_cast<int>( QgsOwsConnection::settingsDpiMode->value( { service.toLower(), connections[i] } ) ) );

      QgsHttpHeaders httpHeader( QgsOwsConnection::settingsHeaders->value( { service.toLower(), connections[i] } ) );
      httpHeader.updateDomElement( el );
    }

    el.setAttribute( QStringLiteral( "username" ), QgsOwsConnection::settingsUsername->value( { service.toLower(), connections[i] } ) );
    el.setAttribute( QStringLiteral( "password" ), QgsOwsConnection::settingsPassword->value( { service.toLower(), connections[i] } ) );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveWfsConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsWFSConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "wfs" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsOwsConnection::settingsUrl->value( { QStringLiteral( "wfs" ), connections[i] } ) );

    el.setAttribute( QStringLiteral( "version" ), QgsOwsConnection::settingsVersion->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "maxnumfeatures" ), QgsOwsConnection::settingsMaxNumFeatures->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "pagesize" ), QgsOwsConnection::settingsPagesize->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "pagingenabled" ), QgsOwsConnection::settingsPagingEnabled->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "ignoreAxisOrientation" ), QgsOwsConnection::settingsIgnoreAxisOrientation->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "invertAxisOrientation" ), QgsOwsConnection::settingsInvertAxisOrientation->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "username" ), QgsOwsConnection::settingsUsername->value( { QStringLiteral( "wfs" ), connections[i] } ) );
    el.setAttribute( QStringLiteral( "password" ), QgsOwsConnection::settingsPassword->value( { QStringLiteral( "wfs" ), connections[i] } ) );
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

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/PostgreSQL/connections/" + connections[i];
    QDomElement el = doc.createElement( QStringLiteral( "postgis" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database" ).toString() );
    el.setAttribute( QStringLiteral( "service" ), settings.value( path + "/service" ).toString() );
    el.setAttribute( QStringLiteral( "sslmode" ), settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );
    el.setAttribute( QStringLiteral( "projectsInDatabase" ), settings.value( path + "/projectsInDatabase", "0" ).toString() );
    el.setAttribute( QStringLiteral( "dontResolveType" ), settings.value( path + "/dontResolveType", "0" ).toString() );
    el.setAttribute( QStringLiteral( "allowGeometrylessTables" ), settings.value( path + "/allowGeometrylessTables", "0" ).toString() );
    el.setAttribute( QStringLiteral( "geometryColumnsOnly" ), settings.value( path + "/geometryColumnsOnly", "0" ).toString() );
    el.setAttribute( QStringLiteral( "publicOnly" ), settings.value( path + "/publicOnly", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password" ).toString() );
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

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/MSSQL/connections/" + connections[i];
    QDomElement el = doc.createElement( QStringLiteral( "mssql" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database" ).toString() );
    el.setAttribute( QStringLiteral( "service" ), settings.value( path + "/service" ).toString() );
    el.setAttribute( QStringLiteral( "sslmode" ), settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password" ).toString() );
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

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/Oracle/connections/" + connections[i];
    QDomElement el = doc.createElement( QStringLiteral( "oracle" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host" ).toString() );
    el.setAttribute( QStringLiteral( "port" ), settings.value( path + "/port" ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database" ).toString() );
    el.setAttribute( QStringLiteral( "dboptions" ), settings.value( path + "/dboptions" ).toString() );
    el.setAttribute( QStringLiteral( "dbworkspace" ), settings.value( path + "/dbworkspace" ).toString() );
    el.setAttribute( QStringLiteral( "schema" ), settings.value( path + "/schema" ).toString() );
    el.setAttribute( QStringLiteral( "estimatedMetadata" ), settings.value( path + "/estimatedMetadata", "0" ).toString() );
    el.setAttribute( QStringLiteral( "userTablesOnly" ), settings.value( path + "/userTablesOnly", "0" ).toString() );
    el.setAttribute( QStringLiteral( "geometryColumnsOnly" ), settings.value( path + "/geometryColumnsOnly", "0" ).toString() );
    el.setAttribute( QStringLiteral( "allowGeometrylessTables" ), settings.value( path + "/allowGeometrylessTables", "0" ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveHanaConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsHanaConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/HANA/connections/" + connections[i];
    QDomElement el = doc.createElement( QStringLiteral( "hana" ) );
    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "driver" ), settings.value( path + "/driver", QString() ).toString() );
    el.setAttribute( QStringLiteral( "host" ), settings.value( path + "/host", QString() ).toString() );
    el.setAttribute( QStringLiteral( "identifierType" ), settings.value( path + "/identifierType", QString() ).toString() );
    el.setAttribute( QStringLiteral( "identifier" ), settings.value( path + "/identifier", QString() ).toString() );
    el.setAttribute( QStringLiteral( "multitenant" ), settings.value( path + "/multitenant", QString() ).toString() );
    el.setAttribute( QStringLiteral( "database" ), settings.value( path + "/database", QString() ).toString() );
    el.setAttribute( QStringLiteral( "schema" ), settings.value( path + "/schema", QString() ).toString() );
    el.setAttribute( QStringLiteral( "userTablesOnly" ), settings.value( path + "/userTablesOnly", QStringLiteral( "0" ) ).toString() );
    el.setAttribute( QStringLiteral( "allowGeometrylessTables" ), settings.value( path + "/allowGeometrylessTables", QStringLiteral( "0" ) ).toString() );

    el.setAttribute( QStringLiteral( "saveUsername" ), settings.value( path + "/saveUsername", QStringLiteral( "false" ) ).toString() );
    if ( settings.value( path + "/saveUsername", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "username" ), settings.value( path + "/username", QString() ).toString() );
    }

    el.setAttribute( QStringLiteral( "savePassword" ), settings.value( path + "/savePassword", QStringLiteral( "false" ) ).toString() );
    if ( settings.value( path + "/savePassword", "false" ).toString() == QLatin1String( "true" ) )
    {
      el.setAttribute( QStringLiteral( "password" ), settings.value( path + "/password", QString() ).toString() );
    }

    el.setAttribute( QStringLiteral( "sslEnabled" ), settings.value( path + "/sslEnabled", QStringLiteral( "false" ) ).toString() );
    el.setAttribute( QStringLiteral( "sslCryptoProvider" ), settings.value( path + "/sslCryptoProvider", QStringLiteral( "openssl" ) ).toString() );
    el.setAttribute( QStringLiteral( "sslKeyStore" ), settings.value( path + "/sslKeyStore", QString() ).toString() );
    el.setAttribute( QStringLiteral( "sslTrustStore" ), settings.value( path + "/sslTrustStore", QString() ).toString() );
    el.setAttribute( QStringLiteral( "sslValidateCertificate" ), settings.value( path + "/sslValidateCertificate", QStringLiteral( "false" ) ).toString() );
    el.setAttribute( QStringLiteral( "sslHostNameInCertificate" ), settings.value( path + "/sslHostNameInCertificate", QString() ).toString() );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveXyzTilesConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsXYZTilesConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "xyztiles" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsXyzConnectionSettings::settingsUrl->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "zmin" ), QgsXyzConnectionSettings::settingsZmin->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "zmax" ), QgsXyzConnectionSettings::settingsZmax->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsXyzConnectionSettings::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "username" ), QgsXyzConnectionSettings::settingsUsername->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "password" ), QgsXyzConnectionSettings::settingsPassword->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "tilePixelRatio" ), QgsXyzConnectionSettings::settingsTilePixelRatio->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsXyzConnectionSettings::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveArcgisConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( "qgsARCGISFEATURESERVERConnections" );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( const QString &connection : connections )
  {
    QDomElement el = doc.createElement( QStringLiteral( "arcgisfeatureserver" ) );
    el.setAttribute( QStringLiteral( "name" ), connection );
    el.setAttribute( QStringLiteral( "url" ), QgsArcGisConnectionSettings::settingsUrl->value( connection ) );

    QgsHttpHeaders httpHeader( QgsArcGisConnectionSettings::settingsHeaders->value( connection ) );
    httpHeader.updateDomElement( el );

    el.setAttribute( QStringLiteral( "username" ), QgsArcGisConnectionSettings::settingsUsername->value( connection ) );
    el.setAttribute( QStringLiteral( "password" ), QgsArcGisConnectionSettings::settingsPassword->value( connection ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsArcGisConnectionSettings::settingsAuthcfg->value( connection ) );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveVectorTileConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsVectorTileConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "vectortile" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsVectorTileProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "zmin" ), QgsVectorTileProviderConnection::settingsZmin->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "zmax" ), QgsVectorTileProviderConnection::settingsZmax->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "serviceType" ), QgsVectorTileProviderConnection::settingsServiceType->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsVectorTileProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "username" ), QgsVectorTileProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "password" ), QgsVectorTileProviderConnection::settingsPassword->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "styleUrl" ), QgsVectorTileProviderConnection::settingsStyleUrl->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsVectorTileProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveTiledSceneConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsTiledSceneConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "tiledscene" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "provider" ), QgsTiledSceneProviderConnection::settingsProvider->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "url" ), QgsTiledSceneProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsTiledSceneProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "username" ), QgsTiledSceneProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "password" ), QgsTiledSceneProviderConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsTiledSceneProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveSensorThingsConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsSensorThingsConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "sensorthings" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsSensorThingsProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsSensorThingsProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "username" ), QgsSensorThingsProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "password" ), QgsSensorThingsProviderConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsTiledSceneProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el );

    root.appendChild( el );
  }

  return doc;
}


QDomDocument QgsManageConnectionsDialog::saveCloudStorageConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsCloudStorageConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "cloudstorage" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "handler" ), QgsGdalCloudProviderConnection::settingsVsiHandler->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "container" ), QgsGdalCloudProviderConnection::settingsContainer->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "path" ), QgsGdalCloudProviderConnection::settingsPath->value( connections[i] ) );

    const QVariantMap credentialOptions = QgsGdalCloudProviderConnection::settingsCredentialOptions->value( connections[i] );
    QString credentialString;
    for ( auto it = credentialOptions.constBegin(); it != credentialOptions.constEnd(); ++it )
    {
      if ( !it.value().toString().isEmpty() )
      {
        credentialString += QStringLiteral( "|credential:%1=%2" ).arg( it.key(), it.value().toString() );
      }
    }
    el.setAttribute( QStringLiteral( "credentials" ), credentialString );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveStacConnections( const QStringList &connections )
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgsStacConnections" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( QStringLiteral( "stac" ) );

    el.setAttribute( QStringLiteral( "name" ), connections[i] );
    el.setAttribute( QStringLiteral( "url" ), QgsStacConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "authcfg" ), QgsStacConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "username" ), QgsStacConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( QStringLiteral( "password" ), QgsStacConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsStacConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el );

    root.appendChild( el );
  }

  return doc;
}

void QgsManageConnectionsDialog::loadOWSConnections( const QDomDocument &doc, const QStringList &items, const QString &service )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgs" + service.toUpper() + "Connections" )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a %1 connections exchange file." ).arg( service ) );
    return;
  }

  QString connectionName;

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
    if ( QgsOwsConnection::settingsUrl->exists( { service.toLower(), connectionName } ) && prompt )
    {
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( QgsOwsConnection::settingsUrl->exists( { service.toLower(), connectionName } ) && !overwrite )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // no dups detected or overwrite is allowed
    QgsOwsConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreGetMapURI->setValue( child.attribute( QStringLiteral( "ignoreGetMapURI" ) ) == QLatin1String( "true" ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->setValue( child.attribute( QStringLiteral( "ignoreGetFeatureInfoURI" ) ) == QLatin1String( "true" ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( child.attribute( QStringLiteral( "ignoreAxisOrientation" ) ) == QLatin1String( "true" ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( child.attribute( QStringLiteral( "invertAxisOrientation" ) ) == QLatin1String( "true" ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsSmoothPixmapTransform->setValue( child.attribute( QStringLiteral( "smoothPixmapTransform" ) ) == QLatin1String( "true" ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsDpiMode->setValue( static_cast<Qgis::DpiMode>( child.attribute( QStringLiteral( "dpiMode" ), QStringLiteral( "7" ) ).toInt() ), { service.toLower(), connectionName } );

    QgsHttpHeaders httpHeader( child );
    QgsOwsConnection::settingsHeaders->setValue( httpHeader.headers(), { service.toLower(), connectionName } );

    if ( !child.attribute( QStringLiteral( "username" ) ).isEmpty() )
    {
      QgsOwsConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), { service.toUpper(), connectionName } );
      QgsOwsConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), { service.toUpper(), connectionName } );
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadWfsConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsWFSConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WFS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QStringList keys = QgsOwsConnection::sTreeOwsConnections->items( { QStringLiteral( "wfs" ) } );

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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    // no dups detected or overwrite is allowed

    QgsOwsConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsVersion->setValue( child.attribute( QStringLiteral( "version" ) ), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsMaxNumFeatures->setValue( child.attribute( QStringLiteral( "maxnumfeatures" ) ), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsPagesize->setValue( child.attribute( QStringLiteral( "pagesize" ) ), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsPagingEnabled->setValue( child.attribute( QStringLiteral( "pagingenabled" ) ), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( child.attribute( QStringLiteral( "ignoreAxisOrientation" ) ).toInt(), { QStringLiteral( "wfs" ), connectionName } );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( child.attribute( QStringLiteral( "invertAxisOrientation" ) ).toInt(), { QStringLiteral( "wfs" ), connectionName } );

    if ( !child.attribute( QStringLiteral( "username" ) ).isEmpty() )
    {
      QgsOwsConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), { QStringLiteral( "wfs" ), connectionName } );
      QgsOwsConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), { QStringLiteral( "wfs" ), connectionName } );
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadPgConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsPgConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a PostGIS connections exchange file." ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
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
    settings.setValue( QStringLiteral( "/projectsInDatabase" ), child.attribute( QStringLiteral( "projectsInDatabase" ), 0 ) );
    settings.setValue( QStringLiteral( "/dontResolveType" ), child.attribute( QStringLiteral( "dontResolveType" ), 0 ) );
    settings.setValue( QStringLiteral( "/allowGeometrylessTables" ), child.attribute( QStringLiteral( "allowGeometrylessTables" ), 0 ) );
    settings.setValue( QStringLiteral( "/geometryColumnsOnly" ), child.attribute( QStringLiteral( "geometryColumnsOnly" ), 0 ) );
    settings.setValue( QStringLiteral( "/publicOnly" ), child.attribute( QStringLiteral( "publicOnly" ), 0 ) );
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
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsMssqlConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a MS SQL Server connections exchange file." ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
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
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsOracleConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not an Oracle connections exchange file." ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/Oracle/connections/" + connectionName );

    settings.setValue( QStringLiteral( "/host" ), child.attribute( QStringLiteral( "host" ) ) );
    settings.setValue( QStringLiteral( "/port" ), child.attribute( QStringLiteral( "port" ) ) );
    settings.setValue( QStringLiteral( "/database" ), child.attribute( QStringLiteral( "database" ) ) );
    settings.setValue( QStringLiteral( "/dboptions" ), child.attribute( QStringLiteral( "dboptions" ) ) );
    settings.setValue( QStringLiteral( "/dbworkspace" ), child.attribute( QStringLiteral( "dbworkspace" ) ) );
    settings.setValue( QStringLiteral( "/schema" ), child.attribute( QStringLiteral( "schema" ) ) );
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

void QgsManageConnectionsDialog::loadHanaConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsHanaConnections" ) )
  {
    QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The file is not a HANA connections exchange file." ) );
    return;
  }

  const QDomAttr version = root.attributeNode( "version" );
  if ( version.value() != QLatin1String( "1.0" ) )
  {
    QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The HANA connections exchange file version '%1' is not supported." ).arg( version.value() ) );
    return;
  }

  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/HANA/connections" ) );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    const QString connectionName = child.attribute( QStringLiteral( "name" ) );
    if ( !items.contains( connectionName ) )
    {
      child = child.nextSiblingElement();
      continue;
    }

    // check for duplicates
    if ( keys.contains( connectionName ) && prompt )
    {
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    //no dups detected or overwrite is allowed
    settings.beginGroup( "/HANA/connections/" + connectionName );

    for ( const QString param :
          { "driver", "host", "database", "identifierType", "identifier", "multitenant", "schema", "userTablesOnly",
            "allowGeometrylessTables", "saveUsername", "username", "savePassword", "password", "sslEnabled",
            "sslCryptoProvider", "sslKeyStore", "sslTrustStore", "sslValidateCertificate", "sslHostNameInCertificate"
          } )
      settings.setValue( QStringLiteral( "/" ) + param, child.attribute( param ) );

    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadXyzTilesConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsXYZTilesConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a XYZ Tiles connections exchange file." ) );
    return;
  }

  QString connectionName;
  QStringList keys = QgsXyzConnectionSettings::sTreeXyzConnections->items();
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }


    QgsXyzConnectionSettings::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );
    QgsXyzConnectionSettings::settingsZmin->setValue( child.attribute( QStringLiteral( "zmin" ) ).toInt(), connectionName );
    QgsXyzConnectionSettings::settingsZmax->setValue( child.attribute( QStringLiteral( "zmax" ) ).toInt(), connectionName );
    QgsXyzConnectionSettings::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );
    QgsXyzConnectionSettings::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsXyzConnectionSettings::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );
    QgsXyzConnectionSettings::settingsTilePixelRatio->setValue( child.attribute( QStringLiteral( "tilePixelRatio" ) ).toInt(), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsXyzConnectionSettings::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadArcgisConnections( const QDomDocument &doc, const QStringList &items, const QString &service )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgs" + service.toUpper() + "Connections" )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a %1 connections exchange file." ).arg( service ) );
    return;
  }

  QString connectionName;
  QStringList keys = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    // no dups detected or overwrite is allowed
    QgsArcGisConnectionSettings::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );

    QgsArcGisConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( child ).headers(), connectionName );


    QgsArcGisConnectionSettings::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsArcGisConnectionSettings::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );
    QgsArcGisConnectionSettings::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadVectorTileConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsVectorTileConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a Vector Tile connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/qgis/connections-vector-tile" ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsVectorTileProviderConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );
    QgsVectorTileProviderConnection::settingsZmin->setValue( child.attribute( QStringLiteral( "zmin" ) ).toInt(), connectionName );
    QgsVectorTileProviderConnection::settingsZmax->setValue( child.attribute( QStringLiteral( "zmax" ) ).toInt(), connectionName );
    QgsVectorTileProviderConnection::settingsServiceType->setValue( child.attribute( QStringLiteral( "serviceType" ) ), connectionName );
    QgsVectorTileProviderConnection::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );
    QgsVectorTileProviderConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsVectorTileProviderConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );
    QgsVectorTileProviderConnection::settingsStyleUrl->setValue( child.attribute( QStringLiteral( "styleUrl" ) ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsVectorTileProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadTiledSceneConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsTiledSceneConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a tiled scene connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/qgis/connections-tiled-scene" ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsTiledSceneProviderConnection::settingsProvider->setValue( child.attribute( QStringLiteral( "provider" ) ), connectionName );
    QgsTiledSceneProviderConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );
    QgsTiledSceneProviderConnection::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );
    QgsTiledSceneProviderConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsTiledSceneProviderConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsTiledSceneProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadSensorThingsConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsSensorThingsConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a SensorThings connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/connections/sensorthings/items" ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsSensorThingsProviderConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );
    QgsSensorThingsProviderConnection::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );
    QgsSensorThingsProviderConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsSensorThingsProviderConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsSensorThingsProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadCloudStorageConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsCloudStorageConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a cloud storage connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/connections/cloud/items" ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsGdalCloudProviderConnection::settingsVsiHandler->setValue( child.attribute( QStringLiteral( "handler" ) ), connectionName );
    QgsGdalCloudProviderConnection::settingsContainer->setValue( child.attribute( QStringLiteral( "container" ) ), connectionName );
    QgsGdalCloudProviderConnection::settingsPath->setValue( child.attribute( QStringLiteral( "path" ) ), connectionName );

    QString credentialString = child.attribute( QStringLiteral( "credentials" ) );

    QVariantMap credentialOptions;
    while ( true )
    {
      const thread_local QRegularExpression credentialOptionRegex( QStringLiteral( "\\|credential:([^|]*)" ) );
      const thread_local QRegularExpression credentialOptionKeyValueRegex( QStringLiteral( "(.*?)=(.*)" ) );

      const QRegularExpressionMatch match = credentialOptionRegex.match( credentialString );
      if ( match.hasMatch() )
      {
        const QRegularExpressionMatch keyValueMatch = credentialOptionKeyValueRegex.match( match.captured( 1 ) );
        if ( keyValueMatch.hasMatch() )
        {
          credentialOptions.insert( keyValueMatch.captured( 1 ), keyValueMatch.captured( 2 ) );
        }
        credentialString = credentialString.remove( match.capturedStart( 0 ), match.capturedLength( 0 ) );
      }
      else
      {
        break;
      }
    }

    QgsGdalCloudProviderConnection::settingsCredentialOptions->setValue( credentialOptions, connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadStacConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "qgsStacConnections" ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a STAC connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/qgis/connections-stac" ) );
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
      const int res = QMessageBox::warning( this, tr( "Loading Connections" ), tr( "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );

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

    if ( keys.contains( connectionName ) )
    {
      if ( !overwrite )
      {
        child = child.nextSiblingElement();
        continue;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsStacConnection::settingsUrl->setValue( child.attribute( QStringLiteral( "url" ) ), connectionName );
    QgsStacConnection::settingsAuthcfg->setValue( child.attribute( QStringLiteral( "authcfg" ) ), connectionName );
    QgsStacConnection::settingsUsername->setValue( child.attribute( QStringLiteral( "username" ) ), connectionName );
    QgsStacConnection::settingsPassword->setValue( child.attribute( QStringLiteral( "password" ) ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsStacConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

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
