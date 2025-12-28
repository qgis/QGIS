/***************************************************************************
    qgswfscapabilities.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsgetcapabilities.h"

#include <cpl_minixml.h>

#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsogcutils.h"
#include "qgssettings.h"
#include "qgswfsconstants.h"
#include "qgswfsutils.h"

#include <QDomDocument>
#include <QRegularExpression>
#include <QStringList>
#include <QUrlQuery>

#include "moc_qgswfsgetcapabilities.cpp"

QgsWfsGetCapabilitiesRequest::QgsWfsGetCapabilitiesRequest( const QString &uri, const QgsDataProvider::ProviderOptions &options )
  : QgsWfsRequest( QgsWFSDataSourceURI( uri ) ), mOptions( options )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsWfsRequest::downloadFinished, this, &QgsWfsGetCapabilitiesRequest::capabilitiesReplyFinished, Qt::DirectConnection );
}

QUrl QgsWfsGetCapabilitiesRequest::requestUrl() const
{
  QUrl url( mUri.baseURL() );
  QUrlQuery query( url );
  query.addQueryItem( u"REQUEST"_s, u"GetCapabilities"_s );

  const QString &version = mUri.version();
  if ( version == QgsWFSConstants::VERSION_AUTO )
    // MapServer honours the order with the first value being the preferred one
    query.addQueryItem( u"ACCEPTVERSIONS"_s, u"2.0.0,1.1.0,1.0.0"_s );
  else
    query.addQueryItem( u"VERSION"_s, version );

  url.setQuery( query );
  return url;
}

bool QgsWfsGetCapabilitiesRequest::requestCapabilities( bool synchronous, bool forceRefresh )
{
  if ( !sendGET( requestUrl(), QString(), synchronous, forceRefresh ) )
  {
    emit gotCapabilities();
    return false;
  }
  return true;
}

class CPLXMLTreeUniquePointer
{
  public:
    //! Constructor
    explicit CPLXMLTreeUniquePointer( CPLXMLNode *data ) { the_data_ = data; }

    //! Destructor
    ~CPLXMLTreeUniquePointer()
    {
      if ( the_data_ )
        CPLDestroyXMLNode( the_data_ );
    }

    /**
     * Returns the node pointer/
     * Modifying the contents pointed to by the return is allowed.
     * \return the node pointer
    */
    CPLXMLNode *get() const { return the_data_; }

    /**
     * Returns the node pointer/
     * Modifying the contents pointed to by the return is allowed.
     * \return the node pointer
    */
    CPLXMLNode *operator->() const { return get(); }

  private:
    CPLXMLNode *the_data_;
};


