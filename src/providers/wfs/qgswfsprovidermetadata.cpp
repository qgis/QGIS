/***************************************************************************
                              qgswfsprovidermetadata.cpp
                              -------------------
  begin                : November 2022
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2016-2022 by Even Rouault
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsfeedback.h"
#include "qgsogcutils.h"
#include "qgsoapifprovider.h"
#include "qgswfsdataitems.h"
#include "qgsprovidersublayerdetails.h"
#include "qgswfsconstants.h"
#include "qgswfsprovider.h"
#include "qgswfsprovidermetadata.h"
#include "qgswfscapabilities.h"
#include "qgswfsgetfeature.h"
#include "qgswfsshareddata.h"
#include "qgssettings.h"

#include <QDomDocument>
#include <QMessageBox>
#include <QDomNodeList>
#include <QFile>
#include <QIcon>
#include <QUrl>
#include <QTimer>

QgsDataProvider *QgsWfsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsWFSProvider( uri, options );
}

QList<QgsMapLayerType> QgsWfsProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::VectorLayer };
}

QList<QgsDataItemProvider *> QgsWfsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsWfsDataItemProvider;
  return providers;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsWfsProviderMetadata::capabilities() const
{
  return QuerySublayers;
}

QString QgsWFSProvider::buildFilterByGeometryType( const QgsWfsCapabilities::Capabilities &caps,
    const QString &geometryElement,
    const QString &function )
{
  /* Generate something like:
   <fes:Filter xmlns="http://www.opengis.net/fes/2.0">
    <fes:PropertyIsEqualTo>
      <fes:Function name="IsPoint">
        <fes:ValueReference>xplan:geltungsbereich</fes:ValueReference>
      </fes:Function>
      <fes:Literal>false</fes:Literal>
    </fes:PropertyIsEqualTo>
  </fes:Filter>
  */

  QDomDocument doc;
  QDomElement filterElem =
    ( caps.version.startsWith( QStringLiteral( "2.0" ) ) ) ?
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/fes/2.0" ), QStringLiteral( "fes:Filter" ) ) :
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/ogc" ), QStringLiteral( "ogc:Filter" ) ) ;
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? QStringLiteral( "fes" ) : QStringLiteral( "ogc" ) );
  QDomElement propertyIsEqualToElem = doc.createElement( filterPrefix + QStringLiteral( ":PropertyIsEqualTo" ) );
  filterElem.appendChild( propertyIsEqualToElem );
  QDomElement functionElem = doc.createElement( filterPrefix + QStringLiteral( ":Function" ) );
  propertyIsEqualToElem.appendChild( functionElem );
  QDomAttr attrFunctionName = doc.createAttribute( QStringLiteral( "name" ) );
  attrFunctionName.setValue( function );
  functionElem.setAttributeNode( attrFunctionName );
  QDomElement valueReferenceElem = doc.createElement(
                                     ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + QStringLiteral( ":ValueReference" ) : filterPrefix + QStringLiteral( ":PropertyName" ) );
  functionElem.appendChild( valueReferenceElem );
  valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
  QDomElement literalElem = doc.createElement( filterPrefix + QStringLiteral( ":Literal" ) );
  propertyIsEqualToElem.appendChild( literalElem );
  literalElem.appendChild( doc.createTextNode( QStringLiteral( "true" ) ) );

  return doc.toString();
}

