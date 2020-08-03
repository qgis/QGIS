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

#include "qgslandingpageutils.h"
#include "qgsserverprojectutils.h"
#include "qgsmessagelog.h"
#include "qgslayertree.h"
#include "qgsvectorlayer.h"
#include "nlohmann/json.hpp"

#include <QCryptographicHash>

const QRegularExpression QgsLandingPageUtils::PROJECT_HASH_RE { QStringLiteral( "/(?<projectHash>[a-f0-9]{32})" ) };

QMap<QString, QString> QgsLandingPageUtils::projects( )
{
  // TODO: cache this information and use a dir-watcher to invalidate
  QMap<QString, QString> availableProjects;
  for ( const auto &path : QString( qgetenv( "QGIS_SERVER_PROJECTS_DIRECTORIES" ) ).split( QStringLiteral( "||" ) ) )
  {
    const QDir dir { path };
    if ( dir.exists() )
    {
      const auto constFiles { dir.entryList( ) };
      for ( const auto &f : constFiles )
      {
        if ( f.endsWith( QStringLiteral( ".qgs" ), Qt::CaseSensitivity::CaseInsensitive ) ||
             f.endsWith( QStringLiteral( ".qgz" ), Qt::CaseSensitivity::CaseInsensitive ) )
        {
          const QString fullPath { path + '/' + f };
          availableProjects[ QCryptographicHash::hash( fullPath.toUtf8(), QCryptographicHash::Md5 ).toHex() ] = fullPath;
        }
      }
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "QGIS_SERVER_PROJECTS_DIRECTORIES entry '%1' was not found: skipping." ).arg( path ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
    }
  }

  // TODO: PG projects

  return availableProjects;
}

