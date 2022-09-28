/***************************************************************************
  qgslandingpageutils.cpp - QgsLandingPageUtils

 ---------------------
 begin                : 3.8.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprojectstorage.h"
#include "qgsprojectviewsettings.h"
#include "qgsreferencedgeometry.h"
#include "qgslandingpageutils.h"
#include "qgsserverprojectutils.h"
#include "qgsconfigcache.h"
#include "qgsmessagelog.h"
#include "qgslayertree.h"
#include "qgsvectorlayer.h"
#include "nlohmann/json.hpp"
#include "qgscoordinatetransform.h"

#include <mutex>
#include <QCryptographicHash>
#include <QFileSystemWatcher>
#include <QDomDocument>

const QRegularExpression QgsLandingPageUtils::PROJECT_HASH_RE { QStringLiteral( "/(?<projectHash>[a-f0-9]{32})" ) };
QMap<QString, QString> QgsLandingPageUtils::AVAILABLE_PROJECTS;

std::once_flag initDirWatcher;

QMap<QString, QString> QgsLandingPageUtils::projects( const QgsServerSettings &settings )
{

  static QString QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES;
  static QString QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS;

  // Init directory watcher
  static QFileSystemWatcher dirWatcher;
  std::call_once( initDirWatcher, [ = ]
  {
    QObject::connect( &dirWatcher, &QFileSystemWatcher::directoryChanged, qApp, [ = ]( const QString & path )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Directory '%1' has changed: project information cache cleared." ).arg( path ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Info );
      AVAILABLE_PROJECTS.clear();
    } );
  } );


  const QString projectDir { settings.landingPageProjectsDirectories() };

  // Clear cache if QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES has changed
  if ( projectDir != QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES )
  {
    QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES = projectDir;
    AVAILABLE_PROJECTS.clear();
    const QStringList cWatchedDirs { dirWatcher.directories() };
    dirWatcher.removePaths( cWatchedDirs );
  }

  const QString pgConnections { settings.landingPageProjectsPgConnections() };

  // Clear cache if QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS has changed
  if ( pgConnections != QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS )
  {
    QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS = pgConnections;
    AVAILABLE_PROJECTS.clear();
  }

  // Scan QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES
  const QString envDirName = QgsServerSettings::name( QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES );
  if ( AVAILABLE_PROJECTS.isEmpty() )
  {
    const auto cProjectDirs { projectDir.split( QStringLiteral( "||" ) ) };
    for ( const auto &path : cProjectDirs )
    {
      if ( ! path.isEmpty() )
      {
        const QDir dir { path };
        if ( dir.exists() )
        {
          dirWatcher.addPath( dir.path() );
          const auto constFiles { dir.entryList( ) };
          for ( const auto &f : constFiles )
          {
            if ( f.endsWith( QStringLiteral( ".qgs" ), Qt::CaseSensitivity::CaseInsensitive ) ||
                 f.endsWith( QStringLiteral( ".qgz" ), Qt::CaseSensitivity::CaseInsensitive ) )
            {
              const QString fullPath { path + '/' + f };
              const auto projectHash { QCryptographicHash::hash( fullPath.toUtf8(), QCryptographicHash::Md5 ).toHex() };
              AVAILABLE_PROJECTS[ projectHash ] = fullPath;
              QgsMessageLog::logMessage( QStringLiteral( "Adding filesystem project '%1' with id '%2'" ).arg( QFileInfo( f ).fileName(), QString::fromUtf8( projectHash ) ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Info );
            }
          }
        }
        else
        {
          QgsMessageLog::logMessage( QStringLiteral( "%1 entry '%2' was not found: skipping." ).arg( envDirName, path ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "%1 empty path: skipping." ).arg( envDirName ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
      }
    }
  }

  // PG projects (there is no watcher for PG: scan every time)
  const QString envPgName = QgsServerSettings::name( QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS );
  const auto storage { QgsApplication::projectStorageRegistry()->projectStorageFromType( QStringLiteral( "postgresql" ) ) };
  Q_ASSERT( storage );
  const auto cPgConnections { pgConnections.split( QStringLiteral( "||" ) ) };
  for ( const auto &connectionString : cPgConnections )
  {
    if ( ! connectionString.isEmpty() )
    {
      const auto constProjects { storage->listProjects( connectionString ) };
      if ( ! constProjects.isEmpty() )
      {
        for ( const auto &projectName : constProjects )
        {
          const QString projectFullPath { connectionString + QStringLiteral( "&project=%1" ).arg( projectName ) };
          const auto projectHash { QCryptographicHash::hash( projectFullPath.toUtf8(), QCryptographicHash::Md5 ).toHex() };
          AVAILABLE_PROJECTS[ projectHash ] = projectFullPath;
          QgsMessageLog::logMessage( QStringLiteral( "Adding postgres project '%1' with id '%2'" ).arg( projectName, QString::fromUtf8( projectHash ) ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "%1 entry '%2' was not found or has not projects: skipping." ).arg( envPgName, connectionString ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "%1 empty connection: skipping." ).arg( envPgName ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
    }
  }

  return AVAILABLE_PROJECTS;
}

json QgsLandingPageUtils::projectInfo( const QString &projectUri, const QgsServerSettings *serverSettings, const QgsServerRequest &request )
{
  // Helper for QStringList
  auto jList = [ ]( const QStringList & l ) -> json
  {
    json a = json::array( );
    for ( const auto &e : std::as_const( l ) )
    {
      a.push_back( e.toStdString() );
    }
    return a;
  };

  auto jHash = [ ]( const QHash<QString, QString> &l ) -> json
  {
    json a = json::object();
    const auto &constKeys { l.keys() };
    for ( const auto &k : constKeys )
    {
      a[ k.toStdString() ] = l[ k ].toStdString();
    }
    return a;
  };

  auto jContactList = [ ]( const QgsAbstractMetadataBase::ContactList & contacts ) -> json
  {
    json jContacts = json::array();
    for ( const auto &c : contacts )
    {
      json jContact
      {
        { "name", c.name.toStdString() },
        { "role", c.role.toStdString() },
        { "voice", c.voice.toStdString() },
        { "fax", c.fax.toStdString() },
        { "email", c.email.toStdString() },
        { "position", c.position.toStdString() },
        { "organization", c.organization.toStdString() },
        { "addresses", json::array() }
      };
      // Addresses
      const auto &cAddresses { c.addresses };
      for ( const auto &a : cAddresses )
      {
        jContact[ "addresses" ].push_back(
        {
          { "address", a.address.toStdString()},
          { "type", a.type.toStdString() },
          { "city", a.city.toStdString() },
          { "country", a.country.toStdString() },
          { "postalCode", a.postalCode.toStdString() },
          { "administrativeArea", a.administrativeArea.toStdString() }
        } );
      }
      jContacts.push_back( jContact );
    }
    return jContacts;
  };

  auto jLinksList = [ ]( const QgsAbstractMetadataBase::LinkList & links ) -> json
  {
    json jLinks = json::array();
    for ( const auto &l : links )
    {
      jLinks.push_back(
      {
        { "name", l.name.toStdString() },
        { "url", l.url.toStdString()},
        { "description", l.description.toStdString()},
        { "type", l.type.toStdString()},
        { "mimeType", l.mimeType.toStdString()},
        { "format", l.format.toStdString()},
        { "size", l.size.toStdString()}
      } );
    }
    return jLinks;
  };

  json info = json::object();
  info[ "id" ] = QCryptographicHash::hash( projectUri.toUtf8(), QCryptographicHash::Md5 ).toHex();

  const QgsProject *p { QgsConfigCache::instance()->project( projectUri, serverSettings ) };

  if ( p )
  {

    // Initial extent for map display, in 4326 CRS.
    // Check view settings first, read map canvas extent from XML if it's not set
    const QgsProjectViewSettings *viewSettings { p->viewSettings() };
    if ( viewSettings && ! viewSettings->defaultViewExtent().isEmpty() )
    {
      QgsRectangle extent { viewSettings->defaultViewExtent() };
      // Need conversion?
      if ( viewSettings->defaultViewExtent().crs().authid() != QLatin1String( "EPSG:4326" ) )
      {
        QgsCoordinateTransform ct { p->crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), p->transformContext() };
        extent = ct.transform( extent );
      }
      info[ "initial_extent" ] = json::array( { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } );
    }
    else
      // Old projects do not have view extent information, we have no choice than
      // re-read the project and extract the information from there
    {
      QgsProject temporaryProject( nullptr, Qgis::ProjectCapabilities() );
      QObject::connect( &temporaryProject, &QgsProject::readProject, qApp, [ & ]( const QDomDocument & projectDoc )
      {
        const QDomNodeList canvasElements { projectDoc.elementsByTagName( QStringLiteral( "mapcanvas" ) ) };
        if ( ! canvasElements.isEmpty() )
        {
          const QDomNode canvasElement { canvasElements.item( 0 ).firstChildElement( QStringLiteral( "extent" ) ) };
          if ( !canvasElement.isNull() &&
               !canvasElement.firstChildElement( QStringLiteral( "xmin" ) ).isNull() &&
               !canvasElement.firstChildElement( QStringLiteral( "ymin" ) ).isNull() &&
               !canvasElement.firstChildElement( QStringLiteral( "xmax" ) ).isNull() &&
               !canvasElement.firstChildElement( QStringLiteral( "ymax" ) ).isNull()
             )
          {
            QgsRectangle extent
            {
              canvasElement.firstChildElement( QStringLiteral( "xmin" ) ).text().toDouble(),
              canvasElement.firstChildElement( QStringLiteral( "ymin" ) ).text().toDouble(),
              canvasElement.firstChildElement( QStringLiteral( "xmax" ) ).text().toDouble(),
              canvasElement.firstChildElement( QStringLiteral( "ymax" ) ).text().toDouble(),
            };
            // Need conversion?
            if ( temporaryProject.crs().authid() != QLatin1String( "EPSG:4326" ) )
            {
              QgsCoordinateTransform ct { temporaryProject.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), temporaryProject.transformContext() };
              extent = ct.transform( extent );
            }
            info[ "initial_extent" ] = json::array( { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } );
          }
        }
      } );

      QgsMessageLog::logMessage( QStringLiteral( "The project '%1' was saved with a version of QGIS which does not contain initial extent information. "
                                 "For better performances consider re-saving the project with the latest version of QGIS." )
                                 .arg( projectUri ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
      temporaryProject.read( projectUri );
    }

    // Title
    QString title { p->metadata().title() };
    if ( title.isEmpty() )
      title = QgsServerProjectUtils::owsServiceTitle( *p );
    info["title"] = title.toStdString();
    // Description
    QString description { p->metadata().abstract() };
    if ( description.isEmpty() )
      description = QgsServerProjectUtils::owsServiceAbstract( *p );
    info["description"] = description.toStdString();
    // CRS
    const QStringList wmsOutputCrsList { QgsServerProjectUtils::wmsOutputCrsList( *p ) };
    const QString crs = wmsOutputCrsList.contains( QStringLiteral( "EPSG:4326" ) ) || wmsOutputCrsList.isEmpty() ?
                        QStringLiteral( "EPSG:4326" ) : wmsOutputCrsList.first();
    info["crs"] = crs.toStdString();
    // Typenames for WMS
    const bool useIds { QgsServerProjectUtils::wmsUseLayerIds( *p ) };
    QStringList typenames;
    const QStringList wmsRestrictedLayers { QgsServerProjectUtils::wmsRestrictedLayers( *p ) };
    const auto constLayers { p->mapLayers().values( ) };
    for ( const auto &l : constLayers )
    {
      if ( ! wmsRestrictedLayers.contains( l->name() ) )
      {
        typenames.push_back( useIds ? l->id() : l->name() );
      }
    }
    // Extent
    QgsRectangle extent { QgsServerProjectUtils::wmsExtent( *p ) };
    QgsCoordinateReferenceSystem targetCrs;
    if ( crs.split( ':' ).count() == 2 )
      targetCrs = QgsCoordinateReferenceSystem::fromEpsgId( crs.split( ':' ).last().toLong() );
    if ( extent.isNull() && crs.split( ':' ).count() == 2 )
    {
      for ( const auto &l : constLayers )
      {
        if ( ! wmsRestrictedLayers.contains( l->name() ) )
        {
          QgsRectangle layerExtent { l->extent() };
          if ( l->crs() != targetCrs && targetCrs.isValid() )
          {
            QgsCoordinateTransform ct { l->crs(), targetCrs, p->transformContext() };
            layerExtent = ct.transform( layerExtent );
          }
          if ( extent.isNull() )
          {
            extent = layerExtent;
          }
          else
          {
            extent.combineExtentWith( layerExtent );
          }
        }
      }
    }
    else if ( ! extent.isNull() )
    {
      if ( targetCrs.isValid() && targetCrs != p->crs() )
      {
        QgsCoordinateTransform ct { p->crs(), targetCrs, p->transformContext() };
        extent = ct.transform( extent );
      }
    }
    info["extent"] = json::array( { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } );
    QgsRectangle geographicExtent { extent };
    if ( targetCrs.authid() != QLatin1String( "EPSG:4326" ) )
    {
      QgsCoordinateTransform ct { targetCrs,  QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), p->transformContext() };
      geographicExtent = ct.transform( geographicExtent );
    }
    info["geographic_extent"] = json::array( { geographicExtent.xMinimum(), geographicExtent.yMinimum(), geographicExtent.xMaximum(), geographicExtent.yMaximum() } );

    // Metadata
    json metadata;
    const QgsProjectMetadata &md { p->metadata() };
    metadata["title"] = md.title().toStdString();
    metadata["identifier"] = md.identifier().toStdString();
    metadata["parentIdentifier"] = md.parentIdentifier().toStdString();
    metadata["creationDateTime"] = md.creationDateTime().toString( Qt::DateFormat::ISODate ).toStdString();
    metadata["abstract"] = md.abstract().toStdString();
    metadata["author"] = md.author().toStdString();
    metadata["language"] = md.language().toStdString();
    metadata["categories"] = jList( md.categories() );
    metadata["history"] = jList( md.history() );
    metadata["type"] = md.type().toStdString();
    // Links
    metadata["links"] = jLinksList( md.links() );
    // Contacts
    metadata["contacts"] = jContactList( md.contacts() );
    info[ "metadata" ] = metadata;
    // Capabilities
    json capabilities = json::object();
    capabilities["owsServiceCapabilities"] = QgsServerProjectUtils::owsServiceCapabilities( *p );
    capabilities["owsServiceAbstract"] = QgsServerProjectUtils::owsServiceAbstract( *p ).toStdString();
    capabilities["owsServiceAccessConstraints"] = QgsServerProjectUtils::owsServiceAccessConstraints( *p ).toStdString();
    capabilities["owsServiceContactMail"] = QgsServerProjectUtils::owsServiceContactMail( *p ).toStdString();
    capabilities["owsServiceContactOrganization"] = QgsServerProjectUtils::owsServiceContactOrganization( *p ).toStdString();
    capabilities["owsServiceContactPerson"] = QgsServerProjectUtils::owsServiceContactPerson( *p ).toStdString();
    capabilities["owsServiceContactPhone"] = QgsServerProjectUtils::owsServiceContactPhone( *p ).toStdString();
    capabilities["owsServiceContactPosition"] = QgsServerProjectUtils::owsServiceContactPosition( *p ).toStdString();
    capabilities["owsServiceFees"] = QgsServerProjectUtils::owsServiceFees( *p ).toStdString();
    capabilities["owsServiceKeywords"] = jList( QgsServerProjectUtils::owsServiceKeywords( *p ) );
    capabilities["owsServiceOnlineResource"] = QgsServerProjectUtils::owsServiceOnlineResource( *p ).toStdString();
    capabilities["owsServiceTitle"] = QgsServerProjectUtils::owsServiceTitle( *p ).toStdString();
    capabilities["wcsLayerIds"] = jList( QgsServerProjectUtils::wcsLayerIds( *p ) );
    capabilities["wcsServiceUrl"] = QgsServerProjectUtils::wcsServiceUrl( *p, request, *serverSettings ).toStdString();
    capabilities["wfsLayerIds"] = jList( QgsServerProjectUtils::wfsLayerIds( *p ) );
    capabilities["wfsServiceUrl"] = QgsServerProjectUtils::wfsServiceUrl( *p, request, *serverSettings ).toStdString();
    capabilities["wfstDeleteLayerIds"] = jList( QgsServerProjectUtils::wfstDeleteLayerIds( *p ) );
    capabilities["wfstInsertLayerIds"] = jList( QgsServerProjectUtils::wfstInsertLayerIds( *p ) );
    capabilities["wfstUpdateLayerIds"] = jList( QgsServerProjectUtils::wfstUpdateLayerIds( *p ) );
    capabilities["wmsDefaultMapUnitsPerMm"] = QgsServerProjectUtils::wmsDefaultMapUnitsPerMm( *p );
    capabilities["wmsFeatureInfoAddWktGeometry"] = QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( *p );
    //capabilities["wmsExtent"] = info["extent"];
    capabilities["wmsFeatureInfoDocumentElement"] = QgsServerProjectUtils::wmsFeatureInfoDocumentElement( *p ).toStdString();
    capabilities["wmsFeatureInfoDocumentElementNs"] = QgsServerProjectUtils::wmsFeatureInfoDocumentElementNs( *p ).toStdString();
    capabilities["wmsFeatureInfoLayerAliasMap"] = jHash( QgsServerProjectUtils::wmsFeatureInfoLayerAliasMap( *p ) );
    capabilities["wmsFeatureInfoPrecision"] = QgsServerProjectUtils::wmsFeatureInfoPrecision( *p );
    capabilities["wmsFeatureInfoSchema"] = QgsServerProjectUtils::wmsFeatureInfoSchema( *p ).toStdString();
    capabilities["wmsFeatureInfoSegmentizeWktGeometry"] = QgsServerProjectUtils::wmsFeatureInfoSegmentizeWktGeometry( *p );
    capabilities["wmsImageQuality"] = QgsServerProjectUtils::wmsImageQuality( *p );
    capabilities["wmsInfoFormatSia2045"] = QgsServerProjectUtils::wmsInfoFormatSia2045( *p );
    capabilities["wmsInspireActivate"] = QgsServerProjectUtils::wmsInspireActivate( *p );
    capabilities["wmsInspireLanguage"] = QgsServerProjectUtils::wmsInspireLanguage( *p ).toStdString();
    capabilities["wmsInspireMetadataDate"] = QgsServerProjectUtils::wmsInspireMetadataDate( *p ).toStdString();
    capabilities["wmsInspireMetadataUrl"] = QgsServerProjectUtils::wmsInspireMetadataUrl( *p ).toStdString();
    capabilities["wmsInspireMetadataUrlType"] = QgsServerProjectUtils::wmsInspireMetadataUrlType( *p ).toStdString();
    capabilities["wmsInspireTemporalReference"] = QgsServerProjectUtils::wmsInspireTemporalReference( *p ).toStdString();
    capabilities["wmsMaxAtlasFeatures"] = QgsServerProjectUtils::wmsMaxAtlasFeatures( *p );
    capabilities["wmsMaxHeight"] = QgsServerProjectUtils::wmsMaxHeight( *p );
    capabilities["wmsMaxWidth"] = QgsServerProjectUtils::wmsMaxWidth( *p );
    capabilities["wmsOutputCrsList"] = jList( QgsServerProjectUtils::wmsOutputCrsList( *p ) );
    capabilities["wmsRestrictedComposers"] = jList( QgsServerProjectUtils::wmsRestrictedComposers( *p ) );
    capabilities["wmsRestrictedLayers"] = jList( QgsServerProjectUtils::wmsRestrictedLayers( *p ) );
    capabilities["wmsRootName"] = QgsServerProjectUtils::wmsRootName( *p ).toStdString();
    capabilities["wmsServiceUrl"] = QgsServerProjectUtils::wmsServiceUrl( *p, request, *serverSettings ).toStdString();
    capabilities["wmsTileBuffer"] = QgsServerProjectUtils::wmsTileBuffer( *p );
    capabilities["wmsUseLayerIds"] = QgsServerProjectUtils::wmsUseLayerIds( *p );
    capabilities["wmtsServiceUrl" ] = QgsServerProjectUtils::wmtsServiceUrl( *p, request, *serverSettings ).toStdString();
    info["capabilities"] = capabilities;
    // WMS layers
    info[ "wms_root_name" ] = QgsServerProjectUtils::wmsRootName( *p ).toStdString();
    if ( QgsServerProjectUtils::wmsRootName( *p ).isEmpty() )
    {
      info[ "wms_root_name" ] = p->title().toStdString();
    }
    json wmsLayers;
    // For convenience:
    // Map layer typename to layer id, if use_ids, it's an identity map
    json wmsLayersTypenameIdMap;
    // Map layer title to layer (short) name (or id if use_ids)
    json wmsLayersTitleIdMap;
    QStringList wmsLayersSearchable;
    QStringList wmsLayersQueryable;
    for ( const auto &l : constLayers )
    {
      if ( ! wmsRestrictedLayers.contains( l->name() ) )
      {
        json wmsLayer
        {
          { "name", l->name().toStdString() },
          { "id", l->id().toStdString() },
          { "crs", l->crs().authid().toStdString() },
          { "type", l->type() ==  QgsMapLayerType::VectorLayer ? "vector" : "raster" },
        };
        if ( l->type() == QgsMapLayerType::VectorLayer )
        {
          const QgsVectorLayer *vl = static_cast<const QgsVectorLayer *>( l );
          wmsLayer[ "pk" ] = vl->primaryKeyAttributes();
          int fieldIdx { 0 };
          json fieldsData;
          const QgsFields &cFields { vl->fields() };
          for ( const QgsField &field : cFields )
          {
            if ( field.configurationFlags().testFlag( QgsField::ConfigurationFlag::HideFromWfs ) )
            {
              ++fieldIdx;
              continue;
            }
            const QgsFieldConstraints::Constraints constraints { field.constraints().constraints() };
            const bool notNull = constraints & QgsFieldConstraints::Constraint::ConstraintNotNull &&
                                 field.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintNotNull ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard;
            const bool unique = constraints & QgsFieldConstraints::Constraint::ConstraintUnique &&
                                field.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintUnique ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard;
            const bool hasExpression = constraints & QgsFieldConstraints::Constraint::ConstraintExpression &&
                                       field.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintExpression ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard;
            const QString defaultValue = vl->dataProvider()->defaultValueClause( fieldIdx );
            const bool isReadOnly = notNull && unique && ! defaultValue.isEmpty();
            fieldsData[ field.name().toStdString() ] =
            {
              { "type", field.typeName().toStdString() },
              { "label", field.alias().isEmpty() ? field.name().toStdString() : field.alias().toStdString() },
              { "precision", field.precision() },
              { "length", field.length() },
              { "unique", unique },
              { "not_null", notNull },
              { "has_expression", hasExpression },
              { "default", defaultValue.toStdString() },
              { "expression", field.constraints().constraintExpression().toStdString() },
              { "editable", !isReadOnly }
            };

            ++fieldIdx;
          }
          wmsLayer[ "fields" ] = fieldsData;
        }
        wmsLayer[ "extent" ] = { l->extent().xMinimum(), l->extent().yMinimum(), l->extent().xMaximum(), l->extent().yMaximum() };

        // Fill maps
        const QString layerTitle { l->title().isEmpty() ? l->name() : l->title() };
        const QString shortName { ! l->shortName().isEmpty() ? l->shortName() : l->name() };
        const std::string layerIdentifier { useIds ? l->id().toStdString() : shortName.toStdString() };
        wmsLayersTypenameIdMap[ layerIdentifier ] = l->id().toStdString();
        wmsLayersTitleIdMap[ layerTitle.toStdString() ] = layerIdentifier;

        if ( l->flags() & QgsMapLayer::Searchable )
        {
          wmsLayersSearchable.push_back( l->id() );
        }
        if ( l->flags() & QgsMapLayer::Identifiable )
        {
          wmsLayersQueryable.push_back( l->id() );
        }

        // Layer metadata
        json layerMetadata;
        const auto &md { l->metadata() };
        layerMetadata[ "abstract" ] = md.abstract( ).toStdString();
        layerMetadata[ "categories" ] = jList( md.categories( ) );
        layerMetadata[ "contacts" ] = jContactList( md.contacts() );
        layerMetadata[ "encoding" ] = md.encoding( ).toStdString();
        layerMetadata[ "fees" ] = md.fees( ).toStdString();
        layerMetadata[ "history" ] = jList( md.history( ) );
        layerMetadata[ "identifier" ] = md.identifier( ).toStdString();
        layerMetadata[ "keywordVocabularies" ] = jList( md.keywordVocabularies( ) );
        // Keywords
        json jKeywords = json::object();
        const auto &cKw { md.keywords().keys() };
        const auto &cVals { md.keywords() };
        for ( const auto &k : cKw )
        {
          jKeywords[ k.toStdString() ] = jList( cVals[ k ] );
        }
        layerMetadata[ "keywords" ] = jKeywords;
        layerMetadata[ "language" ] = md.language( ).toStdString();
        layerMetadata[ "licenses" ] = jList( md.licenses( ) );
        layerMetadata[ "links" ] = jLinksList( md.links( ) );
        layerMetadata[ "parentIdentifier" ] = md.parentIdentifier( ).toStdString();
        layerMetadata[ "rights" ] = jList( md.rights( ) );
        layerMetadata[ "title" ] = md.title( ).toStdString();
        layerMetadata[ "type" ] = md.type( ).toStdString();
        layerMetadata[ "crs" ] = md.crs().authid().toStdString();
        const auto cConstraints {  md.constraints() };
        json constraints = json::array();
        for ( const auto &c : cConstraints )
        {
          constraints.push_back(
          {
            { "type", c.type.toStdString() },
            { "constraint", c.constraint.toStdString() },
          } );
        }
        layerMetadata[ "constraints" ] = constraints;
        wmsLayer[ "metadata" ] = layerMetadata;
        wmsLayers[ l->id().toStdString() ] = wmsLayer;
      }
    }

    info[ "wms_layers" ] = wmsLayers;
    info[ "wms_layers_map" ] = wmsLayersTitleIdMap;
    info[ "wms_layers_queryable" ] = jList( wmsLayersQueryable );
    info[ "wms_layers_searchable" ] = jList( wmsLayersSearchable );
    info[ "wms_layers_typename_id_map" ] = wmsLayersTypenameIdMap;
    info[ "toc" ] = layerTree( *p, wmsLayersQueryable, wmsLayersSearchable, wmsRestrictedLayers );

  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Could not read project '%1': skipping." ).arg( projectUri ), QStringLiteral( "Landing Page" ), Qgis::MessageLevel::Warning );
  }
  return info;
}

json QgsLandingPageUtils::layerTree( const QgsProject &project, const QStringList &wmsLayersQueryable, const QStringList &wmsLayersSearchable, const QStringList &wmsRestrictedLayers )
{
  const bool useIds { QgsServerProjectUtils::wmsUseLayerIds( project ) };
  const QStringList wfsLayerIds { QgsServerProjectUtils::wfsLayerIds( project ) };


  std::function<json( const QgsLayerTreeNode *, const QString & )> harvest = [ & ]( const QgsLayerTreeNode * node, const QString & parentId ) -> json
  {
    const std::string nodeName { parentId.isEmpty() ? "root" : node->name().toStdString() };
    QString title { QString::fromStdString( nodeName ) };
    QString nodeIdentifier = title;
    json rec {
      { "title", nodeName },
      { "name", nodeName },
      { "expanded", node->isExpanded() },
      { "visible", node->isVisible() },
    };
    if ( QgsLayerTree::isLayer( node ) )
    {
      const QgsLayerTreeLayer *l { static_cast<const QgsLayerTreeLayer *>( node ) };
      if ( l->layer() && ( l->layer()->type() == QgsMapLayerType::VectorLayer || l->layer()->type() == QgsMapLayerType::RasterLayer )
           && ! wmsRestrictedLayers.contains( l->name() ) )
      {
        rec[ "id" ] = l->layerId().toStdString();
        nodeIdentifier = l->layerId();
        rec[ "queryable" ] = wmsLayersQueryable.contains( l->layerId() );
        rec[ "searchable" ] = wmsLayersSearchable.contains( l->layerId() );
        rec[ "wfs_enabled" ] = wfsLayerIds.contains( l->layerId() );
        const QString layerName { l->layer()->shortName().isEmpty() ? l->layer()->name() : l->layer()->shortName()};
        rec[ "typename" ] = useIds ? l->layer()->id().toStdString() : layerName.toStdString();
        // Override title
        if ( ! l->layer()->title().isEmpty() )
        {
          title = l->layer()->title();
        }
      }
      else
      {
        // Unsupported layer
        return nullptr;
      }
      rec["has_scale_based_visibility"] = l->layer()->hasScaleBasedVisibility();
      if ( l->layer()->hasScaleBasedVisibility() )
      {
        rec["min_scale"] = l->layer()->minimumScale();
        rec["max_scale"] = l->layer()->maximumScale();
      }
      rec[ "is_layer" ] = true;
      rec[ "layer_type" ] = l->layer()->type() == QgsMapLayerType::VectorLayer ? "vector" : "raster";
    }
    else
    {
      rec[ "is_layer" ] = false;
    }
    rec[ "title"] = title.toStdString();
    const QString treeId = parentId.isEmpty() ? QStringLiteral( "root" ) : parentId + "." + nodeIdentifier;
    rec[ "tree_id" ] = treeId.toStdString();
    rec[ "tree_id_hash" ] = QCryptographicHash::hash( treeId.toUtf8(), QCryptographicHash::Md5 ).toHex().toStdString();

    // Collect children
    json children = json::array();
    const auto cChildren { node->children() };
    for ( const auto &c : cChildren )
    {
      const json harvested = harvest( c, treeId );
      if ( ! harvested.is_null() )
      {
        children.push_back( harvested );
      }
    }
    rec [ "children" ] = children;

    return rec;
  };

  return harvest( project.layerTreeRoot(), QString() );
}

QString QgsLandingPageUtils::projectUriFromUrl( const QString &url, const QgsServerSettings &settings )
{
  const auto match { QgsLandingPageUtils::PROJECT_HASH_RE.match( url ) };
  if ( match.hasMatch() )
  {
    const auto availableProjects { QgsLandingPageUtils::projects( settings ) };
    return availableProjects.value( match.captured( QStringLiteral( "projectHash" ) ), QString() );
  }
  return QString();
};