void QgsWfsGetCapabilitiesRequest::capabilitiesReplyFinished()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    emit gotCapabilities();
    return;
  }
  const QByteArray &buffer = mResponse;

  QgsDebugMsgLevel( u"parsing capabilities: "_s + buffer, 4 );

  // parse XML
  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    mErrorCode = QgsWfsRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::XmlError;
    mErrorMessage = capabilitiesDocError;
    emit gotCapabilities();
    return;
  }

  CPLXMLTreeUniquePointer oCPLXML( CPLParseXMLString( buffer.constData() ) );

  QDomElement doc = capabilitiesDocument.documentElement();

  // handle exceptions
  if ( doc.tagName() == "ExceptionReport"_L1 )
  {
    QDomNode ex = doc.firstChild(); // Get Exception element
    QString exc = ex.toElement().attribute( u"exceptionCode"_s, u"Exception"_s );
    QDomElement ext = ex.firstChild().toElement(); // Get ExceptionText element
    mErrorCode = QgsWfsRequest::ServerExceptionError;
    mErrorMessage = exc + ": " + ext.firstChild().nodeValue();
    emit gotCapabilities();
    return;
  }

  // handle WMS exceptions as well (to be nice with users...)
  // See https://github.com/qgis/QGIS/issues/29866
  if ( doc.tagName() == "ServiceExceptionReport"_L1 )
  {
    QDomNode ex = doc.firstChild(); // Get ServiceExceptionReport element
    QString exc = ex.toElement().attribute( u"code"_s, u"Exception"_s );
    QDomElement ext = ex.toElement();
    mErrorCode = QgsWfsRequest::ServerExceptionError;
    mErrorMessage = exc + ": " + ext.firstChild().nodeValue().trimmed();
    emit gotCapabilities();
    return;
  }

  mCaps.clear();

  //test wfs version
  mCaps.version = doc.attribute( u"version"_s );
  if ( !mCaps.version.startsWith( "1.0"_L1 ) && !mCaps.version.startsWith( "1.1"_L1 ) && !mCaps.version.startsWith( "2.0"_L1 ) )
  {
    mErrorCode = QgsWfsRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::VersionNotSupported;
    mErrorMessage = tr( "WFS version %1 not supported" ).arg( mCaps.version );
    emit gotCapabilities();
    return;
  }

  // WFS 2.0 implementation are supposed to implement resultType=hits, and some
  // implementations (GeoServer) might advertise it, whereas others (MapServer) do not.
  // WFS 1.1 implementation too I think, but in the examples of the GetCapabilities
  // response of the WFS 1.1 standard (and in common implementations), this is
  // explicitly advertised
  if ( mCaps.version.startsWith( "2.0"_L1 ) )
    mCaps.supportsHits = true;

  // Note: for conveniency, we do not use the elementsByTagNameNS() method as
  // the WFS and OWS namespaces URI are not the same in all versions

  if ( mCaps.version.startsWith( "1.0"_L1 ) )
  {
    QDomElement capabilityElem = doc.firstChildElement( u"Capability"_s );
    if ( !capabilityElem.isNull() )
    {
      QDomElement requestElem = capabilityElem.firstChildElement( u"Request"_s );
      if ( !requestElem.isNull() )
      {
        QDomElement getFeatureElem = requestElem.firstChildElement( u"GetFeature"_s );
        if ( !getFeatureElem.isNull() )
        {
          QDomElement resultFormatElem = getFeatureElem.firstChildElement( u"ResultFormat"_s );
          if ( !resultFormatElem.isNull() )
          {
            QDomElement child = resultFormatElem.firstChildElement();
            while ( !child.isNull() )
            {
              mCaps.outputFormats << child.tagName();
              child = child.nextSiblingElement();
            }
          }
        }
      }
    }
  }

  // find <ows:OperationsMetadata>
  QDomElement operationsMetadataElem = doc.firstChildElement( u"OperationsMetadata"_s );
  if ( !operationsMetadataElem.isNull() )
  {
    QDomNodeList constraintList = operationsMetadataElem.elementsByTagName( u"Constraint"_s );
    for ( int i = 0; i < constraintList.size(); ++i )
    {
      QDomElement constraint = constraintList.at( i ).toElement();
      if ( constraint.attribute( u"name"_s ) == "DefaultMaxFeatures"_L1 /* WFS 1.1 */ )
      {
        QDomElement value = constraint.firstChildElement( u"Value"_s );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsgLevel( u"maxFeatures: %1"_s.arg( mCaps.maxFeatures ), 2 );
        }
      }
      else if ( constraint.attribute( u"name"_s ) == "CountDefault"_L1 /* WFS 2.0 (e.g. MapServer) */ )
      {
        QDomElement value = constraint.firstChildElement( u"DefaultValue"_s );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsgLevel( u"maxFeatures: %1"_s.arg( mCaps.maxFeatures ), 2 );
        }
      }
      else if ( constraint.attribute( u"name"_s ) == "ImplementsResultPaging"_L1 /* WFS 2.0 */ )
      {
        QDomElement value = constraint.firstChildElement( u"DefaultValue"_s );
        if ( !value.isNull() && value.text() == "TRUE"_L1 )
        {
          mCaps.supportsPaging = true;
          QgsDebugMsgLevel( u"Supports paging"_s, 2 );
        }
      }
      else if ( constraint.attribute( u"name"_s ) == "ImplementsStandardJoins"_L1 || constraint.attribute( u"name"_s ) == "ImplementsSpatialJoins"_L1 /* WFS 2.0 */ )
      {
        QDomElement value = constraint.firstChildElement( u"DefaultValue"_s );
        if ( !value.isNull() && value.text() == "TRUE"_L1 )
        {
          mCaps.supportsJoins = true;
          QgsDebugMsgLevel( u"Supports joins"_s, 2 );
        }
      }
    }

    // In WFS 2.0, max features can also be set in Operation.GetFeature (e.g. GeoServer)
    // and we are also interested by resultType=hits for WFS 1.1
    QDomNodeList operationList = operationsMetadataElem.elementsByTagName( u"Operation"_s );
    for ( int i = 0; i < operationList.size(); ++i )
    {
      QDomElement operation = operationList.at( i ).toElement();
      QString name = operation.attribute( u"name"_s );

      // Search for DCP/HTTP
      QDomNodeList operationHttpList = operation.elementsByTagName( u"HTTP"_s );
      for ( int j = 0; j < operationHttpList.size(); ++j )
      {
        QDomElement value = operationHttpList.at( j ).toElement();
        QDomNodeList httpGetMethodList = value.elementsByTagName( u"Get"_s );
        QDomNodeList httpPostMethodList = value.elementsByTagName( u"Post"_s );
        if ( httpGetMethodList.size() > 0 )
        {
          mCaps.operationGetEndpoints[name] = httpGetMethodList.at( 0 ).toElement().attribute( u"href"_s );
          QgsDebugMsgLevel( u"Adding DCP Get %1 %2"_s.arg( name, mCaps.operationGetEndpoints[name] ), 3 );
        }
        if ( httpPostMethodList.size() > 0 )
        {
          mCaps.operationPostEndpoints[name] = httpPostMethodList.at( 0 ).toElement().attribute( u"href"_s );
          QgsDebugMsgLevel( u"Adding DCP Post %1 %2"_s.arg( name, mCaps.operationPostEndpoints[name] ), 3 );
        }
      }

      if ( name == "GetFeature"_L1 )
      {
        QDomNodeList operationConstraintList = operation.elementsByTagName( u"Constraint"_s );
        for ( int j = 0; j < operationConstraintList.size(); ++j )
        {
          QDomElement constraint = operationConstraintList.at( j ).toElement();
          if ( constraint.attribute( u"name"_s ) == "CountDefault"_L1 )
          {
            QDomElement value = constraint.firstChildElement( u"DefaultValue"_s );
            if ( !value.isNull() )
            {
              mCaps.maxFeatures = value.text().toInt();
              QgsDebugMsgLevel( u"maxFeatures: %1"_s.arg( mCaps.maxFeatures ), 2 );
            }
            break;
          }
        }

        QDomNodeList parameterList = operation.elementsByTagName( u"Parameter"_s );
        for ( int j = 0; j < parameterList.size(); ++j )
        {
          QDomElement parameter = parameterList.at( j ).toElement();
          if ( parameter.attribute( u"name"_s ) == "resultType"_L1 )
          {
            QDomNodeList valueList = parameter.elementsByTagName( u"Value"_s );
            for ( int k = 0; k < valueList.size(); ++k )
            {
              QDomElement value = valueList.at( k ).toElement();
              if ( value.text() == "hits"_L1 )
              {
                mCaps.supportsHits = true;
                QgsDebugMsgLevel( u"Support hits"_s, 2 );
                break;
              }
            }
          }
          else if ( parameter.attribute( u"name"_s ) == "outputFormat"_L1 )
          {
            QDomNodeList valueList = parameter.elementsByTagName( u"Value"_s );
            for ( int k = 0; k < valueList.size(); ++k )
            {
              QDomElement value = valueList.at( k ).toElement();
              mCaps.outputFormats << value.text();
            }
          }
        }
      }
    }
  }

  //go to <FeatureTypeList>
  QDomElement featureTypeListElem = doc.firstChildElement( u"FeatureTypeList"_s );
  if ( featureTypeListElem.isNull() )
  {
    emit gotCapabilities();
    return;
  }

  // Parse operations supported for all feature types
  bool insertCap = false;
  bool updateCap = false;
  bool deleteCap = false;
  // WFS < 2
  if ( mCaps.version.startsWith( QLatin1Char( '1' ) ) )
  {
    parseSupportedOperations( featureTypeListElem.firstChildElement( u"Operations"_s ), insertCap, updateCap, deleteCap );
  }
  else // WFS 2.0.0 tested on GeoServer
  {
    QDomNodeList operationNodes = doc.elementsByTagName( u"Operation"_s );
    for ( int i = 0; i < operationNodes.count(); i++ )
    {
      QDomElement operationElement = operationNodes.at( i ).toElement();
      if ( operationElement.isElement() && "Transaction" == operationElement.attribute( u"name"_s ) )
      {
        insertCap = true;
        updateCap = true;
        deleteCap = true;
      }
    }
  }

  // This is messy, but there's apparently no way to get the xmlns:ci attribute value with QDom API
  // in snippets like
  //  <wfs:FeatureType xmlns:ci="http://www.interactive-instruments.de/namespaces/demo/cities/4.0/cities">
  //    <wfs:Name>ci:City</wfs:Name>
  // so fallback to using GDAL XML parser for that...

  CPLXMLNode *psFeatureTypeIter = nullptr;
  if ( oCPLXML.get() )
  {
    psFeatureTypeIter = CPLGetXMLNode( oCPLXML.get(), "=wfs:WFS_Capabilities.wfs:FeatureTypeList" );
    // also try FeatureTypeList without prefix:
    psFeatureTypeIter = psFeatureTypeIter ? psFeatureTypeIter : CPLGetXMLNode( oCPLXML.get(), "=wfs:WFS_Capabilities.FeatureTypeList" );
    if ( psFeatureTypeIter )
      psFeatureTypeIter = psFeatureTypeIter->psChild;
  }

  // get the <FeatureType> elements
  QDomNodeList featureTypeList = featureTypeListElem.elementsByTagName( u"FeatureType"_s );
  for ( int i = 0; i < featureTypeList.size(); ++i )
  {
    QgsWfsCapabilities::FeatureType featureType;
    QDomElement featureTypeElem = featureTypeList.at( i ).toElement();

    for ( ; psFeatureTypeIter; psFeatureTypeIter = psFeatureTypeIter->psNext )
    {
      if ( psFeatureTypeIter->eType != CXT_Element )
        continue;
      break;
    }

    //Name
    QDomNodeList nameList = featureTypeElem.elementsByTagName( u"Name"_s );
    if ( nameList.length() > 0 )
    {
      featureType.name = nameList.at( 0 ).toElement().text();

      QgsDebugMsgLevel( u"featureType.name = %1"_s.arg( featureType.name ), 4 );
      if ( featureType.name.contains( ':' ) )
      {
        QString prefixOfTypename = featureType.name.section( ':', 0, 0 );

        // for some Deegree servers that requires a NAMESPACES parameter for GetFeature
        if ( psFeatureTypeIter )
        {
          featureType.nameSpace = CPLGetXMLValue( psFeatureTypeIter, ( "xmlns:" + prefixOfTypename ).toUtf8().constData(), "" );
          if ( featureType.nameSpace.isEmpty() )
          {
            //Try to look for namespace in Name tag (Seen in GO Publisher)
            //<wfs:FeatureType>
            // <wfs:Name xmlns:dagi="http://data.gov.dk/schemas/dagi/2/gml3sfp">dagi:Menighedsraadsafstemningsomraade</wfs:Name>
            // <wfs:Title>Menighedsraadsafstemningsomraade</wfs:Title>
            featureType.nameSpace = CPLGetXMLValue( psFeatureTypeIter, ( "wfs:Name.xmlns:" + prefixOfTypename ).toUtf8().constData(), "" );
          }
        }
      }
    }

    if ( psFeatureTypeIter )
      psFeatureTypeIter = psFeatureTypeIter->psNext;

    //Title
    QDomNodeList titleList = featureTypeElem.elementsByTagName( u"Title"_s );
    if ( titleList.length() > 0 )
    {
      featureType.title = titleList.at( 0 ).toElement().text();
    }
    //Abstract
    QDomNodeList abstractList = featureTypeElem.elementsByTagName( u"Abstract"_s );
    if ( abstractList.length() > 0 )
    {
      featureType.abstract = abstractList.at( 0 ).toElement().text();
    }

    //DefaultSRS is always the first entry in the feature srs list
    QDomNodeList defaultCRSList = featureTypeElem.elementsByTagName( u"DefaultSRS"_s );
    if ( defaultCRSList.length() == 0 )
      // In WFS 2.0, this is spelled DefaultCRS...
      defaultCRSList = featureTypeElem.elementsByTagName( u"DefaultCRS"_s );
    if ( defaultCRSList.length() > 0 )
    {
      QString srsname( defaultCRSList.at( 0 ).toElement().text() );
      // Some servers like Geomedia advertise EPSG:XXXX even in WFS 1.1 or 2.0
      if ( srsname.startsWith( "EPSG:"_L1 ) )
        mCaps.useEPSGColumnFormat = true;
      featureType.crslist.append( NormalizeSRSName( srsname ) );
    }

    //OtherSRS
    QDomNodeList otherCRSList = featureTypeElem.elementsByTagName( u"OtherSRS"_s );
    if ( otherCRSList.length() == 0 )
      // In WFS 2.0, this is spelled OtherCRS...
      otherCRSList = featureTypeElem.elementsByTagName( u"OtherCRS"_s );
    for ( int i = 0; i < otherCRSList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( otherCRSList.at( i ).toElement().text() ) );
    }

    //Support <SRS> for compatibility with older versions
    QDomNodeList srsList = featureTypeElem.elementsByTagName( u"SRS"_s );
    for ( int i = 0; i < srsList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( srsList.at( i ).toElement().text() ) );
    }

    // Get BBox WFS 1.0 way
    QDomElement latLongBB = featureTypeElem.firstChildElement( u"LatLongBoundingBox"_s );
    if ( latLongBB.hasAttributes() )
    {
      // Despite the name LatLongBoundingBox, the coordinates are supposed to
      // be expressed in <SRS>. From the WFS schema;
      // <!-- The LatLongBoundingBox element is used to indicate the edges of
      // an enclosing rectangle in the SRS of the associated feature type.
      featureType.bbox = QgsRectangle(
        latLongBB.attribute( u"minx"_s ).toDouble(),
        latLongBB.attribute( u"miny"_s ).toDouble(),
        latLongBB.attribute( u"maxx"_s ).toDouble(),
        latLongBB.attribute( u"maxy"_s ).toDouble()
      );
      featureType.bboxSRSIsWGS84 = false;

      // But some servers do not honour this and systematically reproject to WGS84
      // such as GeoServer. See http://osgeo-org.1560.x6.nabble.com/WFS-LatLongBoundingBox-td3813810.html
      // This is also true of TinyOWS
      if ( !featureType.crslist.isEmpty() && featureType.bbox.xMinimum() >= -180 && featureType.bbox.yMinimum() >= -90 && featureType.bbox.xMaximum() <= 180 && featureType.bbox.yMaximum() < 90 )
      {
        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( featureType.crslist[0] );
        if ( !crs.isGeographic() )
        {
          // If the CRS is projected then check that projecting the corner of the bbox, assumed to be in WGS84,
          // into the CRS, and then back to WGS84, works (check that we are in the validity area)
          QgsCoordinateReferenceSystem crsWGS84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"CRS:84"_s );

          QgsCoordinateTransform ct( crsWGS84, crs, mOptions.transformContext );
          try
          {
            QgsPointXY ptMin( featureType.bbox.xMinimum(), featureType.bbox.yMinimum() );
            QgsPointXY ptMinBack( ct.transform( ct.transform( ptMin, Qgis::TransformDirection::Forward ), Qgis::TransformDirection::Reverse ) );
            QgsPointXY ptMax( featureType.bbox.xMaximum(), featureType.bbox.yMaximum() );
            QgsPointXY ptMaxBack( ct.transform( ct.transform( ptMax, Qgis::TransformDirection::Forward ), Qgis::TransformDirection::Reverse ) );

            QgsDebugMsgLevel( featureType.bbox.toString(), 2 );
            QgsDebugMsgLevel( ptMinBack.toString(), 2 );
            QgsDebugMsgLevel( ptMaxBack.toString(), 2 );

            if ( std::fabs( featureType.bbox.xMinimum() - ptMinBack.x() ) < 1e-5 && std::fabs( featureType.bbox.yMinimum() - ptMinBack.y() ) < 1e-5 && std::fabs( featureType.bbox.xMaximum() - ptMaxBack.x() ) < 1e-5 && std::fabs( featureType.bbox.yMaximum() - ptMaxBack.y() ) < 1e-5 )
            {
              QgsDebugMsgLevel( u"Values of LatLongBoundingBox are consistent with WGS84 long/lat bounds, so as the CRS is projected, assume they are indeed in WGS84 and not in the CRS units"_s, 2 );
              featureType.bboxSRSIsWGS84 = true;
            }
          }
          catch ( const QgsCsException & )
          {
            // can be silently ignored
          }
        }
      }
    }
    else
    {
      // WFS 1.1 way
      QDomElement WGS84BoundingBox = featureTypeElem.firstChildElement( u"WGS84BoundingBox"_s );
      if ( !WGS84BoundingBox.isNull() )
      {
        QDomElement lowerCorner = WGS84BoundingBox.firstChildElement( u"LowerCorner"_s );
        QDomElement upperCorner = WGS84BoundingBox.firstChildElement( u"UpperCorner"_s );
        if ( !lowerCorner.isNull() && !upperCorner.isNull() )
        {
          QStringList lowerCornerList = lowerCorner.text().split( u" "_s, Qt::SkipEmptyParts );
          QStringList upperCornerList = upperCorner.text().split( u" "_s, Qt::SkipEmptyParts );
          if ( lowerCornerList.size() == 2 && upperCornerList.size() == 2 )
          {
            featureType.bbox = QgsRectangle(
              lowerCornerList[0].toDouble(),
              lowerCornerList[1].toDouble(),
              upperCornerList[0].toDouble(),
              upperCornerList[1].toDouble()
            );
            featureType.bboxSRSIsWGS84 = true;
          }
        }
      }
    }

    // Parse Operations specific to the type name
    parseSupportedOperations( featureTypeElem.firstChildElement( u"Operations"_s ), featureType.insertCap, featureType.updateCap, featureType.deleteCap );
    featureType.insertCap |= insertCap;
    featureType.updateCap |= updateCap;
    featureType.deleteCap |= deleteCap;

    mCaps.featureTypes.push_back( featureType );
  }

  for ( const QgsWfsCapabilities::FeatureType &f : std::as_const( mCaps.featureTypes ) )
  {
    mCaps.setAllTypenames.insert( f.name );
    QString unprefixed( QgsWFSUtils::removeNamespacePrefix( f.name ) );
    if ( !mCaps.setAmbiguousUnprefixedTypename.contains( unprefixed ) )
    {
      if ( mCaps.mapUnprefixedTypenameToPrefixedTypename.contains( unprefixed ) )
      {
        mCaps.setAmbiguousUnprefixedTypename.insert( unprefixed );
        mCaps.mapUnprefixedTypenameToPrefixedTypename.remove( unprefixed );
      }
      else
      {
        mCaps.mapUnprefixedTypenameToPrefixedTypename[unprefixed] = f.name;
      }
    }
  }

  //go to <Filter_Capabilities>
  QDomElement filterCapabilitiesElem = doc.firstChildElement( u"Filter_Capabilities"_s );
  if ( !filterCapabilitiesElem.isNull() )
    parseFilterCapabilities( filterCapabilitiesElem );

  // Hard-coded functions
  QgsWfsCapabilities::Function f_ST_GeometryFromText( u"ST_GeometryFromText"_s, 1, 2 );
  f_ST_GeometryFromText.returnType = u"gml:AbstractGeometryType"_s;
  f_ST_GeometryFromText.argumentList << QgsWfsCapabilities::Argument( u"wkt"_s, u"xs:string"_s );
  f_ST_GeometryFromText.argumentList << QgsWfsCapabilities::Argument( u"srsname"_s, u"xs:string"_s );
  mCaps.functionList << f_ST_GeometryFromText;

  QgsWfsCapabilities::Function f_ST_GeomFromGML( u"ST_GeomFromGML"_s, 1 );
  f_ST_GeomFromGML.returnType = u"gml:AbstractGeometryType"_s;
  f_ST_GeomFromGML.argumentList << QgsWfsCapabilities::Argument( u"gml"_s, u"xs:string"_s );
  mCaps.functionList << f_ST_GeomFromGML;

  QgsWfsCapabilities::Function f_ST_MakeEnvelope( u"ST_MakeEnvelope"_s, 4, 5 );
  f_ST_MakeEnvelope.returnType = u"gml:AbstractGeometryType"_s;
  f_ST_MakeEnvelope.argumentList << QgsWfsCapabilities::Argument( u"minx"_s, u"xs:double"_s );
  f_ST_MakeEnvelope.argumentList << QgsWfsCapabilities::Argument( u"miny"_s, u"xs:double"_s );
  f_ST_MakeEnvelope.argumentList << QgsWfsCapabilities::Argument( u"maxx"_s, u"xs:double"_s );
  f_ST_MakeEnvelope.argumentList << QgsWfsCapabilities::Argument( u"maxy"_s, u"xs:double"_s );
  f_ST_MakeEnvelope.argumentList << QgsWfsCapabilities::Argument( u"srsname"_s, u"xs:string"_s );
  mCaps.functionList << f_ST_MakeEnvelope;

  emit gotCapabilities();
}

