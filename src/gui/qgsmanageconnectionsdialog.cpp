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

#include "qgsfileutils.h"
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
#include <QString>
#include <QTextStream>

#include "moc_qgsmanageconnectionsdialog.cpp"

using namespace Qt::StringLiterals;

namespace
{
  enum class ActionOnDuplicate
  {
    Overwrite, //!< Overwrite duplicate connection
    Skip,      //!< Skip duplicate connection
    Cancel     //!< Cancel import
  };

  static bool loadDocument( QWidget *parent, const QString &fileName, QDomDocument &document )
  {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QMessageBox::warning(
        parent,
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Loading Connections" ),
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Cannot read file %1:\n%2." ).arg( fileName, file.errorString() )
      );
      return false;
    }

    QString errorString;
    int errorLine;
    int errorColumn;

    if ( !document.setContent( &file, true, &errorString, &errorLine, &errorColumn ) )
    {
      QMessageBox::warning(
        parent,
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Loading Connections" ),
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Parse error at line %1, column %2:\n%3" ).arg( errorLine ).arg( errorColumn ).arg( errorString )
      );
      return false;
    }

    return true;
  }

  static ActionOnDuplicate checkOverwritePrompt( QWidget *parent, const QString &connectionName, bool &prompt, bool &overwrite )
  {
    if ( prompt )
    {
      const int result = QMessageBox::warning(
        parent,
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Loading Connections" ),
        QCoreApplication::translate( "QgsManageConnectionsDialog", "Connection with name '%1' already exists. Overwrite?" ).arg( connectionName ),
        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel
      );
      switch ( result )
      {
        case QMessageBox::Yes:
          overwrite = true;
          break;
        case QMessageBox::YesToAll:
          prompt = false;
          overwrite = true;
          break;
        case QMessageBox::No:
          overwrite = false;
          break;
        case QMessageBox::NoToAll:
          prompt = false;
          overwrite = false;
          break;
        case QMessageBox::Cancel:
        default:
          return ActionOnDuplicate::Cancel;
      }
    }
    return overwrite ? ActionOnDuplicate::Overwrite : ActionOnDuplicate::Skip;
  }

  /**
 * Writes database connection details to XML element.
 * \param element the target XML element
 * \param settings the settings object to read connection details from
 * \param path the base group path for the target connection (e.g., "/PostgreSQL/connections/my_conn").
 * \param keys list of QgsSettings keys to save as XML attributes. Attribute names will match the setting keys.
 * \param defaults optional map of "settings key - default value" pairs used if a setting does not exist in \a settings.
 *
 * \note Credential keys (e.g. "saveUsername", "username", "savePassword", "password") should not be included
 *       in the \a keys list; they are handled automatically.
 */
  static void saveDatabaseConnection( QDomElement &element, const QgsSettings &settings, const QString &path, const QStringList &keys, const QVariantMap &defaults = {} )
  {
    for ( const QString &key : keys )
    {
      const QVariant defaultValue = defaults.value( key, QString() );
      element.setAttribute( key, settings.value( path + u'/' + key, defaultValue ).toString() );
    }

    const bool saveUsername = settings.value( path + "/saveUsername"_L1, "false"_L1 ).toString() == "true"_L1;
    element.setAttribute( u"saveUsername"_s, saveUsername ? u"true"_s : u"false"_s );
    if ( saveUsername )
    {
      element.setAttribute( u"username"_s, settings.value( path + "/username"_L1 ).toString() );
    }
    const bool savePassword = settings.value( path + "/savePassword"_L1, "false"_L1 ).toString() == "true"_L1;
    element.setAttribute( u"savePassword"_s, savePassword ? u"true"_s : u"false"_s );
    if ( savePassword )
    {
      element.setAttribute( u"password"_s, settings.value( path + "/password"_L1 ).toString() );
    }
  }

  /**
 * Loads database connection settings from an XML DOM \a element into \a settings.
 *
 * \param element the source XML DOM element containing connection attributes.
 * \param settings the QgsSettings instance where settings will be saved (caller should set up current group).
 * \param keys list of attribute names to read from \a element and save into \a settings.
 * \param defaults Optional map of key-default value pairs . If an attribute in \a keys is missing from \a element,
 * its corresponding value in \a defaults will be used instead. Keys not present in \a defaults will fall back to an empty string.
 *
 * \note Credential keys (e.g. "saveUsername", "username", "savePassword", "password") should not be included
 *       in the \a keys list; they are handled automatically.
 */
  static void loadDatabaseConnection( const QDomElement &element, QgsSettings &settings, const QStringList &keys, const QVariantMap &defaults = {} )
  {
    for ( const QString &key : keys )
    {
      const QString defaultValue = defaults.value( key, QString() ).toString();
      settings.setValue( u'/' + key, element.attribute( key, defaultValue ) );
    }

    settings.setValue( u"/saveUsername"_s, element.attribute( u"saveUsername"_s ) );
    settings.setValue( u"/username"_s, element.attribute( u"username"_s ) );
    settings.setValue( u"/savePassword"_s, element.attribute( u"savePassword"_s ) );
    settings.setValue( u"/password"_s, element.attribute( u"password"_s ) );
  }

  static QString rootTagForConnectionType( QgsManageConnectionsDialog::Type type )
  {
    switch ( type )
    {
      case QgsManageConnectionsDialog::WMS:
        return u"qgsWMSConnections"_s;
      case QgsManageConnectionsDialog::WFS:
        return u"qgsWFSConnections"_s;
      case QgsManageConnectionsDialog::WCS:
        return u"qgsWCSConnections"_s;
      case QgsManageConnectionsDialog::PostGIS:
        return u"qgsPgConnections"_s;
      case QgsManageConnectionsDialog::MSSQL:
        return u"qgsMssqlConnections"_s;
      case QgsManageConnectionsDialog::Oracle:
        return u"qgsOracleConnections"_s;
      case QgsManageConnectionsDialog::HANA:
        return u"qgsHanaConnections"_s;
      case QgsManageConnectionsDialog::XyzTiles:
        return u"qgsXyzTilesConnections"_s;
      case QgsManageConnectionsDialog::ArcgisFeatureServer:
      case QgsManageConnectionsDialog::ArcgisMapServer:
        return u"qgsArcgisConnections"_s;
      case QgsManageConnectionsDialog::VectorTile:
        return u"qgsVectorTileConnections"_s;
      case QgsManageConnectionsDialog::TiledScene:
        return u"qgsTiledSceneConnections"_s;
      case QgsManageConnectionsDialog::SensorThings:
        return u"qgsSensorThingsConnections"_s;
      case QgsManageConnectionsDialog::CloudStorage:
        return u"qgsCloudStorageConnections"_s;
      case QgsManageConnectionsDialog::STAC:
        return u"qgsStacConnections"_s;
    }
    return QString();
  }
  static bool checkRootTag( QWidget *parent, const QDomDocument &document, const QString &expectedTag )
  {
    const QDomElement root = document.documentElement();
    if ( expectedTag.isEmpty() || root.tagName() != expectedTag )
    {
      QMessageBox::information( parent, QCoreApplication::translate( "QgsManageConnectionsDialog", "Loading Connections" ), QCoreApplication::translate( "QgsManageConnectionsDialog", "The file is not a valid connections exchange file for the selected service." ) );
      return false;
    }
    return true;
  }

  static bool isValidDocumentType( QWidget *parent, const QDomDocument &document, QgsManageConnectionsDialog::Type type )
  {
    return checkRootTag( parent, document, rootTagForConnectionType( type ) );
  }

  static bool isValidDocumentType( QWidget *parent, const QDomDocument &document, const QString &service )
  {
    return checkRootTag( parent, document, u"qgs"_s + service.toUpper() + u"Connections"_s );
  }

  static void addNamespaceDeclarations( QDomElement &root, const QMap<QString, QString> &namespaceDeclarations )
  {
    for ( auto it = namespaceDeclarations.begin(); it != namespaceDeclarations.end(); ++it )
    {
      root.setAttribute( u"xmlns:"_s + it.key(), it.value() );
    }
  }
} // namespace