QString QgsWFSProvider::buildIsNullGeometryFilter( const QgsWfsCapabilities::Capabilities &caps,
    const QString &geometryElement )
{
  /* Generate something like:
   <fes:Filter xmlns="http://www.opengis.net/fes/2.0">
    <fes:PropertyIsNull>
        <fes:ValueReference>xplan:geltungsbereich</fes:ValueReference>
    </fes:PropertyIsNull>
  </fes:Filter>
  */

  QDomDocument doc;
  QDomElement filterElem =
    ( caps.version.startsWith( QStringLiteral( "2.0" ) ) ) ?
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/fes/2.0" ), QStringLiteral( "fes:Filter" ) ) :
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/ogc" ), QStringLiteral( "ogc:Filter" ) ) ;
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? QStringLiteral( "fes" ) : QStringLiteral( "ogc" ) );
  QDomElement propertyIsNullElem = doc.createElement( filterPrefix + QStringLiteral( ":PropertyIsNull" ) );
  filterElem.appendChild( propertyIsNullElem );
  QDomElement valueReferenceElem = doc.createElement(
                                     ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + QStringLiteral( ":ValueReference" ) : filterPrefix + QStringLiteral( ":PropertyName" ) );
  propertyIsNullElem.appendChild( valueReferenceElem );
  valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );

  return doc.toString();
}

QString QgsWFSProvider::buildGeometryCollectionFilter( const QgsWfsCapabilities::Capabilities &caps,
    const QString &geometryElement )
{
  QDomDocument doc;
  QDomElement filterElem =
    ( caps.version.startsWith( QStringLiteral( "2.0" ) ) ) ?
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/fes/2.0" ), QStringLiteral( "fes:Filter" ) ) :
    doc.createElementNS( QStringLiteral( "http://www.opengis.net/ogc" ), QStringLiteral( "ogc:Filter" ) ) ;
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? QStringLiteral( "fes" ) : QStringLiteral( "ogc" ) );

  QDomElement andElem = doc.createElement( filterPrefix + QStringLiteral( ":And" ) );
  filterElem.appendChild( andElem );

  {

    QDomElement notElem = doc.createElement( filterPrefix + QStringLiteral( ":Not" ) );
    andElem.appendChild( notElem );

    QDomElement propertyIsNullElem = doc.createElement( filterPrefix + QStringLiteral( ":PropertyIsNull" ) );
    notElem.appendChild( propertyIsNullElem );
    QDomElement valueReferenceElem = doc.createElement(
                                       ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + QStringLiteral( ":ValueReference" ) : filterPrefix + QStringLiteral( ":PropertyName" ) );
    propertyIsNullElem.appendChild( valueReferenceElem );
    valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );

  }

  for ( const QString &function : { QStringLiteral( "IsPoint" ), QStringLiteral( "IsCurve" ), QStringLiteral( "IsSurface" ) } )
  {
    QDomElement propertyIsEqualToElem = doc.createElement( filterPrefix + QStringLiteral( ":PropertyIsEqualTo" ) );
    andElem.appendChild( propertyIsEqualToElem );
    QDomElement functionElem = doc.createElement( filterPrefix + QStringLiteral( ":Function" ) );
    propertyIsEqualToElem.appendChild( functionElem );
    QDomAttr attrFunctionName = doc.createAttribute( QStringLiteral( "name" ) );
    attrFunctionName.setValue( function );
    functionElem.setAttributeNode( attrFunctionName );
    QDomElement valueReferenceElem = doc.createElement(
                                       ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + QStringLiteral( ":ValueReference" ) : filterPrefix + QStringLiteral( ":PropertyName" ) );
    functionElem.appendChild( valueReferenceElem );
    valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
    QDomElement literalElem = doc.createElement( filterPrefix + QStringLiteral( ":Literal" ) );
    propertyIsEqualToElem.appendChild( literalElem );
    literalElem.appendChild( doc.createTextNode( QStringLiteral( "false" ) ) );
  }

  return doc.toString();
}

QString QgsWfsProviderMetadata::suggestGroupNameForUri( const QString &uri ) const
{
  QgsWFSDataSourceURI wfsUri( uri );
  return wfsUri.typeName();
}