QString QgsWfsGetCapabilitiesRequest::NormalizeSRSName( const QString &crsName )
{
  QString authority;
  QString code;
  const QgsOgcCrsUtils::CRSFlavor crsFlavor = QgsOgcCrsUtils::parseCrsName( crsName, authority, code );
  if ( crsFlavor != QgsOgcCrsUtils::CRSFlavor::UNKNOWN )
  {
    return authority + ':' + code;
  }
  return crsName;
}

int QgsWfsGetCapabilitiesRequest::defaultExpirationInSec()
{
  QgsSettings s;
  return s.value( u"qgis/defaultCapabilitiesExpiry"_s, "24" ).toInt() * 60 * 60;
}

void QgsWfsGetCapabilitiesRequest::parseSupportedOperations( const QDomElement &operationsElem, bool &insertCap, bool &updateCap, bool &deleteCap )
{
  insertCap = false;
  updateCap = false;
  deleteCap = false;

  if ( operationsElem.isNull() )
  {
    return;
  }

  QDomNodeList childList = operationsElem.childNodes();
  for ( int i = 0; i < childList.size(); ++i )
  {
    QDomElement elt = childList.at( i ).toElement();
    QString elemName = elt.tagName();
    /* WFS 1.0 */
    if ( elemName == "Insert"_L1 )
    {
      insertCap = true;
    }
    else if ( elemName == "Update"_L1 )
    {
      updateCap = true;
    }
    else if ( elemName == "Delete"_L1 )
    {
      deleteCap = true;
    }
    /* WFS 1.1 */
    else if ( elemName == "Operation"_L1 )
    {
      QString elemText = elt.text();
      if ( elemText == "Insert"_L1 )
      {
        insertCap = true;
      }
      else if ( elemText == "Update"_L1 )
      {
        updateCap = true;
      }
      else if ( elemText == "Delete"_L1 )
      {
        deleteCap = true;
      }
    }
  }
}

