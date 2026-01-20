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

#include "qgswfsprovidermetadata.h"

#include "qgis.h"
#include "qgsfeedback.h"
#include "qgsgml.h"
#include "qgslogger.h"
#include "qgsoapifprovider.h"
#include "qgsogcutils.h"
#include "qgsprovidersublayerdetails.h"
#include "qgssettings.h"
#include "qgswfscapabilities.h"
#include "qgswfsconstants.h"
#include "qgswfsdataitems.h"
#include "qgswfsgetfeature.h"
#include "qgswfsprovider.h"
#include "qgswfsshareddata.h"

#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>

#include "moc_qgswfsprovidermetadata.cpp"

QgsDataProvider *QgsWfsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  Q_UNUSED( flags );
  return new QgsWFSProvider( uri, options );
}

QList<Qgis::LayerType> QgsWfsProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
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

QString QgsWFSProvider::buildFilterByGeometryType( const QgsWfsCapabilities &caps, const QString &geometryElement, const QString &function )
{
  /* Generate something like:
  <fes:Filter xmlns="http://www.opengis.net/fes/2.0">
    <fes:And>
      <fes:Not>
        <fes:PropertyIsNull>
          <fes:ValueReference>xplan:geltungsbereich</fes:ValueReference>
        </fes:PropertyIsNull>
      </fes:Not>
      <fes:PropertyIsEqualTo>
        <fes:Function name="IsPoint">
          <fes:ValueReference>xplan:geltungsbereich</fes:ValueReference>
        </fes:Function>
        <fes:Literal>false</fes:Literal>
      </fes:PropertyIsEqualTo>
    </fes:And>
  </fes:Filter>
  */

  QDomDocument doc;
  QDomElement filterElem = ( caps.version.startsWith( "2.0"_L1 ) ) ? doc.createElementNS( u"http://www.opengis.net/fes/2.0"_s, u"fes:Filter"_s ) : doc.createElementNS( u"http://www.opengis.net/ogc"_s, u"ogc:Filter"_s );
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? u"fes"_s : u"ogc"_s );

  QDomElement andElem = doc.createElement( filterPrefix + u":And"_s );
  filterElem.appendChild( andElem );
  {
    QDomElement notElem = doc.createElement( filterPrefix + u":Not"_s );
    andElem.appendChild( notElem );
    {
      QDomElement propertyIsNullElem = doc.createElement( filterPrefix + u":PropertyIsNull"_s );
      notElem.appendChild( propertyIsNullElem );
      QDomElement valueReferenceElem = doc.createElement(
        ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + u":ValueReference"_s : filterPrefix + u":PropertyName"_s
      );
      propertyIsNullElem.appendChild( valueReferenceElem );
      valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
    }
  }
  {
    QDomElement propertyIsEqualToElem = doc.createElement( filterPrefix + u":PropertyIsEqualTo"_s );
    andElem.appendChild( propertyIsEqualToElem );
    {
      QDomElement functionElem = doc.createElement( filterPrefix + u":Function"_s );
      propertyIsEqualToElem.appendChild( functionElem );
      {
        QDomAttr attrFunctionName = doc.createAttribute( u"name"_s );
        attrFunctionName.setValue( function );
        functionElem.setAttributeNode( attrFunctionName );
      }
      {
        QDomElement valueReferenceElem = doc.createElement(
          ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + u":ValueReference"_s : filterPrefix + u":PropertyName"_s
        );
        functionElem.appendChild( valueReferenceElem );
        valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
      }
    }
    {
      QDomElement literalElem = doc.createElement( filterPrefix + u":Literal"_s );
      propertyIsEqualToElem.appendChild( literalElem );
      literalElem.appendChild( doc.createTextNode( u"true"_s ) );
    }
  }

  return doc.toString();
}

