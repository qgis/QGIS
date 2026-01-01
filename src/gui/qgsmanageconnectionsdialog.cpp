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

#include "qgsmanageconnectionsdialog.h"

#include "qgsgdalcloudconnection.h"
#include "qgshttpheaders.h"
#include "qgsowsconnection.h"
#include "qgssensorthingsconnection.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgsstacconnection.h"
#include "qgstiledsceneconnection.h"
#include "qgsvectortileconnection.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>

#include "moc_qgsmanageconnectionsdialog.cpp"

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
    if ( !fileName.endsWith( ".xml"_L1, Qt::CaseInsensitive ) )
    {
      fileName += ".xml"_L1;
    }

    mFileName = fileName;

    QDomDocument doc;
    switch ( mConnectionType )
    {
      case WMS:
        doc = saveOWSConnections( items, u"WMS"_s );
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
        doc = saveOWSConnections( items, u"WCS"_s );
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
        loadOWSConnections( doc, items, u"WMS"_s );
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
        loadOWSConnections( doc, items, u"WCS"_s );
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
        loadArcgisConnections( doc, items, u"ARCGISMAPSERVER"_s );
        break;
      case ArcgisFeatureServer:
        loadArcgisConnections( doc, items, u"ARCGISFEATURESERVER"_s );
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
        connections = QgsOwsConnection::sTreeOwsConnections->items( { u"wms"_s } );
        break;
      case WFS:
        connections = QgsOwsConnection::sTreeOwsConnections->items( { u"wfs"_s } );
        break;
      case WCS:
        connections = QgsOwsConnection::sTreeOwsConnections->items( { u"wcs"_s } );
        break;
      case PostGIS:
        settings.beginGroup( u"/PostgreSQL/connections"_s );
        connections = settings.childGroups();
        break;
      case MSSQL:
        settings.beginGroup( u"/MSSQL/connections"_s );
        connections = settings.childGroups();
        break;
      case Oracle:
        settings.beginGroup( u"/Oracle/connections"_s );
        connections = settings.childGroups();
        break;
      case HANA:
        settings.beginGroup( u"/HANA/connections"_s );
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
        if ( root.tagName() != "qgsWMSConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WMS connections exchange file." ) );
          return false;
        }
        break;

      case WFS:
        if ( root.tagName() != "qgsWFSConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WFS connections exchange file." ) );
          return false;
        }
        break;

      case WCS:
        if ( root.tagName() != "qgsWCSConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WCS connections exchange file." ) );
          return false;
        }
        break;

      case PostGIS:
        if ( root.tagName() != "qgsPgConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a PostGIS connections exchange file." ) );
          return false;
        }
        break;

      case MSSQL:
        if ( root.tagName() != "qgsMssqlConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a MS SQL Server connections exchange file." ) );
          return false;
        }
        break;
      case Oracle:
        if ( root.tagName() != "qgsOracleConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not an Oracle connections exchange file." ) );
          return false;
        }
        break;
      case HANA:
        if ( root.tagName() != "qgsHanaConnections"_L1 )
        {
          QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The file is not a HANA connections exchange file." ) );
          return false;
        }
        break;
      case XyzTiles:
        if ( root.tagName() != "qgsXYZTilesConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a XYZ Tiles connections exchange file." ) );
          return false;
        }
        break;
      case ArcgisMapServer:
        if ( root.tagName() != "qgsARCGISMAPSERVERConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a ArcGIS Map Service connections exchange file." ) );
          return false;
        }
        break;
      case ArcgisFeatureServer:
        if ( root.tagName() != "qgsARCGISFEATURESERVERConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a ArcGIS Feature Service connections exchange file." ) );
          return false;
        }
        break;
      case VectorTile:
        if ( root.tagName() != "qgsVectorTileConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a Vector Tile connections exchange file." ) );
          return false;
        }
        break;
      case TiledScene:
        if ( root.tagName() != "qgsTiledSceneConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a tiled scene connections exchange file." ) );
          return false;
        }
        break;
      case SensorThings:
        if ( root.tagName() != "qgsSensorThingsConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a SensorThings connections exchange file." ) );
          return false;
        }
        break;
      case CloudStorage:
        if ( root.tagName() != "qgsCloudStorageConnections"_L1 )
        {
          QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a cloud storage connections exchange file." ) );
          return false;
        }
        break;
      case STAC:
        if ( root.tagName() != "qgsStacConnections"_L1 )
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
      item->setText( child.attribute( u"name"_s ) );
      listConnections->addItem( item );
      child = child.nextSiblingElement();
    }
  }
  return true;
}