static QgsWfsCapabilities::Function getSpatialPredicate( const QString &name )
{
  QgsWfsCapabilities::Function f;
  // WFS 1.0 advertise Intersect, but for conveniency we internally convert it to Intersects
  if ( name == "Intersect"_L1 )
    f.name = u"ST_Intersects"_s;
  else
    f.name = ( name == "BBOX"_L1 ) ? u"BBOX"_s : "ST_" + name;
  f.returnType = u"xs:boolean"_s;
  if ( name == "DWithin"_L1 || name == "Beyond"_L1 )
  {
    f.minArgs = 3;
    f.maxArgs = 3;
    f.argumentList << QgsWfsCapabilities::Argument( u"geometry"_s, u"gml:AbstractGeometryType"_s );
    f.argumentList << QgsWfsCapabilities::Argument( u"geometry"_s, u"gml:AbstractGeometryType"_s );
    f.argumentList << QgsWfsCapabilities::Argument( u"distance"_s );
  }
  else
  {
    f.minArgs = 2;
    f.maxArgs = 2;
    f.argumentList << QgsWfsCapabilities::Argument( u"geometry"_s, u"gml:AbstractGeometryType"_s );
    f.argumentList << QgsWfsCapabilities::Argument( u"geometry"_s, u"gml:AbstractGeometryType"_s );
  }
  return f;
}

