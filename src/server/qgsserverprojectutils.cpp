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
#include "qgsproject.h"

bool QgsServerProjectUtils::owsServiceCapabilities( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), false );
}

QString QgsServerProjectUtils::owsServiceTitle( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAbstract( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ) );
}

QStringList QgsServerProjectUtils::owsServiceKeywords( const QgsProject &project )
{
  QStringList keywordList;
  QStringList list = project.readListEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ), QStringList() );
  if ( !list.isEmpty() )
  {
    for ( int i = 0; i < list.size(); ++i )
    {
      QString keyword = list.at( i );
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
  return project.readEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactOrganization( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPosition( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPerson( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactMail( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPhone( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceFees( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAccessConstraints( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ) );
}

int QgsServerProjectUtils::wmsMaxWidth( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ), -1 );
}

int QgsServerProjectUtils::wmsMaxHeight( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ), -1 );
}

bool QgsServerProjectUtils::wmsUseLayerIds( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSUseLayerIDs" ), QStringLiteral( "/" ), false );
}

int QgsServerProjectUtils::wmsImageQuality( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSImageQuality" ), QStringLiteral( "/" ), -1 );
}

bool QgsServerProjectUtils::wmsInfoFormatSia2045( const QgsProject &project )
{
  QString sia2045 = project.readEntry( QStringLiteral( "WMSInfoFormatSIA2045" ), QStringLiteral( "/" ), "" );

  return sia2045.compare( QLatin1String( "enabled" ), Qt::CaseInsensitive ) == 0
         || sia2045.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsFeatureInfoAddWktGeometry( const QgsProject &project )
{
  QString wktGeom = project.readEntry( QStringLiteral( "WMSAddWktGeometry" ), QStringLiteral( "/" ), "" );

  return wktGeom.compare( QLatin1String( "enabled" ), Qt::CaseInsensitive ) == 0
         || wktGeom.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
}

bool QgsServerProjectUtils::wmsFeatureInfoSegmentizeWktGeometry( const QgsProject &project )
{
  QString segmGeom = project.readEntry( QStringLiteral( "WMSSegmentizeFeatureInfoGeometry" ), QStringLiteral( "/" ), "" );

  return segmGeom.compare( QLatin1String( "enabled" ), Qt::CaseInsensitive ) == 0
         || segmGeom.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
}

int QgsServerProjectUtils::wmsFeatureInfoPrecision( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSPrecision" ), QStringLiteral( "/" ), 6 );
}

QString QgsServerProjectUtils::wmsFeatureInfoDocumentElement( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSFeatureInfoDocumentElement" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wmsFeatureInfoDocumentElementNs( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSFeatureInfoDocumentElementNS" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wmsFeatureInfoSchema( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSFeatureInfoSchema" ), QStringLiteral( "/" ), "" );
}

QHash<QString, QString> QgsServerProjectUtils::wmsFeatureInfoLayerAliasMap( const QgsProject &project )
{
  QHash<QString, QString> aliasMap;

  //WMSFeatureInfoAliasLayers
  QStringList aliasLayerStringList = project.readListEntry( QStringLiteral( "WMSFeatureInfoAliasLayers" ), QStringLiteral( "/value" ), QStringList() );
  if ( aliasLayerStringList.isEmpty() )
  {
    return aliasMap;
  }

  //WMSFeatureInfoLayerAliases
  QStringList layerAliasStringList = project.readListEntry( QStringLiteral( "WMSFeatureInfoLayerAliases" ), QStringLiteral( "/value" ), QStringList() );
  if ( layerAliasStringList.isEmpty() )
  {
    return aliasMap;
  }

  int nMapEntries = std::min( aliasLayerStringList.size(), layerAliasStringList.size() );
  for ( int i = 0; i < nMapEntries; ++i )
  {
    aliasMap.insert( aliasLayerStringList.at( i ), layerAliasStringList.at( i ) );
  }

  return aliasMap;
}

bool QgsServerProjectUtils::wmsInspireActivate( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/activated" ) );
}

QString QgsServerProjectUtils::wmsInspireLanguage( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/language" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrl" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrlType( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrlType" ) );
}

QString QgsServerProjectUtils::wmsInspireTemporalReference( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/temporalReference" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataDate( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataDate" ) );
}

QStringList QgsServerProjectUtils::wmsRestrictedComposers( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ), QStringList() );
}

QStringList QgsServerProjectUtils::wmsOutputCrsList( const QgsProject &project )
{
  QStringList crsList;
  QStringList wmsCrsList = project.readListEntry( QStringLiteral( "WMSCrsList" ), QStringLiteral( "/" ), QStringList() );
  if ( !wmsCrsList.isEmpty() )
  {
    for ( int i = 0; i < wmsCrsList.size(); ++i )
    {
      QString crs = wmsCrsList.at( i );
      if ( !crs.isEmpty() )
      {
        crsList.append( crs );
      }
    }
  }
  if ( crsList.isEmpty() )
  {
    QStringList valueList = project.readListEntry( QStringLiteral( "WMSEpsgList" ), QStringLiteral( "/" ), QStringList() );
    bool conversionOk;
    for ( int i = 0; i < valueList.size(); ++i )
    {
      int epsgNr = valueList.at( i ).toInt( &conversionOk );
      if ( conversionOk )
      {
        crsList.append( QStringLiteral( "EPSG:%1" ).arg( epsgNr ) );
      }
    }
  }
  if ( crsList.isEmpty() )
  {
    //no CRS restriction defined in the project. Provide project CRS, wgs84 and pseudo mercator
    QString projectCrsId = project.crs().authid();
    crsList.append( projectCrsId );
    if ( projectCrsId.compare( QLatin1String( "EPSG:4326" ), Qt::CaseInsensitive ) != 0 )
    {
      crsList.append( QStringLiteral( "EPSG:%1" ).arg( 4326 ) );
    }
    if ( projectCrsId.compare( QLatin1String( "EPSG:3857" ), Qt::CaseInsensitive ) != 0 )
    {
      crsList.append( QStringLiteral( "EPSG:%1" ).arg( 3857 ) );
    }
  }
  return crsList;
}

QString QgsServerProjectUtils::wmsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wmsRootName( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSRootName" ), QStringLiteral( "/" ), "" );
}

QStringList QgsServerProjectUtils::wmsRestrictedLayers( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WMSRestrictedLayers" ), QStringLiteral( "/" ), QStringList() );
}

QgsRectangle QgsServerProjectUtils::wmsExtent( const QgsProject &project )
{
  bool ok = false;
  QStringList values = project.readListEntry( QStringLiteral( "WMSExtent" ), QStringLiteral( "/" ), QStringList(), &ok );
  if ( !ok || values.size() != 4 )
  {
    return QgsRectangle();
  }
  //order of value elements must be xmin, ymin, xmax, ymax
  double xmin = values[ 0 ].toDouble();
  double ymin = values[ 1 ].toDouble();
  double xmax = values[ 2 ].toDouble();
  double ymax = values[ 3 ].toDouble();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QString QgsServerProjectUtils::wfsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), "" );
}

QStringList QgsServerProjectUtils::wfsLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSLayers" ), QStringLiteral( "/" ) );
}

int QgsServerProjectUtils::wfsLayerPrecision( const QgsProject &project, const QString &layerId )
{
  return project.readNumEntry( QStringLiteral( "WFSLayersPrecision" ), "/" + layerId, 6 );
}

QStringList QgsServerProjectUtils::wfstUpdateLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Update" ) );
}

QStringList QgsServerProjectUtils::wfstInsertLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Insert" ) );
}

QStringList QgsServerProjectUtils::wfstDeleteLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Delete" ) );
}

QString QgsServerProjectUtils::wcsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), "" );
}

QStringList QgsServerProjectUtils::wcsLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::wmtsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMTSSUrl" ), QStringLiteral( "/" ), "" );
}