QString QgsWFSProvider::buildIsNullGeometryFilter( const QgsWfsCapabilities &caps, const QString &geometryElement )
{
  /* Generate something like:
   <fes:Filter xmlns="http://www.opengis.net/fes/2.0">
    <fes:PropertyIsNull>
        <fes:ValueReference>xplan:geltungsbereich</fes:ValueReference>
    </fes:PropertyIsNull>
  </fes:Filter>
  */

  QDomDocument doc;
  QDomElement filterElem = ( caps.version.startsWith( "2.0"_L1 ) ) ? doc.createElementNS( u"http://www.opengis.net/fes/2.0"_s, u"fes:Filter"_s ) : doc.createElementNS( u"http://www.opengis.net/ogc"_s, u"ogc:Filter"_s );
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? u"fes"_s : u"ogc"_s );
  QDomElement propertyIsNullElem = doc.createElement( filterPrefix + u":PropertyIsNull"_s );
  filterElem.appendChild( propertyIsNullElem );
  QDomElement valueReferenceElem = doc.createElement(
    ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + u":ValueReference"_s : filterPrefix + u":PropertyName"_s
  );
  propertyIsNullElem.appendChild( valueReferenceElem );
  valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );

  return doc.toString();
}

QString QgsWFSProvider::buildGeometryCollectionFilter( const QgsWfsCapabilities &caps, const QString &geometryElement )
{
  QDomDocument doc;
  QDomElement filterElem = ( caps.version.startsWith( "2.0"_L1 ) ) ? doc.createElementNS( u"http://www.opengis.net/fes/2.0"_s, u"fes:Filter"_s ) : doc.createElementNS( u"http://www.opengis.net/ogc"_s, u"ogc:Filter"_s );
  doc.appendChild( filterElem );
  QString filterPrefix( caps.version.startsWith( "2.0" ) ? u"fes"_s : u"ogc"_s );

  QDomElement andElem = doc.createElement( filterPrefix + u":And"_s );
  filterElem.appendChild( andElem );

  {
    QDomElement notElem = doc.createElement( filterPrefix + u":Not"_s );
    andElem.appendChild( notElem );

    QDomElement propertyIsNullElem = doc.createElement( filterPrefix + u":PropertyIsNull"_s );
    notElem.appendChild( propertyIsNullElem );
    QDomElement valueReferenceElem = doc.createElement(
      ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + u":ValueReference"_s : filterPrefix + u":PropertyName"_s
    );
    propertyIsNullElem.appendChild( valueReferenceElem );
    valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
  }

  for ( const QString &function : { u"IsPoint"_s, u"IsCurve"_s, u"IsSurface"_s } )
  {
    QDomElement propertyIsEqualToElem = doc.createElement( filterPrefix + u":PropertyIsEqualTo"_s );
    andElem.appendChild( propertyIsEqualToElem );
    QDomElement functionElem = doc.createElement( filterPrefix + u":Function"_s );
    propertyIsEqualToElem.appendChild( functionElem );
    QDomAttr attrFunctionName = doc.createAttribute( u"name"_s );
    attrFunctionName.setValue( function );
    functionElem.setAttributeNode( attrFunctionName );
    QDomElement valueReferenceElem = doc.createElement(
      ( caps.version.startsWith( "2.0" ) ) ? filterPrefix + u":ValueReference"_s : filterPrefix + u":PropertyName"_s
    );
    functionElem.appendChild( valueReferenceElem );
    valueReferenceElem.appendChild( doc.createTextNode( geometryElement ) );
    QDomElement literalElem = doc.createElement( filterPrefix + u":Literal"_s );
    propertyIsEqualToElem.appendChild( literalElem );
    literalElem.appendChild( doc.createTextNode( u"false"_s ) );
  }

  return doc.toString();
}

QString QgsWfsProviderMetadata::suggestGroupNameForUri( const QString &uri ) const
{
  QgsWFSDataSourceURI wfsUri( uri );
  return wfsUri.typeName();
}

QString QgsWfsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
  {
    if ( it.key() == "authcfg"_L1 )
    {
      dsUri.setAuthConfigId( it.value().toString() );
    }
    else
    {
      dsUri.setParam( it.key(), it.value().toString() );
    }
  }
  return dsUri.uri( false );
}