void QgsWfsGetCapabilitiesRequest::parseFilterCapabilities( const QDomElement &filterCapabilitiesElem )
{
  // WFS 1.0
  QDomElement spatial_Operators = filterCapabilitiesElem.firstChildElement( u"Spatial_Capabilities"_s ).firstChildElement( u"Spatial_Operators"_s );
  QDomElement spatial_Operator = spatial_Operators.firstChildElement();
  while ( !spatial_Operator.isNull() )
  {
    QString name = spatial_Operator.tagName();
    if ( !name.isEmpty() )
    {
      mCaps.spatialPredicatesList << getSpatialPredicate( name );
    }
    spatial_Operator = spatial_Operator.nextSiblingElement();
  }

  // WFS 1.1 and 2.0
  QDomElement spatialOperators = filterCapabilitiesElem.firstChildElement( u"Spatial_Capabilities"_s ).firstChildElement( u"SpatialOperators"_s );
  QDomElement spatialOperator = spatialOperators.firstChildElement( u"SpatialOperator"_s );
  while ( !spatialOperator.isNull() )
  {
    QString name = spatialOperator.attribute( u"name"_s );
    if ( !name.isEmpty() )
    {
      mCaps.spatialPredicatesList << getSpatialPredicate( name );
    }
    spatialOperator = spatialOperator.nextSiblingElement( u"SpatialOperator"_s );
  }

  // WFS 1.0
  QDomElement function_Names = filterCapabilitiesElem.firstChildElement( u"Scalar_Capabilities"_s )
                                 .firstChildElement( u"Arithmetic_Operators"_s )
                                 .firstChildElement( u"Functions"_s )
                                 .firstChildElement( u"Function_Names"_s );
  QDomElement function_NameElem = function_Names.firstChildElement( u"Function_Name"_s );
  while ( !function_NameElem.isNull() )
  {
    QgsWfsCapabilities::Function f;
    f.name = function_NameElem.text();
    bool ok;
    int nArgs = function_NameElem.attribute( u"nArgs"_s ).toInt( &ok );
    if ( ok )
    {
      if ( nArgs >= 0 )
      {
        f.minArgs = nArgs;
        f.maxArgs = nArgs;
      }
      else
      {
        f.minArgs = -nArgs;
      }
    }
    mCaps.functionList << f;
    function_NameElem = function_NameElem.nextSiblingElement( u"Function_Name"_s );
  }

  // WFS 1.1
  QDomElement functionNames = filterCapabilitiesElem.firstChildElement( u"Scalar_Capabilities"_s )
                                .firstChildElement( u"ArithmeticOperators"_s )
                                .firstChildElement( u"Functions"_s )
                                .firstChildElement( u"FunctionNames"_s );
  QDomElement functionNameElem = functionNames.firstChildElement( u"FunctionName"_s );
  while ( !functionNameElem.isNull() )
  {
    QgsWfsCapabilities::Function f;
    f.name = functionNameElem.text();
    bool ok;
    int nArgs = functionNameElem.attribute( u"nArgs"_s ).toInt( &ok );
    if ( ok )
    {
      if ( nArgs >= 0 )
      {
        f.minArgs = nArgs;
        f.maxArgs = nArgs;
      }
      else
      {
        f.minArgs = -nArgs;
      }
    }
    mCaps.functionList << f;
    functionNameElem = functionNameElem.nextSiblingElement( u"FunctionName"_s );
  }

  QDomElement functions = filterCapabilitiesElem.firstChildElement( u"Functions"_s );
  QDomElement functionElem = functions.firstChildElement( u"Function"_s );
  while ( !functionElem.isNull() )
  {
    QString name = functionElem.attribute( u"name"_s );
    if ( !name.isEmpty() )
    {
      QgsWfsCapabilities::Function f;
      f.name = name;
      QDomElement returnsElem = functionElem.firstChildElement( u"Returns"_s );
      f.returnType = returnsElem.text();
      QDomElement argumentsElem = functionElem.firstChildElement( u"Arguments"_s );
      QDomElement argumentElem = argumentsElem.firstChildElement( u"Argument"_s );
      while ( !argumentElem.isNull() )
      {
        QgsWfsCapabilities::Argument arg;
        arg.name = argumentElem.attribute( u"name"_s );
        arg.type = argumentElem.firstChildElement( u"Type"_s ).text();
        f.argumentList << arg;
        argumentElem = argumentElem.nextSiblingElement( u"Argument"_s );
      }
      f.minArgs = f.argumentList.count();
      f.maxArgs = f.argumentList.count();
      mCaps.functionList << f;
    }
    functionElem = functionElem.nextSiblingElement( u"Function"_s );
  }
}

QString QgsWfsGetCapabilitiesRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of capabilities failed: %1" ).arg( reason );
}