json QgsLandingPageUtils::projectInfo( const QString &projectPath )
{
  // Helper for QStringList
  auto jList = [ ]( const QStringList & l ) -> json
  {
    json a = json::array( );
    for ( const auto &e : qgis::as_const( l ) )
    {
      a.push_back( e.toStdString() );
    }
    return a;
  };

  auto jHash = [ ]( const QHash<QString, QString> &l ) -> json
  {
    json a;
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
      const auto &addrs { c.addresses };
      for ( const auto &a : addrs )
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
      jContacts.push_back( jContacts );
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
        { "format", l.format.toStdString()}
      } );
    }
    return jLinks;
  };

  json info;
  QgsProject p;
  if ( p.read( projectPath ) )
  {
    // Title
    QString title { p.metadata().title() };
    if ( title.isEmpty() )
      title = QgsServerProjectUtils::owsServiceTitle( p );
    if ( title.isEmpty() )
      title = p.title();
    if ( title.isEmpty() )
      title = p.baseName();
    info["title"] = title.toStdString();
    // Description
    QString description { p.metadata().abstract() };
    if ( description.isEmpty() )
      description = QgsServerProjectUtils::owsServiceAbstract( p );
    info["description"] = description.toStdString();
    // CRS
    const QStringList wmsOutputCrsList { QgsServerProjectUtils::wmsOutputCrsList( p ) };
    const QString crs { wmsOutputCrsList.contains( QStringLiteral( "EPSG:4326" ) ) || wmsOutputCrsList.isEmpty() ?
                        QStringLiteral( "EPSG:4326" ) : wmsOutputCrsList.first() };
    info["crs"] = crs.toStdString();
    // Typenames for WMS
    const bool useIds { QgsServerProjectUtils::wmsUseLayerIds( p ) };
    QStringList typenames;
    const QStringList restrictedWms { QgsServerProjectUtils::wmsRestrictedLayers( p ) };
    const auto constLayers { p.mapLayers().values( ) };
    for ( const auto &l : constLayers )
    {
      if ( ! restrictedWms.contains( l->name() ) )
      {
        typenames.push_back( useIds ? l->id() : l->name() );
      }
    }
    // Extent
    QgsRectangle extent { QgsServerProjectUtils::wmsExtent( p ) };
    QgsCoordinateReferenceSystem targetCrs;
    if ( crs.split( ':' ).count() == 2 )
      targetCrs = QgsCoordinateReferenceSystem::fromEpsgId( crs.split( ':' ).last().toLong() );
    if ( extent.isNull() && crs.split( ':' ).count() == 2 )
    {
      for ( const auto &l : constLayers )
      {
        if ( ! restrictedWms.contains( l->name() ) )
        {
          QgsRectangle layerExtent { l->extent() };
          if ( l->crs() != targetCrs && targetCrs.isValid() )
          {
            QgsCoordinateTransform ct { l->crs(), targetCrs, p.transformContext() };
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
      if ( targetCrs.isValid() && targetCrs != p.crs() )
      {
        QgsCoordinateTransform ct { p.crs(), targetCrs, p.transformContext() };
        extent = ct.transform( extent );
      }
    }
    info["extent"] = json::array( { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } );
    QgsRectangle geographicExtent { extent };
    if ( targetCrs.authid() != 4326 )
    {
      QgsCoordinateTransform ct { targetCrs,  QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), p.transformContext() };
      geographicExtent = ct.transform( geographicExtent );
    }
    info["geographic_extent"] = json::array( { geographicExtent.xMinimum(), geographicExtent.yMinimum(), geographicExtent.xMaximum(), geographicExtent.yMaximum() } );
    // Metadata
    json metadata;
    const QgsProjectMetadata &md { p.metadata() };
    metadata["tile"] = md.title().toStdString();
    metadata["identifier"] = md.identifier().toStdString();
    metadata["parentIdentifier"] = md.parentIdentifier().toStdString();
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
    capabilities["owsServiceCapabilities"] = QgsServerProjectUtils::owsServiceCapabilities( p );
    capabilities["owsServiceAbstract"] = QgsServerProjectUtils::owsServiceAbstract( p ).toStdString();
    capabilities["owsServiceAccessConstraints"] = QgsServerProjectUtils::owsServiceAccessConstraints( p ).toStdString();
    capabilities["owsServiceContactMail"] = QgsServerProjectUtils::owsServiceContactMail( p ).toStdString();
    capabilities["owsServiceContactOrganization"] = QgsServerProjectUtils::owsServiceContactOrganization( p ).toStdString();
    capabilities["owsServiceContactPerson"] = QgsServerProjectUtils::owsServiceContactPerson( p ).toStdString();
    capabilities["owsServiceContactPhone"] = QgsServerProjectUtils::owsServiceContactPhone( p ).toStdString();
    capabilities["owsServiceContactPosition"] = QgsServerProjectUtils::owsServiceContactPosition( p ).toStdString();
    capabilities["owsServiceFees"] = QgsServerProjectUtils::owsServiceFees( p ).toStdString();
    capabilities["owsServiceKeywords"] = jList( QgsServerProjectUtils::owsServiceKeywords( p ) );
    capabilities["owsServiceOnlineResource"] = QgsServerProjectUtils::owsServiceOnlineResource( p ).toStdString();
    capabilities["owsServiceTitle"] = QgsServerProjectUtils::owsServiceTitle( p ).toStdString();
    capabilities["wcsLayerIds"] = jList( QgsServerProjectUtils::wcsLayerIds( p ) );
    capabilities["wcsServiceUrl"] = QgsServerProjectUtils::wcsServiceUrl( p ).toStdString();
    capabilities["wfsLayerIds"] = jList( QgsServerProjectUtils::wfsLayerIds( p ) );
    capabilities["wfsServiceUrl"] = QgsServerProjectUtils::wfsServiceUrl( p ).toStdString();
    capabilities["wfstDeleteLayerIds"] = jList( QgsServerProjectUtils::wfstDeleteLayerIds( p ) );
    capabilities["wfstInsertLayerIds"] = jList( QgsServerProjectUtils::wfstInsertLayerIds( p ) );
    capabilities["wfstUpdateLayerIds"] = jList( QgsServerProjectUtils::wfstUpdateLayerIds( p ) );
    capabilities["wmsDefaultMapUnitsPerMm"] = QgsServerProjectUtils::wmsDefaultMapUnitsPerMm( p );
    // Skip wmsExtent because it's already in "extent"
    capabilities["wmsFeatureInfoAddWktGeometry"] = QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( p );
    capabilities["wmsFeatureInfoDocumentElement"] = QgsServerProjectUtils::wmsFeatureInfoDocumentElement( p ).toStdString();
    capabilities["wmsFeatureInfoDocumentElementNs"] = QgsServerProjectUtils::wmsFeatureInfoDocumentElementNs( p ).toStdString();
    capabilities["wmsFeatureInfoLayerAliasMap"] = jHash( QgsServerProjectUtils::wmsFeatureInfoLayerAliasMap( p ) );
    capabilities["wmsFeatureInfoPrecision"] = QgsServerProjectUtils::wmsFeatureInfoPrecision( p );
    capabilities["wmsFeatureInfoSchema"] = QgsServerProjectUtils::wmsFeatureInfoSchema( p ).toStdString();
    capabilities["wmsFeatureInfoSegmentizeWktGeometry"] = QgsServerProjectUtils::wmsFeatureInfoSegmentizeWktGeometry( p );
    capabilities["wmsImageQuality"] = QgsServerProjectUtils::wmsImageQuality( p );
    capabilities["wmsInfoFormatSia2045"] = QgsServerProjectUtils::wmsInfoFormatSia2045( p );
    capabilities["wmsInspireActivate"] = QgsServerProjectUtils::wmsInspireActivate( p );
    capabilities["wmsInspireLanguage"] = QgsServerProjectUtils::wmsInspireLanguage( p ).toStdString();
    capabilities["wmsInspireMetadataDate"] = QgsServerProjectUtils::wmsInspireMetadataDate( p ).toStdString();
    capabilities["wmsInspireMetadataUrl"] = QgsServerProjectUtils::wmsInspireMetadataUrl( p ).toStdString();
    capabilities["wmsInspireMetadataUrlType"] = QgsServerProjectUtils::wmsInspireMetadataUrlType( p ).toStdString();
    capabilities["wmsInspireTemporalReference"] = QgsServerProjectUtils::wmsInspireTemporalReference( p ).toStdString();
    capabilities["wmsMaxAtlasFeatures"] = QgsServerProjectUtils::wmsMaxAtlasFeatures( p );
    capabilities["wmsMaxHeight"] = QgsServerProjectUtils::wmsMaxHeight( p );
    capabilities["wmsMaxWidth"] = QgsServerProjectUtils::wmsMaxWidth( p );
    capabilities["wmsOutputCrsList"] = jList( QgsServerProjectUtils::wmsOutputCrsList( p ) );
    capabilities["wmsRestrictedComposers"] = jList( QgsServerProjectUtils::wmsRestrictedComposers( p ) );
    capabilities["wmsRestrictedLayers"] = jList( QgsServerProjectUtils::wmsRestrictedLayers( p ) );
    capabilities["wmsRootName"] = QgsServerProjectUtils::wmsRootName( p ).toStdString();
    capabilities["wmsServiceUrl"] = QgsServerProjectUtils::wmsServiceUrl( p ).toStdString();
    capabilities["wmsTileBuffer"] = QgsServerProjectUtils::wmsTileBuffer( p );
    capabilities["wmsUseLayerIds"] = QgsServerProjectUtils::wmsUseLayerIds( p );
    capabilities["wmtsServiceUrl" ] = QgsServerProjectUtils::wmtsServiceUrl( p ).toStdString();
    info["capabilities"] = capabilities;
    // WMS layers
    info[ "wms_root_name" ] = QgsServerProjectUtils::wmsRootName( p ).toStdString();
    if ( QgsServerProjectUtils::wmsRootName( p ).isEmpty() )
    {
      info[ "wms_root_name" ] = title.toStdString();
    }
    json wmsLayers;
    // For convenience:
    // Map layer (short) name to layer id
    json wmsLayersTypenameIdMap;
    // Map layer title to layer (short) name (or id if use_ids)
    json wmsLayersMap;
    QStringList wmsLayersSearchable;
    QStringList wmsLayersQueryable;
    for ( const auto &l : constLayers )
    {
      if ( ! restrictedWms.contains( l->name() ) )
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
          const auto &cFields { vl->fields() };
          for ( const auto &f : cFields )
          {
            if ( vl->excludeAttributesWfs().contains( vl->name() ) )
            {
              ++fieldIdx;
              continue;
            }
            const auto &constraints { f.constraints().constraints() };
            const bool notNull { constraints &QgsFieldConstraints::Constraint::ConstraintNotNull &&
                                 f.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintNotNull ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard };
            const bool unique { constraints &QgsFieldConstraints::Constraint::ConstraintUnique &&
                                f.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintUnique ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard };
            const bool hasExpression { constraints &QgsFieldConstraints::Constraint::ConstraintExpression &&
                                       f.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintExpression ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard };
            const QString &defaultValue { vl->dataProvider()->defaultValueClause( fieldIdx ) };
            fieldsData[ f.name().toStdString() ] =
            {
              { "type", f.typeName().toStdString() },
              { "label", f.alias().isEmpty() ? f.name().toStdString() : f.alias().toStdString() },
              { "precision", f.precision() },
              { "length", f.length() },
              { "unique", unique },
              { "not_null", notNull },
              { "has_expression", hasExpression },
              { "default", defaultValue.toStdString() },
              { "expression", f.constraints().constraintExpression().toStdString() },
              { "editable", !( notNull &&unique && ! defaultValue.isEmpty() ) }
            };

            ++fieldIdx;
          }
          wmsLayer[ "fields" ] = fieldsData;
        }
        wmsLayer[ "extent" ] = {{ l->extent().xMinimum(), l->extent().yMinimum(), l->extent().xMaximum(), l->extent().yMaximum() }};
        wmsLayers[ l->id().toStdString() ] = wmsLayer;

        // Fill maps
        const QString name { l->title().isEmpty() ? l->name() : l->title() };
        const QString shortName { l->shortName().isEmpty() ? l->shortName() : l->name() };
        wmsLayersTypenameIdMap[ shortName.toStdString()] = l->id().toStdString();
        wmsLayersMap[ name.toStdString() ] = useIds ? l->id().toStdString() : shortName.toStdString();

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
        const auto &cKw { md.keywords().keys() };
        json jKeywords;
        for ( const auto &k : cKw )
        {
          jKeywords[ k.toStdString() ] = jList( md.keywords()[ k ] );
        }
        layerMetadata[ "keywords" ] = jKeywords;
        layerMetadata[ "language" ] = md.language( ).toStdString();
        layerMetadata[ "licenses" ] = jList( md.licenses( ) );
        layerMetadata[ "links" ] = jLinksList( md.links( ) );
        layerMetadata[ "parentIdentifier" ] = md.parentIdentifier( ).toStdString();
        layerMetadata[ "rights" ] = jList( md.rights( ) );
        layerMetadata[ "title" ] = md.title( ).toStdString();
        layerMetadata[ "type" ] = md.type( ).toStdString();
      }
    }
    info[ "wms_layers" ] = wmsLayers;
    info[ "wms_layers_map" ] = wmsLayersMap;
    info[ "wms_layers_queryable" ] = jList( wmsLayersQueryable );
    info[ "wms_layers_searchable" ] = jList( wmsLayersSearchable );
    info[ "wms_layers_typename_id_map" ] = wmsLayersTypenameIdMap;
    info[ "toc " ] = layerTree( p, wmsLayersQueryable, wmsLayersSearchable );

  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Could not read project '%1': skipping." ).arg( projectPath ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
  }
  return info;
}