static void addNamespaceDeclarations( QDomElement &root, const QMap<QString, QString> &namespaceDeclarations )
{
  for ( auto it = namespaceDeclarations.begin(); it != namespaceDeclarations.end(); ++it )
  {
    root.setAttribute( u"xmlns:"_s + it.key(), it.value() );
  }
}

QDomDocument QgsManageConnectionsDialog::saveOWSConnections( const QStringList &connections, const QString &service )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( "qgs" + service.toUpper() + "Connections" );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( service.toLower() );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsOwsConnection::settingsUrl->value( { service.toLower(), connections[i] } ) );

    if ( service == "WMS"_L1 )
    {
      el.setAttribute( u"ignoreGetMapURI"_s, QgsOwsConnection::settingsIgnoreGetMapURI->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( u"ignoreGetFeatureInfoURI"_s, QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( u"ignoreAxisOrientation"_s, QgsOwsConnection::settingsIgnoreAxisOrientation->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( u"invertAxisOrientation"_s, QgsOwsConnection::settingsInvertAxisOrientation->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( u"smoothPixmapTransform"_s, QgsOwsConnection::settingsSmoothPixmapTransform->value( { service.toLower(), connections[i] } ) );
      el.setAttribute( u"dpiMode"_s, static_cast<int>( QgsOwsConnection::settingsDpiMode->value( { service.toLower(), connections[i] } ) ) );

      QgsHttpHeaders httpHeader( QgsOwsConnection::settingsHeaders->value( { service.toLower(), connections[i] } ) );
      httpHeader.updateDomElement( el, namespaceDeclarations );
    }

    el.setAttribute( u"username"_s, QgsOwsConnection::settingsUsername->value( { service.toLower(), connections[i] } ) );
    el.setAttribute( u"password"_s, QgsOwsConnection::settingsPassword->value( { service.toLower(), connections[i] } ) );
    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveWfsConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsWFSConnections"_s );
  root.setAttribute( u"version"_s, u"1.1"_s );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"wfs"_s );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsOwsConnection::settingsUrl->value( { u"wfs"_s, connections[i] } ) );

    el.setAttribute( u"version"_s, QgsOwsConnection::settingsVersion->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"maxnumfeatures"_s, QgsOwsConnection::settingsMaxNumFeatures->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"pagesize"_s, QgsOwsConnection::settingsPagesize->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"pagingenabled"_s, QgsOwsConnection::settingsPagingEnabled->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"ignoreAxisOrientation"_s, QgsOwsConnection::settingsIgnoreAxisOrientation->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"invertAxisOrientation"_s, QgsOwsConnection::settingsInvertAxisOrientation->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"username"_s, QgsOwsConnection::settingsUsername->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"password"_s, QgsOwsConnection::settingsPassword->value( { u"wfs"_s, connections[i] } ) );
    el.setAttribute( u"httpMethod"_s, QgsOwsConnection::settingsPreferredHttpMethod->value( { u"wfs"_s, connections[i] } ) == Qgis::HttpMethod::Post ? u"post"_s : u"get"_s );
    el.setAttribute( u"featureMode"_s, QgsOwsConnection::settingsWfsFeatureMode->value( { u"wfs"_s, connections[i] } ) );
    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::savePgConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsPgConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/PostgreSQL/connections/" + connections[i];
    QDomElement el = doc.createElement( u"postgis"_s );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"host"_s, settings.value( path + "/host" ).toString() );
    el.setAttribute( u"port"_s, settings.value( path + "/port" ).toString() );
    el.setAttribute( u"database"_s, settings.value( path + "/database" ).toString() );
    el.setAttribute( u"service"_s, settings.value( path + "/service" ).toString() );
    el.setAttribute( u"sslmode"_s, settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( u"estimatedMetadata"_s, settings.value( path + "/estimatedMetadata", "0" ).toString() );
    el.setAttribute( u"projectsInDatabase"_s, settings.value( path + "/projectsInDatabase", "0" ).toString() );
    el.setAttribute( u"dontResolveType"_s, settings.value( path + "/dontResolveType", "0" ).toString() );
    el.setAttribute( u"allowGeometrylessTables"_s, settings.value( path + "/allowGeometrylessTables", "0" ).toString() );
    el.setAttribute( u"geometryColumnsOnly"_s, settings.value( path + "/geometryColumnsOnly", "0" ).toString() );
    el.setAttribute( u"publicOnly"_s, settings.value( path + "/publicOnly", "0" ).toString() );
    el.setAttribute( u"schema"_s, settings.value( path + "/schema" ).toString() );
    el.setAttribute( u"saveUsername"_s, settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"username"_s, settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( u"savePassword"_s, settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"password"_s, settings.value( path + "/password" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveMssqlConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsMssqlConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/MSSQL/connections/" + connections[i];
    QDomElement el = doc.createElement( u"mssql"_s );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"host"_s, settings.value( path + "/host" ).toString() );
    el.setAttribute( u"port"_s, settings.value( path + "/port" ).toString() );
    el.setAttribute( u"database"_s, settings.value( path + "/database" ).toString() );
    el.setAttribute( u"service"_s, settings.value( path + "/service" ).toString() );
    el.setAttribute( u"sslmode"_s, settings.value( path + "/sslmode", "1" ).toString() );
    el.setAttribute( u"estimatedMetadata"_s, settings.value( path + "/estimatedMetadata", "0" ).toString() );

    el.setAttribute( u"saveUsername"_s, settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"username"_s, settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( u"savePassword"_s, settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"password"_s, settings.value( path + "/password" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveOracleConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsOracleConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/Oracle/connections/" + connections[i];
    QDomElement el = doc.createElement( u"oracle"_s );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"host"_s, settings.value( path + "/host" ).toString() );
    el.setAttribute( u"port"_s, settings.value( path + "/port" ).toString() );
    el.setAttribute( u"database"_s, settings.value( path + "/database" ).toString() );
    el.setAttribute( u"dboptions"_s, settings.value( path + "/dboptions" ).toString() );
    el.setAttribute( u"dbworkspace"_s, settings.value( path + "/dbworkspace" ).toString() );
    el.setAttribute( u"schema"_s, settings.value( path + "/schema" ).toString() );
    el.setAttribute( u"estimatedMetadata"_s, settings.value( path + "/estimatedMetadata", "0" ).toString() );
    el.setAttribute( u"userTablesOnly"_s, settings.value( path + "/userTablesOnly", "0" ).toString() );
    el.setAttribute( u"geometryColumnsOnly"_s, settings.value( path + "/geometryColumnsOnly", "0" ).toString() );
    el.setAttribute( u"allowGeometrylessTables"_s, settings.value( path + "/allowGeometrylessTables", "0" ).toString() );

    el.setAttribute( u"saveUsername"_s, settings.value( path + "/saveUsername", "false" ).toString() );

    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"username"_s, settings.value( path + "/username" ).toString() );
    }

    el.setAttribute( u"savePassword"_s, settings.value( path + "/savePassword", "false" ).toString() );

    if ( settings.value( path + "/savePassword", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"password"_s, settings.value( path + "/password" ).toString() );
    }

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveHanaConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsHanaConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  const QgsSettings settings;
  QString path;
  for ( int i = 0; i < connections.count(); ++i )
  {
    path = "/HANA/connections/" + connections[i];
    QDomElement el = doc.createElement( u"hana"_s );
    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"driver"_s, settings.value( path + "/driver", QString() ).toString() );
    el.setAttribute( u"host"_s, settings.value( path + "/host", QString() ).toString() );
    el.setAttribute( u"identifierType"_s, settings.value( path + "/identifierType", QString() ).toString() );
    el.setAttribute( u"identifier"_s, settings.value( path + "/identifier", QString() ).toString() );
    el.setAttribute( u"multitenant"_s, settings.value( path + "/multitenant", QString() ).toString() );
    el.setAttribute( u"database"_s, settings.value( path + "/database", QString() ).toString() );
    el.setAttribute( u"schema"_s, settings.value( path + "/schema", QString() ).toString() );
    el.setAttribute( u"userTablesOnly"_s, settings.value( path + "/userTablesOnly", u"0"_s ).toString() );
    el.setAttribute( u"allowGeometrylessTables"_s, settings.value( path + "/allowGeometrylessTables", u"0"_s ).toString() );

    el.setAttribute( u"saveUsername"_s, settings.value( path + "/saveUsername", u"false"_s ).toString() );
    if ( settings.value( path + "/saveUsername", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"username"_s, settings.value( path + "/username", QString() ).toString() );
    }

    el.setAttribute( u"savePassword"_s, settings.value( path + "/savePassword", u"false"_s ).toString() );
    if ( settings.value( path + "/savePassword", "false" ).toString() == "true"_L1 )
    {
      el.setAttribute( u"password"_s, settings.value( path + "/password", QString() ).toString() );
    }

    el.setAttribute( u"sslEnabled"_s, settings.value( path + "/sslEnabled", u"false"_s ).toString() );
    el.setAttribute( u"sslCryptoProvider"_s, settings.value( path + "/sslCryptoProvider", u"openssl"_s ).toString() );
    el.setAttribute( u"sslKeyStore"_s, settings.value( path + "/sslKeyStore", QString() ).toString() );
    el.setAttribute( u"sslTrustStore"_s, settings.value( path + "/sslTrustStore", QString() ).toString() );
    el.setAttribute( u"sslValidateCertificate"_s, settings.value( path + "/sslValidateCertificate", u"false"_s ).toString() );
    el.setAttribute( u"sslHostNameInCertificate"_s, settings.value( path + "/sslHostNameInCertificate", QString() ).toString() );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveXyzTilesConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsXYZTilesConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"xyztiles"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsXyzConnectionSettings::settingsUrl->value( connections[i] ) );
    el.setAttribute( u"zmin"_s, QgsXyzConnectionSettings::settingsZmin->value( connections[i] ) );
    el.setAttribute( u"zmax"_s, QgsXyzConnectionSettings::settingsZmax->value( connections[i] ) );
    el.setAttribute( u"authcfg"_s, QgsXyzConnectionSettings::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( u"username"_s, QgsXyzConnectionSettings::settingsUsername->value( connections[i] ) );
    el.setAttribute( u"password"_s, QgsXyzConnectionSettings::settingsPassword->value( connections[i] ) );
    el.setAttribute( u"tilePixelRatio"_s, QgsXyzConnectionSettings::settingsTilePixelRatio->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsXyzConnectionSettings::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveArcgisConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( "qgsARCGISFEATURESERVERConnections" );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( const QString &connection : connections )
  {
    QDomElement el = doc.createElement( u"arcgisfeatureserver"_s );
    el.setAttribute( u"name"_s, connection );
    el.setAttribute( u"url"_s, QgsArcGisConnectionSettings::settingsUrl->value( connection ) );

    QgsHttpHeaders httpHeader( QgsArcGisConnectionSettings::settingsHeaders->value( connection ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    el.setAttribute( u"username"_s, QgsArcGisConnectionSettings::settingsUsername->value( connection ) );
    el.setAttribute( u"password"_s, QgsArcGisConnectionSettings::settingsPassword->value( connection ) );
    el.setAttribute( u"authcfg"_s, QgsArcGisConnectionSettings::settingsAuthcfg->value( connection ) );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveVectorTileConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsVectorTileConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"vectortile"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsVectorTileProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( u"zmin"_s, QgsVectorTileProviderConnection::settingsZmin->value( connections[i] ) );
    el.setAttribute( u"zmax"_s, QgsVectorTileProviderConnection::settingsZmax->value( connections[i] ) );
    el.setAttribute( u"serviceType"_s, QgsVectorTileProviderConnection::settingsServiceType->value( connections[i] ) );
    el.setAttribute( u"authcfg"_s, QgsVectorTileProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( u"username"_s, QgsVectorTileProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( u"password"_s, QgsVectorTileProviderConnection::settingsPassword->value( connections[i] ) );
    el.setAttribute( u"styleUrl"_s, QgsVectorTileProviderConnection::settingsStyleUrl->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsVectorTileProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveTiledSceneConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsTiledSceneConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"tiledscene"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"provider"_s, QgsTiledSceneProviderConnection::settingsProvider->value( connections[i] ) );
    el.setAttribute( u"url"_s, QgsTiledSceneProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( u"authcfg"_s, QgsTiledSceneProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( u"username"_s, QgsTiledSceneProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( u"password"_s, QgsTiledSceneProviderConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsTiledSceneProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveSensorThingsConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsSensorThingsConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"sensorthings"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsSensorThingsProviderConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( u"authcfg"_s, QgsSensorThingsProviderConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( u"username"_s, QgsSensorThingsProviderConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( u"password"_s, QgsSensorThingsProviderConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsTiledSceneProviderConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

  return doc;
}


QDomDocument QgsManageConnectionsDialog::saveCloudStorageConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsCloudStorageConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"cloudstorage"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"handler"_s, QgsGdalCloudProviderConnection::settingsVsiHandler->value( connections[i] ) );
    el.setAttribute( u"container"_s, QgsGdalCloudProviderConnection::settingsContainer->value( connections[i] ) );
    el.setAttribute( u"path"_s, QgsGdalCloudProviderConnection::settingsPath->value( connections[i] ) );

    const QVariantMap credentialOptions = QgsGdalCloudProviderConnection::settingsCredentialOptions->value( connections[i] );
    QString credentialString;
    for ( auto it = credentialOptions.constBegin(); it != credentialOptions.constEnd(); ++it )
    {
      if ( !it.value().toString().isEmpty() )
      {
        credentialString += u"|credential:%1=%2"_s.arg( it.key(), it.value().toString() );
      }
    }
    el.setAttribute( u"credentials"_s, credentialString );

    root.appendChild( el );
  }

  return doc;
}

QDomDocument QgsManageConnectionsDialog::saveStacConnections( const QStringList &connections )
{
  QDomDocument doc( u"connections"_s );
  QDomElement root = doc.createElement( u"qgsStacConnections"_s );
  root.setAttribute( u"version"_s, u"1.0"_s );
  doc.appendChild( root );

  QMap<QString, QString> namespaceDeclarations;
  for ( int i = 0; i < connections.count(); ++i )
  {
    QDomElement el = doc.createElement( u"stac"_s );

    el.setAttribute( u"name"_s, connections[i] );
    el.setAttribute( u"url"_s, QgsStacConnection::settingsUrl->value( connections[i] ) );
    el.setAttribute( u"authcfg"_s, QgsStacConnection::settingsAuthcfg->value( connections[i] ) );
    el.setAttribute( u"username"_s, QgsStacConnection::settingsUsername->value( connections[i] ) );
    el.setAttribute( u"password"_s, QgsStacConnection::settingsPassword->value( connections[i] ) );

    QgsHttpHeaders httpHeader( QgsStacConnection::settingsHeaders->value( connections[i] ) );
    httpHeader.updateDomElement( el, namespaceDeclarations );

    root.appendChild( el );
  }

  addNamespaceDeclarations( root, namespaceDeclarations );

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
    connectionName = child.attribute( u"name"_s );
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
    QgsOwsConnection::settingsUrl->setValue( child.attribute( u"url"_s ), { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreGetMapURI->setValue( child.attribute( u"ignoreGetMapURI"_s ) == "true"_L1, { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->setValue( child.attribute( u"ignoreGetFeatureInfoURI"_s ) == "true"_L1, { service.toLower(), connectionName } );
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( child.attribute( u"ignoreAxisOrientation"_s ) == "true"_L1, { service.toLower(), connectionName } );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( child.attribute( u"invertAxisOrientation"_s ) == "true"_L1, { service.toLower(), connectionName } );
    QgsOwsConnection::settingsSmoothPixmapTransform->setValue( child.attribute( u"smoothPixmapTransform"_s ) == "true"_L1, { service.toLower(), connectionName } );
    QgsOwsConnection::settingsDpiMode->setValue( static_cast<Qgis::DpiMode>( child.attribute( u"dpiMode"_s, u"7"_s ).toInt() ), { service.toLower(), connectionName } );

    QgsHttpHeaders httpHeader( child );
    QgsOwsConnection::settingsHeaders->setValue( httpHeader.headers(), { service.toLower(), connectionName } );

    if ( !child.attribute( u"username"_s ).isEmpty() )
    {
      QgsOwsConnection::settingsUsername->setValue( child.attribute( u"username"_s ), { service.toUpper(), connectionName } );
      QgsOwsConnection::settingsPassword->setValue( child.attribute( u"password"_s ), { service.toUpper(), connectionName } );
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadWfsConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsWFSConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a WFS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QStringList keys = QgsOwsConnection::sTreeOwsConnections->items( { u"wfs"_s } );

  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsOwsConnection::settingsUrl->setValue( child.attribute( u"url"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsVersion->setValue( child.attribute( u"version"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsMaxNumFeatures->setValue( child.attribute( u"maxnumfeatures"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPagesize->setValue( child.attribute( u"pagesize"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPagingEnabled->setValue( child.attribute( u"pagingenabled"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( child.attribute( u"ignoreAxisOrientation"_s ).toInt(), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( child.attribute( u"invertAxisOrientation"_s ).toInt(), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPreferredHttpMethod->setValue( child.attribute( u"httpMethod"_s ).compare( "post"_L1, Qt::CaseInsensitive ) == 0 ? Qgis::HttpMethod::Post : Qgis::HttpMethod::Get, { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsWfsFeatureMode->setValue( child.attribute( u"featureMode"_s ), { u"wfs"_s, connectionName } );

    if ( !child.attribute( u"username"_s ).isEmpty() )
    {
      QgsOwsConnection::settingsUsername->setValue( child.attribute( u"username"_s ), { u"wfs"_s, connectionName } );
      QgsOwsConnection::settingsPassword->setValue( child.attribute( u"password"_s ), { u"wfs"_s, connectionName } );
    }
    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadPgConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsPgConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a PostGIS connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/PostgreSQL/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    settings.setValue( u"/host"_s, child.attribute( u"host"_s ) );
    settings.setValue( u"/port"_s, child.attribute( u"port"_s ) );
    settings.setValue( u"/database"_s, child.attribute( u"database"_s ) );
    if ( child.hasAttribute( u"service"_s ) )
    {
      settings.setValue( u"/service"_s, child.attribute( u"service"_s ) );
    }
    else
    {
      settings.setValue( u"/service"_s, "" );
    }
    settings.setValue( u"/sslmode"_s, child.attribute( u"sslmode"_s ) );
    settings.setValue( u"/estimatedMetadata"_s, child.attribute( u"estimatedMetadata"_s ) );
    settings.setValue( u"/projectsInDatabase"_s, child.attribute( u"projectsInDatabase"_s, 0 ) );
    settings.setValue( u"/dontResolveType"_s, child.attribute( u"dontResolveType"_s, 0 ) );
    settings.setValue( u"/allowGeometrylessTables"_s, child.attribute( u"allowGeometrylessTables"_s, 0 ) );
    settings.setValue( u"/geometryColumnsOnly"_s, child.attribute( u"geometryColumnsOnly"_s, 0 ) );
    settings.setValue( u"/publicOnly"_s, child.attribute( u"publicOnly"_s, 0 ) );
    settings.setValue( u"/saveUsername"_s, child.attribute( u"saveUsername"_s ) );
    settings.setValue( u"/username"_s, child.attribute( u"username"_s ) );
    settings.setValue( u"/savePassword"_s, child.attribute( u"savePassword"_s ) );
    settings.setValue( u"/password"_s, child.attribute( u"password"_s ) );
    settings.setValue( u"/schema"_s, child.attribute( u"schema"_s ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadMssqlConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsMssqlConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a MS SQL Server connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/MSSQL/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    settings.setValue( u"/host"_s, child.attribute( u"host"_s ) );
    settings.setValue( u"/port"_s, child.attribute( u"port"_s ) );
    settings.setValue( u"/database"_s, child.attribute( u"database"_s ) );
    if ( child.hasAttribute( u"service"_s ) )
    {
      settings.setValue( u"/service"_s, child.attribute( u"service"_s ) );
    }
    else
    {
      settings.setValue( u"/service"_s, "" );
    }
    settings.setValue( u"/sslmode"_s, child.attribute( u"sslmode"_s ) );
    settings.setValue( u"/estimatedMetadata"_s, child.attribute( u"estimatedMetadata"_s ) );
    settings.setValue( u"/saveUsername"_s, child.attribute( u"saveUsername"_s ) );
    settings.setValue( u"/username"_s, child.attribute( u"username"_s ) );
    settings.setValue( u"/savePassword"_s, child.attribute( u"savePassword"_s ) );
    settings.setValue( u"/password"_s, child.attribute( u"password"_s ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadOracleConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsOracleConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not an Oracle connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/Oracle/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    settings.setValue( u"/host"_s, child.attribute( u"host"_s ) );
    settings.setValue( u"/port"_s, child.attribute( u"port"_s ) );
    settings.setValue( u"/database"_s, child.attribute( u"database"_s ) );
    settings.setValue( u"/dboptions"_s, child.attribute( u"dboptions"_s ) );
    settings.setValue( u"/dbworkspace"_s, child.attribute( u"dbworkspace"_s ) );
    settings.setValue( u"/schema"_s, child.attribute( u"schema"_s ) );
    settings.setValue( u"/estimatedMetadata"_s, child.attribute( u"estimatedMetadata"_s ) );
    settings.setValue( u"/userTablesOnly"_s, child.attribute( u"userTablesOnly"_s ) );
    settings.setValue( u"/geometryColumnsOnly"_s, child.attribute( u"geometryColumnsOnly"_s ) );
    settings.setValue( u"/allowGeometrylessTables"_s, child.attribute( u"allowGeometrylessTables"_s ) );
    settings.setValue( u"/saveUsername"_s, child.attribute( u"saveUsername"_s ) );
    settings.setValue( u"/username"_s, child.attribute( u"username"_s ) );
    settings.setValue( u"/savePassword"_s, child.attribute( u"savePassword"_s ) );
    settings.setValue( u"/password"_s, child.attribute( u"password"_s ) );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadHanaConnections( const QDomDocument &doc, const QStringList &items )
{
  QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsHanaConnections"_L1 )
  {
    QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The file is not a HANA connections exchange file." ) );
    return;
  }

  const QDomAttr version = root.attributeNode( "version" );
  if ( version.value() != "1.0"_L1 )
  {
    QMessageBox::warning( this, tr( "Loading Connections" ), tr( "The HANA connections exchange file version '%1' is not supported." ).arg( version.value() ) );
    return;
  }

  QgsSettings settings;
  settings.beginGroup( u"/HANA/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    const QString connectionName = child.attribute( u"name"_s );
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
      settings.setValue( u"/"_s + param, child.attribute( param ) );

    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadXyzTilesConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsXYZTilesConnections"_L1 )
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
    connectionName = child.attribute( u"name"_s );
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


    QgsXyzConnectionSettings::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );
    QgsXyzConnectionSettings::settingsZmin->setValue( child.attribute( u"zmin"_s ).toInt(), connectionName );
    QgsXyzConnectionSettings::settingsZmax->setValue( child.attribute( u"zmax"_s ).toInt(), connectionName );
    QgsXyzConnectionSettings::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );
    QgsXyzConnectionSettings::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsXyzConnectionSettings::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );
    QgsXyzConnectionSettings::settingsTilePixelRatio->setValue( child.attribute( u"tilePixelRatio"_s ).toInt(), connectionName );

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
    connectionName = child.attribute( u"name"_s );
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
    QgsArcGisConnectionSettings::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );

    QgsArcGisConnectionSettings::settingsHeaders->setValue( QgsHttpHeaders( child ).headers(), connectionName );


    QgsArcGisConnectionSettings::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsArcGisConnectionSettings::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );
    QgsArcGisConnectionSettings::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadVectorTileConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsVectorTileConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a Vector Tile connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/qgis/connections-vector-tile"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsVectorTileProviderConnection::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );
    QgsVectorTileProviderConnection::settingsZmin->setValue( child.attribute( u"zmin"_s ).toInt(), connectionName );
    QgsVectorTileProviderConnection::settingsZmax->setValue( child.attribute( u"zmax"_s ).toInt(), connectionName );
    QgsVectorTileProviderConnection::settingsServiceType->setValue( child.attribute( u"serviceType"_s ), connectionName );
    QgsVectorTileProviderConnection::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );
    QgsVectorTileProviderConnection::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsVectorTileProviderConnection::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );
    QgsVectorTileProviderConnection::settingsStyleUrl->setValue( child.attribute( u"styleUrl"_s ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsVectorTileProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadTiledSceneConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsTiledSceneConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a tiled scene connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/qgis/connections-tiled-scene"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsTiledSceneProviderConnection::settingsProvider->setValue( child.attribute( u"provider"_s ), connectionName );
    QgsTiledSceneProviderConnection::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );
    QgsTiledSceneProviderConnection::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );
    QgsTiledSceneProviderConnection::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsTiledSceneProviderConnection::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsTiledSceneProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadSensorThingsConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsSensorThingsConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a SensorThings connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/connections/sensorthings/items"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsSensorThingsProviderConnection::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );
    QgsSensorThingsProviderConnection::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );
    QgsSensorThingsProviderConnection::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsSensorThingsProviderConnection::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );

    QgsHttpHeaders httpHeader( child );
    QgsSensorThingsProviderConnection::settingsHeaders->setValue( httpHeader.headers(), connectionName );

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadCloudStorageConnections( const QDomDocument &doc, const QStringList &items )
{
  const QDomElement root = doc.documentElement();
  if ( root.tagName() != "qgsCloudStorageConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a cloud storage connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/connections/cloud/items"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsGdalCloudProviderConnection::settingsVsiHandler->setValue( child.attribute( u"handler"_s ), connectionName );
    QgsGdalCloudProviderConnection::settingsContainer->setValue( child.attribute( u"container"_s ), connectionName );
    QgsGdalCloudProviderConnection::settingsPath->setValue( child.attribute( u"path"_s ), connectionName );

    QString credentialString = child.attribute( u"credentials"_s );

    QVariantMap credentialOptions;
    while ( true )
    {
      const thread_local QRegularExpression credentialOptionRegex( u"\\|credential:([^|]*)"_s );
      const thread_local QRegularExpression credentialOptionKeyValueRegex( u"(.*?)=(.*)"_s );

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
  if ( root.tagName() != "qgsStacConnections"_L1 )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a STAC connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/qgis/connections-stac"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  QDomElement child = root.firstChildElement();
  bool prompt = true;
  bool overwrite = true;

  while ( !child.isNull() )
  {
    connectionName = child.attribute( u"name"_s );
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

    QgsStacConnection::settingsUrl->setValue( child.attribute( u"url"_s ), connectionName );
    QgsStacConnection::settingsAuthcfg->setValue( child.attribute( u"authcfg"_s ), connectionName );
    QgsStacConnection::settingsUsername->setValue( child.attribute( u"username"_s ), connectionName );
    QgsStacConnection::settingsPassword->setValue( child.attribute( u"password"_s ), connectionName );

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