QList<QgsProviderSublayerDetails> QgsWfsProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback *feedback ) const
{
  QList<QgsProviderSublayerDetails> res;

  if ( ( flags & Qgis::SublayerQueryFlag::FastScan ) )
    return res;

  QgsWFSDataSourceURI wfsUri( uri );
  if ( !wfsUri.isValid() )
    return res;

  QgsWfsCapabilities::Capabilities caps = QgsWFSProvider::getCachedCapabilities( uri );
  if ( caps.version.isEmpty() )
    return res;

  QgsWFSProvider provider(
    uri + " " + QgsWFSConstants::URI_PARAM_SKIP_INITIAL_GET_FEATURE + "='true'",
    QgsDataProvider::ProviderOptions(), caps );
  QgsProviderSublayerDetails details;
  details.setType( QgsMapLayerType::VectorLayer );
  details.setProviderKey( QgsWFSProvider::WFS_PROVIDER_KEY );
  details.setUri( uri );
  details.setName( wfsUri.typeName() );
  details.setWkbType( provider.wkbType() );
  res << details;

  if ( wfsUri.hasGeometryTypeFilter() ||
       !caps.supportsGeometryTypeFilters() )
  {
    if ( provider.wkbType() == QgsWkbTypes::Type::Unknown )
      provider.issueInitialGetFeature();
    details.setWkbType( provider.wkbType() );
    return res;
  }

  if ( provider.wkbType() == QgsWkbTypes::Type::Unknown &&
       provider.sharedData()->layerProperties().size() == 1 )
  {
    std::vector<std::unique_ptr<QgsWFSGetFeature>> requests;
    std::set<QgsWFSGetFeature *> finishedRequests;
    constexpr int INDEX_ALL = 0;
    constexpr int INDEX_NULL = 1;
    constexpr int INDEX_POINT = 2;
    constexpr int INDEX_CURVE = 3;
    constexpr int INDEX_SURFACE = 4;
    // Order of strings in the list must be consistent with the INDEX_* enumeration above
    const QStringList filterNames = { QString(), // all features
                                      QString( "IsNull" ),
                                      QStringLiteral( "IsPoint" ),
                                      QStringLiteral( "IsCurve" ),
                                      QStringLiteral( "IsSurface" )
                                    };

    const auto downloaderLambda = [ &, feedback]()
    {
      QEventLoop loop;
      QTimer timerForHits;
      for ( QString function : filterNames )
      {
        QString filter;
        if ( function == QLatin1String( "IsNull" ) )
        {
          filter = QgsWFSProvider::buildIsNullGeometryFilter( caps, provider.geometryAttribute() );
        }
        else if ( !function.isEmpty() )
        {
          filter = QgsWFSProvider::buildFilterByGeometryType( caps,
                   provider.geometryAttribute(),
                   function );
        }

        if ( !provider.sharedData()->WFSFilter().isEmpty() )
        {
          filter = provider.sharedData()->combineWFSFilters( {filter, provider.sharedData()->WFSFilter()} );
        }

        requests.emplace_back( std::make_unique<QgsWFSGetFeature>( wfsUri ) );

        requests.back()->request( /* synchronous = */ false,
            caps.version,
            wfsUri.typeName(),
            filter,
            /* hitsOnly = */ true,
            caps );

        QgsWFSGetFeature *thisRequest = requests.back().get();
        const auto downloadFinishedLambda = [ &, thisRequest]()
        {
          finishedRequests.insert( thisRequest );
          if ( finishedRequests.size() == requests.size() )
            loop.quit();
        };
        connect( thisRequest, &QgsWfsRequest::downloadFinished, thisRequest, downloadFinishedLambda );
      }
      QgsSettings s;
      if ( !s.contains( QStringLiteral( "qgis/wfsGetFeatureGeometryTypesTimeout" ) ) )
      {
        s.setValue( QStringLiteral( "qgis/wfsGetFeatureGeometryTypesTimeout" ), 2.0 );
      }
      const double timeout = s.value( QStringLiteral( "qgis/wfsGetFeatureGeometryTypesTimeout" ), 2.0 ).toDouble();
      timerForHits.setInterval( static_cast<int>( timeout * 1000 ) );
      timerForHits.setSingleShot( true );
      timerForHits.start();
      connect( &timerForHits, &QTimer::timeout, &loop, &QEventLoop::quit );
      if ( feedback )
      {
        connect( feedback, &QgsFeedback::canceled, &loop, &QEventLoop::quit );
      }
      loop.exec( QEventLoop::ExcludeUserInputEvents );
      // Make sure to terminate requests in this thread, to avoid potential
      // crash in main thread when "requests" goes out of scope.
      for ( auto &request : requests )
      {
        request->abort();
      }
    };

    std::unique_ptr<_DownloaderThread> downloaderThread =
      std::make_unique<_DownloaderThread>( downloaderLambda );
    downloaderThread->start();
    downloaderThread->wait();

    constexpr int INDEX_GEOMETRYCOLLECTION = 5;
    std::vector<int64_t> featureCounts( INDEX_GEOMETRYCOLLECTION + 1, -1 );
    bool countsAllValid = false;
    if ( finishedRequests.size() == requests.size() )
    {
      countsAllValid = true;
      for ( size_t i = 0; i < requests.size(); ++i )
      {
        QByteArray data = requests[i]->response();
        QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
        QString errorMsg;
        if ( gmlParser.processData( data, true, errorMsg ) )
        {
          featureCounts[i] = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() :
                             gmlParser.numberReturned();
        }
        countsAllValid &= featureCounts[i] >= 0;
      }
    }

    if ( countsAllValid )
    {
      // Deduce numbers of geometry collections from other types
      featureCounts[INDEX_GEOMETRYCOLLECTION] = featureCounts[INDEX_ALL] -
          ( featureCounts[INDEX_NULL] + featureCounts[INDEX_POINT] + featureCounts[INDEX_CURVE] + featureCounts[INDEX_SURFACE] );
    }

    const struct
    {
      int index;
      QgsWkbTypes::Type wkbType;
    } types[] =
    {
      { INDEX_NULL, QgsWkbTypes::NoGeometry },
      { INDEX_POINT, QgsWkbTypes::MultiPoint },
      { INDEX_CURVE, QgsWkbTypes::MultiCurve },
      { INDEX_SURFACE, QgsWkbTypes::MultiSurface },
      { INDEX_GEOMETRYCOLLECTION, QgsWkbTypes::GeometryCollection },
    };

    // Create sublayers details
    res.clear();
    for ( const auto &tuple : types )
    {
      if ( !countsAllValid || featureCounts[tuple.index] > 0 ||
           ( tuple.wkbType == QgsWkbTypes::NoGeometry && featureCounts[INDEX_ALL] == 0 ) )
      {
        QgsProviderSublayerDetails details;
        details.setType( QgsMapLayerType::VectorLayer );
        details.setProviderKey( QgsWFSProvider::WFS_PROVIDER_KEY );
        details.setUri( uri + QStringLiteral( " " ) + QgsWFSConstants::URI_PARAM_GEOMETRY_TYPE_FILTER + QStringLiteral( "='" ) + QgsWkbTypes::displayString( tuple.wkbType ) + QStringLiteral( "'" ) );
        details.setName( wfsUri.typeName() + QStringLiteral( " " ) + QgsWkbTypes::translatedDisplayString( tuple.wkbType ) );
        details.setWkbType( tuple.wkbType );
        if ( countsAllValid )
          details.setFeatureCount( featureCounts[tuple.index] );
        res << details;
      }
    }

  }
  return res;
}

QgsWfsProviderMetadata::QgsWfsProviderMetadata():
  QgsProviderMetadata( QgsWFSProvider::WFS_PROVIDER_KEY, QgsWFSProvider::WFS_PROVIDER_DESCRIPTION ) {}

QIcon QgsWfsProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconWfs.svg" ) );
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN void *multipleProviderMetadataFactory()
{
  return new std::vector<QgsProviderMetadata *> { new QgsWfsProviderMetadata(), new QgsOapifProviderMetadata() };
}
#endif
