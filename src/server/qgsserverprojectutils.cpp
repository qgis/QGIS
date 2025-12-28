/***************************************************************************
                              qgsserverprojectutils.cpp
                              -------------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverprojectutils.h"

#include "qgsmessagelog.h"
#include "qgsproject.h"

double QgsServerProjectUtils::ceilWithPrecision( double number, int places )
{
  const double scaleFactor = std::pow( 10.0, places );
  return ( std::ceil( number * scaleFactor ) / scaleFactor );
}

double QgsServerProjectUtils::floorWithPrecision( double number, int places )
{
  const double scaleFactor = std::pow( 10.0, places );
  return ( std::floor( number * scaleFactor ) / scaleFactor );
}

bool QgsServerProjectUtils::owsServiceCapabilities( const QgsProject &project )
{
  return project.readBoolEntry( u"WMSServiceCapabilities"_s, u"/"_s, false );
}

QString QgsServerProjectUtils::owsServiceTitle( const QgsProject &project )
{
  QString title = project.readEntry( u"WMSServiceTitle"_s, u"/"_s );
  if ( title.isEmpty() )
  {
    title = project.title();
  }
  if ( title.isEmpty() )
  {
    title = QObject::tr( "Untitled" );
  }
  return title;
}

QString QgsServerProjectUtils::owsServiceAbstract( const QgsProject &project )
{
  return project.readEntry( u"WMSServiceAbstract"_s, u"/"_s );
}

QStringList QgsServerProjectUtils::owsServiceKeywords( const QgsProject &project )
{
  QStringList keywordList;
  const QStringList list = project.readListEntry( u"WMSKeywordList"_s, u"/"_s, QStringList() );
  if ( !list.isEmpty() )
  {
    for ( int i = 0; i < list.size(); ++i )
    {
      const QString keyword = list.at( i );
      if ( !keyword.isEmpty() )
      {
        keywordList.append( keyword );
      }
    }
  }
  return keywordList;
}

QString QgsServerProjectUtils::owsServiceOnlineResource( const QgsProject &project )
{
  QString wmsOnlineResource = project.readEntry( u"WMSOnlineResource"_s, u"/"_s );

  const QgsProperty wmsOnlineResourceProperty = project.dataDefinedServerProperties().property( QgsProject::DataDefinedServerProperty::WMSOnlineResource );
  if ( wmsOnlineResourceProperty.isActive() && !wmsOnlineResourceProperty.expressionString().isEmpty() )
  {
    const QgsExpressionContext context = project.createExpressionContext();
    return wmsOnlineResourceProperty.valueAsString( context, wmsOnlineResource );
  }

  return wmsOnlineResource;
}

QString QgsServerProjectUtils::owsServiceContactOrganization( const QgsProject &project )
{
  return project.readEntry( u"WMSContactOrganization"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceContactPosition( const QgsProject &project )
{
  return project.readEntry( u"WMSContactPosition"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceContactPerson( const QgsProject &project )
{
  return project.readEntry( u"WMSContactPerson"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceContactMail( const QgsProject &project )
{
  return project.readEntry( u"WMSContactMail"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceContactPhone( const QgsProject &project )
{
  return project.readEntry( u"WMSContactPhone"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceFees( const QgsProject &project )
{
  return project.readEntry( u"WMSFees"_s, u"/"_s );
}

QString QgsServerProjectUtils::owsServiceAccessConstraints( const QgsProject &project )
{
  return project.readEntry( u"WMSAccessConstraints"_s, u"/"_s );
}

int QgsServerProjectUtils::wmsMaxWidth( const QgsProject &project )
{
  return project.readNumEntry( u"WMSMaxWidth"_s, u"/"_s, -1 );
}

int QgsServerProjectUtils::wmsMaxHeight( const QgsProject &project )
{
  return project.readNumEntry( u"WMSMaxHeight"_s, u"/"_s, -1 );
}

bool QgsServerProjectUtils::wmsUseLayerIds( const QgsProject &project )
{
  return project.readBoolEntry( u"WMSUseLayerIDs"_s, u"/"_s, false );
}

int QgsServerProjectUtils::wmsImageQuality( const QgsProject &project )
{
  return project.readNumEntry( u"WMSImageQuality"_s, u"/"_s, -1 );
}

int QgsServerProjectUtils::wmsTileBuffer( const QgsProject &project )
{
  return project.readNumEntry( u"WMSTileBuffer"_s, u"/"_s, 0 );
}

int QgsServerProjectUtils::wmsMaxAtlasFeatures( const QgsProject &project )
{
  return project.readNumEntry( u"WMSMaxAtlasFeatures"_s, u"/"_s, 1 );
}

double QgsServerProjectUtils::wmsDefaultMapUnitsPerMm( const QgsProject &project )
{
  return project.readDoubleEntry( u"WMSDefaultMapUnitsPerMm"_s, u"/"_s, 1 );
}

bool QgsServerProjectUtils::wmsInfoFormatSia2045( const QgsProject &project )
{
  const QString sia2045 = project.readEntry( u"WMSInfoFormatSIA2045"_s, u"/"_s, "" );

  return sia2045.compare( "enabled"_L1, Qt::CaseInsensitive ) == 0
         || sia2045.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( const QgsProject &project )
{
  const QString wktGeom = project.readEntry( u"WMSAddWktGeometry"_s, u"/"_s, "" );

  return wktGeom.compare( "enabled"_L1, Qt::CaseInsensitive ) == 0
         || wktGeom.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsFeatureInfoUseAttributeFormSettings( const QgsProject &project )
{
  const QString useFormSettings = project.readEntry( u"WMSFeatureInfoUseAttributeFormSettings"_s, u"/"_s, "" );
  return useFormSettings.compare( "enabled"_L1, Qt::CaseInsensitive ) == 0
         || useFormSettings.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsHTMLFeatureInfoUseOnlyMaptip( const QgsProject &project )
{
  const QString useFormSettings = project.readEntry( u"WMSHTMLFeatureInfoUseOnlyMaptip"_s, u"/"_s, "" );
  return useFormSettings.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsFeatureInfoSegmentizeWktGeometry( const QgsProject &project )
{
  const QString segmGeom = project.readEntry( u"WMSSegmentizeFeatureInfoGeometry"_s, u"/"_s, "" );

  return segmGeom.compare( "enabled"_L1, Qt::CaseInsensitive ) == 0
         || segmGeom.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsAddLegendGroupsLegendGraphic( const QgsProject &project )
{
  const QString legendGroups = project.readEntry( u"WMSAddLayerGroupsLegendGraphic"_s, u"/"_s, "" );
  return legendGroups.compare( "enabled"_L1, Qt::CaseInsensitive ) == 0
         || legendGroups.compare( "true"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsSkipNameForGroup( const QgsProject &project )
{
  return project.readBoolEntry( u"WMSSkipNameForGroup"_s, u"/"_s, false );
}

int QgsServerProjectUtils::wmsFeatureInfoPrecision( const QgsProject &project )
{
  return project.readNumEntry( u"WMSPrecision"_s, u"/"_s, 6 );
}

QString QgsServerProjectUtils::wmsFeatureInfoDocumentElement( const QgsProject &project )
{
  return project.readEntry( u"WMSFeatureInfoDocumentElement"_s, u"/"_s, "" );
}

QString QgsServerProjectUtils::wmsFeatureInfoDocumentElementNs( const QgsProject &project )
{
  return project.readEntry( u"WMSFeatureInfoDocumentElementNS"_s, u"/"_s, "" );
}

QString QgsServerProjectUtils::wmsFeatureInfoSchema( const QgsProject &project )
{
  return project.readEntry( u"WMSFeatureInfoSchema"_s, u"/"_s, "" );
}

QHash<QString, QString> QgsServerProjectUtils::wmsFeatureInfoLayerAliasMap( const QgsProject &project )
{
  QHash<QString, QString> aliasMap;

  //WMSFeatureInfoAliasLayers
  const QStringList aliasLayerStringList = project.readListEntry( u"WMSFeatureInfoAliasLayers"_s, u"/value"_s, QStringList() );
  if ( aliasLayerStringList.isEmpty() )
  {
    return aliasMap;
  }

  //WMSFeatureInfoLayerAliases
  const QStringList layerAliasStringList = project.readListEntry( u"WMSFeatureInfoLayerAliases"_s, u"/value"_s, QStringList() );
  if ( layerAliasStringList.isEmpty() )
  {
    return aliasMap;
  }

  const int nMapEntries = std::min( aliasLayerStringList.size(), layerAliasStringList.size() );
  for ( int i = 0; i < nMapEntries; ++i )
  {
    aliasMap.insert( aliasLayerStringList.at( i ), layerAliasStringList.at( i ) );
  }

  return aliasMap;
}

bool QgsServerProjectUtils::wmsInspireActivate( const QgsProject &project )
{
  return project.readBoolEntry( u"WMSInspire"_s, u"/activated"_s );
}

QString QgsServerProjectUtils::wmsInspireLanguage( const QgsProject &project )
{
  return project.readEntry( u"WMSInspire"_s, u"/language"_s );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrl( const QgsProject &project )
{
  return project.readEntry( u"WMSInspire"_s, u"/metadataUrl"_s );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrlType( const QgsProject &project )
{
  return project.readEntry( u"WMSInspire"_s, u"/metadataUrlType"_s );
}

QString QgsServerProjectUtils::wmsInspireTemporalReference( const QgsProject &project )
{
  return project.readEntry( u"WMSInspire"_s, u"/temporalReference"_s );
}

QString QgsServerProjectUtils::wmsInspireMetadataDate( const QgsProject &project )
{
  return project.readEntry( u"WMSInspire"_s, u"/metadataDate"_s );
}

QStringList QgsServerProjectUtils::wmsRestrictedComposers( const QgsProject &project )
{
  return project.readListEntry( u"WMSRestrictedComposers"_s, u"/"_s, QStringList() );
}

QStringList QgsServerProjectUtils::wmsOutputCrsList( const QgsProject &project )
{
  QStringList crsList;
  const QStringList wmsCrsList = project.readListEntry( u"WMSCrsList"_s, u"/"_s, QStringList() );
  if ( !wmsCrsList.isEmpty() )
  {
    for ( const auto &crs : std::as_const( wmsCrsList ) )
    {
      if ( !crs.isEmpty() )
      {
        crsList.append( crs );
      }
    }
  }
  if ( crsList.isEmpty() )
  {
    const QStringList valueList = project.readListEntry( u"WMSEpsgList"_s, u"/"_s, QStringList() );
    bool conversionOk;
    for ( const auto &espgStr : valueList )
    {
      const int epsgNr = espgStr.toInt( &conversionOk );
      if ( conversionOk )
      {
        crsList.append( u"EPSG:%1"_s.arg( epsgNr ) );
      }
    }
  }
  if ( crsList.isEmpty() )
  {
    //no CRS restriction defined in the project. Provide project CRS, wgs84 and pseudo mercator
    const QString projectCrsId = project.crs().authid();
    crsList.append( projectCrsId );
    if ( projectCrsId.compare( "EPSG:4326"_L1, Qt::CaseInsensitive ) != 0 )
    {
      crsList.append( u"EPSG:%1"_s.arg( 4326 ) );
    }
    if ( projectCrsId.compare( "EPSG:3857"_L1, Qt::CaseInsensitive ) != 0 )
    {
      crsList.append( u"EPSG:%1"_s.arg( 3857 ) );
    }
  }
  return crsList;
}

QStringList QgsServerProjectUtils::wmsOutputCrsListAsOgcUrn( const QgsProject &project )
{
  const QStringList crsList = wmsOutputCrsList( project );
  QStringList crsListAsOgcUrn;
  for ( const QString &crsString : crsList )
  {
    crsListAsOgcUrn.append( QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsString ).toOgcUrn() );
  }

  return crsListAsOgcUrn;
}

QString QgsServerProjectUtils::serviceUrl( const QString &service, const QgsServerRequest &request, const QgsServerSettings &settings )
{
  const QString serviceUpper = service.toUpper();
  QString url = settings.serviceUrl( serviceUpper );
  if ( !url.isEmpty() )
  {
    return url;
  }

  QgsServerRequest::RequestHeader header = QgsServerRequest::RequestHeader::X_QGIS_SERVICE_URL;
  if ( serviceUpper == "WMS"_L1 )
  {
    header = QgsServerRequest::RequestHeader::X_QGIS_WMS_SERVICE_URL;
  }
  else if ( serviceUpper == "WFS"_L1 )
  {
    header = QgsServerRequest::RequestHeader::X_QGIS_WFS_SERVICE_URL;
  }
  else if ( serviceUpper == "WCS"_L1 )
  {
    header = QgsServerRequest::RequestHeader::X_QGIS_WCS_SERVICE_URL;
  }
  else if ( serviceUpper == "WMTS"_L1 )
  {
    header = QgsServerRequest::RequestHeader::X_QGIS_WMTS_SERVICE_URL;
  }
  url = request.header( header );
  if ( !url.isEmpty() )
  {
    return url;
  }
  url = request.header( QgsServerRequest::RequestHeader::X_QGIS_SERVICE_URL );
  if ( !url.isEmpty() )
  {
    return url;
  }

  QString proto;
  QString host;

  QString forwarded = request.header( QgsServerRequest::FORWARDED );
  if ( !forwarded.isEmpty() )
  {
    forwarded = forwarded.split( QLatin1Char( ',' ) )[0];
    const QStringList elements = forwarded.split( ';' );
    for ( const QString &element : elements )
    {
      QStringList splited_element = element.trimmed().split( QLatin1Char( '=' ) );
      if ( splited_element[0] == "host" )
      {
        host = splited_element[1];
      }
      if ( splited_element[0] == "proto" )
      {
        proto = splited_element[1];
      }
    }
  }

  if ( host.isEmpty() )
  {
    host = request.header( QgsServerRequest::RequestHeader::X_FORWARDED_HOST );
    proto = request.header( QgsServerRequest::RequestHeader::X_FORWARDED_PROTO );
  }

  if ( host.isEmpty() )
  {
    host = request.header( QgsServerRequest::RequestHeader::HOST );
  }

  QUrl urlQUrl = request.baseUrl();
  if ( !proto.isEmpty() )
  {
    urlQUrl.setScheme( proto );
  }

  if ( !host.isEmpty() )
  {
    QStringList hostPort = host.split( QLatin1Char( ':' ) );
    if ( hostPort.length() == 1 )
    {
      urlQUrl.setHost( hostPort[0] );
      urlQUrl.setPort( -1 );
    }
    if ( hostPort.length() == 2 )
    {
      urlQUrl.setHost( hostPort[0] );
      urlQUrl.setPort( hostPort[1].toInt() );
    }
  }

  // https://docs.qgis.org/3.16/en/docs/server_manual/services.html#wms-map
  const QUrlQuery query { request.originalUrl().query() };
  const QList<QPair<QString, QString>> constItems { query.queryItems() };
  QString map;
  for ( const QPair<QString, QString> &item : std::as_const( constItems ) )
  {
    if ( 0 == item.first.compare( "MAP"_L1, Qt::CaseSensitivity::CaseInsensitive ) )
    {
      map = item.second;
      break;
    }
  }

  if ( !map.isEmpty() )
  {
    QUrlQuery query;
    query.setQueryItems( { { "MAP", map } } );
    urlQUrl.setQuery( query );
  }
  else
  {
    urlQUrl.setQuery( nullptr );
  }

  return urlQUrl.url();
}

QString QgsServerProjectUtils::wmsServiceUrl( const QgsProject &project, const QgsServerRequest &request, const QgsServerSettings &settings )
{
  QString url = project.readEntry( u"WMSUrl"_s, u"/"_s, "" );
  if ( url.isEmpty() )
  {
    url = serviceUrl( u"WMS"_s, request, settings );
  }
  return url;
}

QString QgsServerProjectUtils::wmsRootName( const QgsProject &project )
{
  return project.readEntry( u"WMSRootName"_s, u"/"_s, "" );
}

QStringList QgsServerProjectUtils::wmsRestrictedLayers( const QgsProject &project )
{
  return project.readListEntry( u"WMSRestrictedLayers"_s, u"/"_s, QStringList() );
}

QgsRectangle QgsServerProjectUtils::wmsExtent( const QgsProject &project )
{
  bool ok = false;
  QStringList values = project.readListEntry( u"WMSExtent"_s, u"/"_s, QStringList(), &ok );
  if ( !ok || values.size() != 4 )
  {
    return QgsRectangle();
  }
  //order of value elements must be xmin, ymin, xmax, ymax
  const double xmin = values[0].toDouble();
  const double ymin = values[1].toDouble();
  const double xmax = values[2].toDouble();
  const double ymax = values[3].toDouble();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QString QgsServerProjectUtils::wfsServiceUrl( const QgsProject &project, const QgsServerRequest &request, const QgsServerSettings &settings )
{
  QString url = project.readEntry( u"WFSUrl"_s, u"/"_s, "" );
  if ( url.isEmpty() )
  {
    url = serviceUrl( u"WFS"_s, request, settings );
  }
  return url;
}

QStringList QgsServerProjectUtils::wfsLayerIds( const QgsProject &project )
{
  return project.readListEntry( u"WFSLayers"_s, u"/"_s );
}

int QgsServerProjectUtils::wfsLayerPrecision( const QgsProject &project, const QString &layerId )
{
  return project.readNumEntry( u"WFSLayersPrecision"_s, "/" + layerId, 6 );
}

QStringList QgsServerProjectUtils::wfstUpdateLayerIds( const QgsProject &project )
{
  return project.readListEntry( u"WFSTLayers"_s, u"Update"_s );
}

QStringList QgsServerProjectUtils::wfstInsertLayerIds( const QgsProject &project )
{
  return project.readListEntry( u"WFSTLayers"_s, u"Insert"_s );
}

QStringList QgsServerProjectUtils::wfstDeleteLayerIds( const QgsProject &project )
{
  return project.readListEntry( u"WFSTLayers"_s, u"Delete"_s );
}

QString QgsServerProjectUtils::wcsServiceUrl( const QgsProject &project, const QgsServerRequest &request, const QgsServerSettings &settings )
{
  QString url = project.readEntry( u"WCSUrl"_s, u"/"_s, "" );
  if ( url.isEmpty() )
  {
    url = serviceUrl( u"WCS"_s, request, settings );
  }
  return url;
}

QStringList QgsServerProjectUtils::wcsLayerIds( const QgsProject &project )
{
  return project.readListEntry( u"WCSLayers"_s, u"/"_s );
}

QString QgsServerProjectUtils::wmtsServiceUrl( const QgsProject &project, const QgsServerRequest &request, const QgsServerSettings &settings )
{
  QString url = project.readEntry( u"WMTSUrl"_s, u"/"_s, "" );
  if ( url.isEmpty() )
  {
    url = serviceUrl( u"WMTS"_s, request, settings );
  }
  return url;
}

bool QgsServerProjectUtils::wmsRenderMapTiles( const QgsProject &project )
{
  return project.readBoolEntry( u"RenderMapTile"_s, u"/"_s, false );
}