QVariantMap QgsWfsProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap decoded;
  const QSet<QString> parameterKeys = dsUri.parameterKeys();
  for ( const QString &key : std::as_const( parameterKeys ) )
  {
    decoded.insert( key, dsUri.param( key ) );
  }
  return decoded;
}

QList<QgsProviderSublayerDetails> QgsWfsProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback *feedback ) const
{
  QList<QgsProviderSublayerDetails> res;

  if ( ( flags & Qgis::SublayerQueryFlag::FastScan ) )
    return res;

  QgsWFSDataSourceURI wfsUri( uri );
  if ( !wfsUri.isValid() )
    return res;

  QgsWfsCapabilities caps = QgsWFSProvider::getCachedCapabilities( uri );
  if ( caps.version.isEmpty() )
    return res;

  QgsDataSourceUri dsUri( uri );
  dsUri.removeParam( QgsWFSConstants::URI_PARAM_SKIP_INITIAL_GET_FEATURE );
  dsUri.setParam( QgsWFSConstants::URI_PARAM_SKIP_INITIAL_GET_FEATURE, u"true"_s );

  QgsWFSProvider provider(
    dsUri.uri( false ),
    QgsDataProvider::ProviderOptions(), caps
  );

  if ( provider.metadataRetrievalCanceled() )
    return res;

  QgsProviderSublayerDetails details;
  details.setType( Qgis::LayerType::Vector );
  details.setProviderKey( QgsWFSProvider::WFS_PROVIDER_KEY );
  details.setUri( uri );
  details.setName( wfsUri.typeName() );
  details.setWkbType( provider.wkbType() );
  res << details;

  // If set: always issue a GetFeature because the guessed type can't be trusted,
  // for example when dealing with Z geometries identified as 2D.
  const bool forceInitialGetFeature = dsUri.hasParam( QgsWFSConstants::URI_PARAM_FORCE_INITIAL_GET_FEATURE )
                                      && dsUri.param( QgsWFSConstants::URI_PARAM_FORCE_INITIAL_GET_FEATURE ).toUpper() == "TRUE"_L1;

  if ( wfsUri.hasGeometryTypeFilter() || !caps.supportsGeometryTypeFilters() )
  {
    if ( provider.wkbType() == Qgis::WkbType::Unknown || forceInitialGetFeature )
      provider.issueInitialGetFeature( forceInitialGetFeature );

    res.last().setWkbType( provider.wkbType() );
    return res;
  }

  if ( forceInitialGetFeature )
  {
    provider.issueInitialGetFeature( true );
    res.last().setWkbType( provider.wkbType() );
  }

  if ( ( provider.wkbType() == Qgis::WkbType::Unknown || ( provider.wkbType() != Qgis::WkbType::NoGeometry && provider.geometryMaybeMissing() ) ) && provider.sharedData()->layerProperties().size() == 1 )
  {
    std::map<int, std::unique_ptr<QgsWFSGetFeature>> requests;
    std::set<QgsWFSGetFeature *> finishedRequests;
    constexpr int INDEX_ALL = 0;
    constexpr int INDEX_NULL = 1;
    constexpr int INDEX_POINT = 2;
    constexpr int INDEX_CURVE = 3;
    constexpr int INDEX_SURFACE = 4;
    // Order of strings in the list must be consistent with the INDEX_* enumeration above
    const QStringList filterNames = { QString(), // all features
                                      QString( "IsNull" ), u"IsPoint"_s, u"IsCurve"_s, u"IsSurface"_s };

    constexpr int INDEX_GEOMETRYCOLLECTION = 5;
    std::vector<int64_t> featureCounts( INDEX_GEOMETRYCOLLECTION + 1, -1 );

    const auto downloaderLambda = [&, feedback]() {
      QEventLoop loop;
      QTimer timerForHits;
      for ( int i = 0; i <= INDEX_SURFACE; ++i )
      {
        if ( provider.wkbType() == Qgis::WkbType::MultiPoint )
        {
          if ( i != INDEX_ALL && i != INDEX_NULL && i != INDEX_POINT )
          {
            featureCounts[i] = 0;
            continue;
          }
        }
        else if ( provider.wkbType() == Qgis::WkbType::MultiCurve )
        {
          if ( i != INDEX_ALL && i != INDEX_NULL && i != INDEX_CURVE )
          {
            featureCounts[i] = 0;
            continue;
          }
        }
        else if ( provider.wkbType() == Qgis::WkbType::MultiSurface )
        {
          if ( i != INDEX_ALL && i != INDEX_NULL && i != INDEX_SURFACE )
          {
            featureCounts[i] = 0;
            continue;
          }
        }

        QString filter;
        const QString &function = filterNames[i];
        if ( function == "IsNull"_L1 )
        {
          filter = QgsWFSProvider::buildIsNullGeometryFilter( caps, provider.geometryColumnName() );
        }
        else if ( !function.isEmpty() )
        {
          filter = QgsWFSProvider::buildFilterByGeometryType( caps, provider.geometryColumnName(), function );
        }

        if ( !provider.sharedData()->WFSFilter().isEmpty() )
        {
          filter = provider.sharedData()->combineWFSFilters( { filter, provider.sharedData()->WFSFilter() } );
        }

        requests[i] = std::make_unique<QgsWFSGetFeature>( wfsUri );
        QgsWFSGetFeature *thisRequest = requests[i].get();

        thisRequest->request( /* synchronous = */ false, caps.version, wfsUri.typeName(), filter,
                              /* hitsOnly = */ true, caps );

        const auto downloadFinishedLambda = [&, thisRequest]() {
          finishedRequests.insert( thisRequest );
          if ( finishedRequests.size() == requests.size() )
            loop.quit();
        };
        connect( thisRequest, &QgsWfsRequest::downloadFinished, thisRequest, downloadFinishedLambda );
      }
      QgsSettings s;
      if ( !s.contains( u"qgis/wfsGetFeatureGeometryTypesTimeout"_s ) )
      {
        s.setValue( u"qgis/wfsGetFeatureGeometryTypesTimeout"_s, 2.0 );
      }
      const double timeout = s.value( u"qgis/wfsGetFeatureGeometryTypesTimeout"_s, 2.0 ).toDouble();
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
      for ( auto &pair : requests )
      {
        pair.second->abort();
      }
    };

    auto downloaderThread = std::make_unique<_DownloaderThread>( downloaderLambda );
    downloaderThread->start();
    downloaderThread->wait();

    bool countsAllValid = false;
    if ( finishedRequests.size() == requests.size() )
    {
      countsAllValid = true;
      for ( const auto &pair : requests )
      {
        const int i = pair.first;
        QByteArray data = pair.second->response();
        QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
        QString errorMsg;
        if ( gmlParser.processData( data, true, errorMsg ) )
        {
          featureCounts[i] = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() : gmlParser.numberReturned();
        }
        countsAllValid &= featureCounts[i] >= 0;
      }
    }

    if ( countsAllValid )
    {
      // Some servers are buggy and actually ignore the geometry type filter
      // So if the Point, Curve and Surface filters return the same number of
      // features than the No filter request, consider that the geometry type
      // is unknown and try sampling one feature to guess the type
      // e.g with https://geodienste.komm.one/ows/services/org.273.561ba9e8-9b66-45a2-98db-17920e10c53d_wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=xplan:BP_Plan&FILTER=%3Cfes:Filter%20xmlns:fes%3D%22http://www.opengis.net/fes/2.0%22%3E%0A%20%3Cfes:PropertyIsEqualTo%3E%0A%20%20%3Cfes:Function%20name%3D%22IsCurve%22%3E%0A%20%20%20%3Cfes:ValueReference%3EraeumlicherGeltungsbereich%3C/fes:ValueReference%3E%0A%20%20%3C/fes:Function%3E%0A%20%20%3Cfes:Literal%3Etrue%3C/fes:Literal%3E%0A%20%3C/fes:PropertyIsEqualTo%3E%0A%3C/fes:Filter%3E%0A&RESULTTYPE=hits
      if ( featureCounts[INDEX_ALL] > 0 && featureCounts[INDEX_POINT] == featureCounts[INDEX_ALL] && featureCounts[INDEX_CURVE] == featureCounts[INDEX_ALL] && featureCounts[INDEX_SURFACE] == featureCounts[INDEX_ALL] )
      {
        QgsDebugMsgLevel( QString( "%1 declares geometry filters, but they are not working. Guessing the geometry type from one sample" ).arg( uri ), 2 );
        provider.issueInitialGetFeature();
        res.last().setWkbType( provider.wkbType() );
        res.last().setFeatureCount( featureCounts[INDEX_ALL] );
        return res;
      }

      // Deduce numbers of geometry collections from other types
      featureCounts[INDEX_GEOMETRYCOLLECTION] = featureCounts[INDEX_ALL] - ( featureCounts[INDEX_NULL] + featureCounts[INDEX_POINT] + featureCounts[INDEX_CURVE] + featureCounts[INDEX_SURFACE] );
    }

    const struct
    {
        int index;
        Qgis::WkbType wkbType;
    } types[] = {
      { INDEX_NULL, Qgis::WkbType::NoGeometry },
      { INDEX_POINT, Qgis::WkbType::MultiPoint },
      { INDEX_CURVE, Qgis::WkbType::MultiCurve },
      { INDEX_SURFACE, Qgis::WkbType::MultiSurface },
      { INDEX_GEOMETRYCOLLECTION, Qgis::WkbType::GeometryCollection },
    };

    // Create sublayers details
    res.clear();
    for ( const auto &tuple : types )
    {
      if ( provider.wkbType() == Qgis::WkbType::MultiPoint && tuple.index != INDEX_NULL && tuple.index != INDEX_POINT )
        continue;
      if ( provider.wkbType() == Qgis::WkbType::MultiCurve && tuple.index != INDEX_NULL && tuple.index != INDEX_CURVE )
        continue;
      if ( provider.wkbType() == Qgis::WkbType::MultiSurface && tuple.index != INDEX_NULL && tuple.index != INDEX_SURFACE )
        continue;

      if ( !countsAllValid || featureCounts[tuple.index] > 0 || ( tuple.wkbType == Qgis::WkbType::NoGeometry && featureCounts[INDEX_ALL] == 0 ) )
      {
        QgsProviderSublayerDetails details;
        details.setType( Qgis::LayerType::Vector );
        details.setProviderKey( QgsWFSProvider::WFS_PROVIDER_KEY );
        details.setUri( uri + u" "_s + QgsWFSConstants::URI_PARAM_GEOMETRY_TYPE_FILTER + u"='"_s + QgsWkbTypes::displayString( tuple.wkbType ) + u"'"_s );
        details.setName( wfsUri.typeName() + u" "_s + QgsWkbTypes::translatedDisplayString( tuple.wkbType ) );
        details.setWkbType( tuple.wkbType );
        if ( countsAllValid )
          details.setFeatureCount( featureCounts[tuple.index] );
        res << details;
      }
    }
  }
  return res;
}

QgsWfsProviderMetadata::QgsWfsProviderMetadata()
  : QgsProviderMetadata( QgsWFSProvider::WFS_PROVIDER_KEY, QgsWFSProvider::WFS_PROVIDER_DESCRIPTION ) {}

QIcon QgsWfsProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconWfs.svg"_s );
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN void *multipleProviderMetadataFactory()
{
  return new std::vector<QgsProviderMetadata *> { new QgsWfsProviderMetadata(), new QgsOapifProviderMetadata() };
}
#endif