///
/// QgsManageConnectionsDialog
///

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


    fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { "xml" } );
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
    QDomDocument doc;

    if ( !loadDocument( this, mFileName, doc ) )
    {
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
    QDomDocument doc;

    if ( !loadDocument( this, mFileName, doc ) )
    {
      return false;
    }

    if ( !isValidDocumentType( this, doc, mConnectionType ) )
    {
      return false;
    }

    const QDomElement root = doc.documentElement();
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
    QDomElement element = doc.createElement( u"postgis"_s );
    saveDatabaseConnection(
      element,
      settings,
      path,
      { u"name"_s
        u"host"_s,
        u"port"_s,
        u"database"_s,
        u"service"_s,
        u"sslmode"_s,
        u"estimatedMetadata"_s,
        u"projectsInDatabase"_s,
        u"dontResolveType"_s,
        u"allowGeometrylessTables"_s,
        u"geometryColumnsOnly"_s,
        u"publicOnly"_s,
        u"schema"_s },
      { { u"sslmode"_s, "1" },
        { u"estimatedMetadata"_s, "0" },
        { u"projectsInDatabase"_s, "0" },
        { u"dontResolveType"_s, "0" },
        { u"allowGeometrylessTables"_s, "0" },
        { u"geometryColumnsOnly"_s, "0" },
        { u"publicOnly"_s, "0" },
        {} }
    );

    root.appendChild( element );
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
    QDomElement element = doc.createElement( u"mssql"_s );
    saveDatabaseConnection(
      element,
      settings,
      path,
      { u"name"_s
        u"host"_s,
        u"port"_s,
        u"database"_s,
        u"service"_s,
        u"sslmode"_s,
        u"estimatedMetadata"_s },
      { { u"sslmode"_s, "1" }, { u"estimatedMetadata"_s, "0" } }
    );
    root.appendChild( element );
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
    QDomElement element = doc.createElement( u"oracle"_s );
    saveDatabaseConnection(
      element,
      settings,
      path,
      { u"name"_s
        u"host"_s,
        u"port"_s,
        u"database"_s,
        u"dboptions"_s,
        u"dbworkspace"_s,
        u"schema"_s,
        u"estimatedMetadata"_s,
        u"userTablesOnly"_s,
        u"geometryColumnsOnly"_s,
        u"allowGeometrylessTables"_s },
      { { u"estimatedMetadata"_s, "0" }, { u"userTablesOnly"_s, "0" }, { u"geometryColumnsOnly"_s, "0" }, { u"allowGeometrylessTables"_s, "0" } }
    );

    root.appendChild( element );
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
    QDomElement element = doc.createElement( u"hana"_s );
    saveDatabaseConnection(
      element,
      settings,
      path,
      { u"name"_s
        u"driver"_s,
        u"host"_s,
        u"identifierType"_s,
        u"identifier"_s,
        u"multitenant"_s,
        u"database"_s,
        u"schema"_s,
        u"userTablesOnly"_s,
        u"allowGeometrylessTables"_s,
        u"sslEnabled"_s,
        u"sslCryptoProvider"_s,
        u"sslKeyStore"_s,
        u"sslTrustStore"_s,
        u"sslValidateCertificate"_s,
        u"sslHostNameInCertificate"_s },
      { { u"userTablesOnly"_s, "0" }, { u"allowGeometrylessTables"_s, "0" }, { u"sslEnabled"_s, u"false"_s }, { u"sslCryptoProvider"_s, u"openssl"_s }, { u"sslValidateCertificate"_s, u"false"_s } }
    );
    root.appendChild( element );
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

    QgsHttpHeaders httpHeader( QgsSensorThingsProviderConnection::settingsHeaders->value( connections[i] ) );
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
  if ( !isValidDocumentType( this, doc, service ) )
  {
    return;
  }

  QString connectionName;
  const QDomElement root = doc.documentElement();
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

    if ( QgsOwsConnection::settingsUrl->exists( { service.toLower(), connectionName } ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }

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
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::WFS ) )
  {
    return;
  }

  QString connectionName;
  QStringList keys = QgsOwsConnection::sTreeOwsConnections->items( { u"wfs"_s } );

  const QDomElement root = doc.documentElement();
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
    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

    QgsOwsConnection::settingsUrl->setValue( child.attribute( u"url"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsVersion->setValue( child.attribute( u"version"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsMaxNumFeatures->setValue( child.attribute( u"maxnumfeatures"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPagesize->setValue( child.attribute( u"pagesize"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPagingEnabled->setValue( child.attribute( u"pagingenabled"_s ), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( child.attribute( u"ignoreAxisOrientation"_s ).toInt(), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( child.attribute( u"invertAxisOrientation"_s ).toInt(), { u"wfs"_s, connectionName } );
    QgsOwsConnection::settingsPreferredHttpMethod
      ->setValue( child.attribute( u"httpMethod"_s ).compare( "post"_L1, Qt::CaseInsensitive ) == 0 ? Qgis::HttpMethod::Post : Qgis::HttpMethod::Get, { u"wfs"_s, connectionName } );
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
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::PostGIS ) )
  {
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/PostgreSQL/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

    settings.beginGroup( "/PostgreSQL/connections/" + connectionName );
    loadDatabaseConnection(
      child,
      settings,
      { u"host"_s,
        u"port"_s,
        u"database"_s,
        u"ervice"_s,
        u"sslmode"_s,
        u"estimatedMetadata"_s,
        u"projectsInDatabase"_s,
        u"dontResolveType"_s,
        u"allowGeometrylessTables"_s,
        u"geometryColumnsOnly"_s,
        u"publicOnly"_s,
        u"schema"_s },
      { { u"service"_s, "" }, { u"projectsInDatabase"_s, 0 }, { u"dontResolveType"_s, 0 }, { u"allowGeometrylessTables"_s, 0 }, { u"geometryColumnsOnly"_s, 0 }, { u"publicOnly"_s, 0 } }
    );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadMssqlConnections( const QDomDocument &doc, const QStringList &items )
{
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::MSSQL ) )
  {
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/MSSQL/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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
    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

    settings.beginGroup( "/MSSQL/connections/" + connectionName );
    loadDatabaseConnection( child, settings, { u"host"_s, u"port"_s, u"database"_s, u"service"_s, u"sslmode"_s, u"estimatedMetadata"_s }, { { u"service"_s, "" } } );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadOracleConnections( const QDomDocument &doc, const QStringList &items )
{
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::Oracle ) )
  {
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/Oracle/connections"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

    settings.beginGroup( "/Oracle/connections/" + connectionName );
    loadDatabaseConnection( child, settings, { u"host"_s, u"port"_s, u"database"_s, u"dboptions"_s, u"dbworkspace"_s, u"schema"_s, u"estimatedMetadata"_s, u"userTablesOnly"_s, u"geometryColumnsOnly"_s, u"allowGeometrylessTables"_s } );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadHanaConnections( const QDomDocument &doc, const QStringList &items )
{
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::HANA ) )
  {
    return;
  }

  QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

    settings.beginGroup( "/HANA/connections/" + connectionName );
    loadDatabaseConnection(
      child,
      settings,
      { u"driver"_s,
        u"host"_s,
        u"database"_s,
        u"identifierType"_s,
        u"identifier"_s,
        u"multitenant"_s,
        u"schema"_s,
        u"userTablesOnly"_s,
        u"allowGeometrylessTables"_s,
        u"saveUsername"_s,
        u"username"_s,
        u"savePassword"_s,
        u"password"_s,
        u"sslEnabled"_s,
        u"sslCryptoProvider"_s,
        u"sslKeyStore"_s,
        u"sslTrustStore"_s,
        u"sslValidateCertificate"_s,
        u"sslHostNameInCertificate"_s }
    );
    settings.endGroup();

    child = child.nextSiblingElement();
  }
}

void QgsManageConnectionsDialog::loadXyzTilesConnections( const QDomDocument &doc, const QStringList &items )
{
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::XyzTiles ) )
  {
    return;
  }

  QString connectionName;
  QStringList keys = QgsXyzConnectionSettings::sTreeXyzConnections->items();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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
  if ( !isValidDocumentType( this, doc, service ) )
  {
    return;
  }

  QString connectionName;
  QStringList keys = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
      }
    }
    else
    {
      keys << connectionName;
    }

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
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::VectorTile ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a Vector Tile connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/qgis/connections-vector-tile"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::TiledScene ) )
  {
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/qgis/connections-tiled-scene"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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
  if ( !isValidDocumentType( this, doc, QgsManageConnectionsDialog::SensorThings ) )
  {
    QMessageBox::information( this, tr( "Loading Connections" ), tr( "The file is not a SensorThings connections exchange file." ) );
    return;
  }

  QString connectionName;
  QgsSettings settings;
  settings.beginGroup( u"/connections/sensorthings/items"_s );
  QStringList keys = settings.childGroups();
  settings.endGroup();
  const QDomElement root = doc.documentElement();
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
    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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

    if ( keys.contains( connectionName ) )
    {
      switch ( checkOverwritePrompt( this, connectionName, prompt, overwrite ) )
      {
        case ActionOnDuplicate::Overwrite:
          break;
        case ActionOnDuplicate::Skip:
          child = child.nextSiblingElement();
          continue;
        case ActionOnDuplicate::Cancel:
          return;
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