json QgsLandingPageUtils::layerTree( const QgsProject &project, const QStringList &wmsLayersQueryable,  const QStringList &wmsLayersSearchable )
{
  const bool useIds { QgsServerProjectUtils::wmsUseLayerIds( project ) };
  const QStringList wmsRestrictedLayers { QgsServerProjectUtils::wmsRestrictedLayers( project ) };
  const QStringList wfsLayerIds { QgsServerProjectUtils::wfsLayerIds( project ) };

  std::function<json( const QgsLayerTreeNode *, const QString & )> harvest = [ & ]( const QgsLayerTreeNode * node, const QString & parentId ) -> json
  {
    const std::string nodeName { parentId.isEmpty() ? "root" : node->name().toStdString() };
    QString title { QString::fromStdString( nodeName ) };
    json rec {
      { "title", nodeName },
      { "name", nodeName },
      { "expanded", node->isExpanded() },
      { "visible", node->isVisible() },
    };
    if ( QgsLayerTree::isLayer( node ) )
    {
      const QgsLayerTreeLayer *l { static_cast<const QgsLayerTreeLayer *>( node ) };
      if ( l->layer()->type() == QgsMapLayerType::VectorLayer || l->layer()->type() == QgsMapLayerType::RasterLayer )
      {
        rec[ "id" ] = l->layerId().toStdString();
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
      rec[ "is_layer" ] = true;
    }
    else
    {
      rec[ "is_layer" ] = false;
    }
    rec[ "title"] = title.toStdString();
    const QString treeId = parentId == QStringLiteral( "root" ) ? QStringLiteral( "root" ) : parentId + "." + title;
    rec[ "tree_id" ] = treeId.toStdString();
    rec[ "tree_id_hash" ] = QCryptographicHash::hash( treeId.toUtf8(), QCryptographicHash::Md5 ).toHex().toStdString();

    // Collect children
    json children = json::array();
    const auto cChildren { node->children() };
    for ( const auto &c : cChildren )
    {
      const json harvested { harvest( c, treeId ) };
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

QString QgsLandingPageUtils::projectPathFromUrl( const QString &url )
{
  const auto match { QgsLandingPageUtils::PROJECT_HASH_RE.match( url ) };
  if ( match.hasMatch() )
  {
    const auto availableProjects { QgsLandingPageUtils::projects() };
    return availableProjects.value( match.captured( QStringLiteral( "projectHash" ) ), QString() );
  }
  return QString();
};


