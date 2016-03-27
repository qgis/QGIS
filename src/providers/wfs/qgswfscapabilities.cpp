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
#include "qgswfscapabilities.h"
#include "qgswfsconstants.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include <QDomDocument>
#include <QSettings>
#include <QStringList>

QgsWFSCapabilities::QgsWFSCapabilities( const QString& theUri )
    : QgsWFSRequest( theUri )
{
  connect( this, SIGNAL( downloadFinished() ), this, SLOT( capabilitiesReplyFinished() ) );
}

QgsWFSCapabilities::~QgsWFSCapabilities()
{
}

bool QgsWFSCapabilities::requestCapabilities( bool synchronous )
{
  QUrl url( baseURL() );
  url.addQueryItem( "REQUEST", "GetCapabilities" );

  const QString& version = mUri.version();
  if ( version == QgsWFSConstants::VERSION_AUTO )
    // MapServer honours the order with the first value being the prefered one
    url.addQueryItem( "ACCEPTVERSIONS", "2.0.0,1.1.0,1.0.0" );
  else
    url.addQueryItem( "VERSION", version );

  if ( !sendGET( url, synchronous, false ) )
  {
    emit gotCapabilities();
    return false;
  }
  return true;
}

QgsWFSCapabilities::Capabilities::Capabilities()
{
  clear();
}

void QgsWFSCapabilities::Capabilities::clear()
{
  maxFeatures = 0;
  supportsHits = false;
  supportsPaging = false;
  version = "";
  featureTypes.clear();
}

void QgsWFSCapabilities::capabilitiesReplyFinished()
{
  const QByteArray& buffer = mResponse;

  QgsDebugMsg( "parsing capabilities: " + buffer );

  // parse XML
  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    mErrorCode = QgsWFSRequest::XmlError;
    mErrorMessage = capabilitiesDocError;
    emit gotCapabilities();
    return;
  }

  QDomElement doc = capabilitiesDocument.documentElement();

  // handle exceptions
  if ( doc.tagName() == "ExceptionReport" )
  {
    QDomNode ex = doc.firstChild();
    QString exc = ex.toElement().attribute( "exceptionCode", "Exception" );
    QDomElement ext = ex.firstChild().toElement();
    mErrorCode = QgsWFSRequest::ServerExceptionError;
    mErrorMessage = exc + ": " + ext.firstChild().nodeValue();
    emit gotCapabilities();
    return;
  }

  mCaps.clear();

  //test wfs version
  mCaps.version = doc.attribute( "version" );
  if ( !mCaps.version.startsWith( "1.0" ) &&
       !mCaps.version.startsWith( "1.1" ) &&
       !mCaps.version.startsWith( "2.0" ) )
  {
    mErrorCode = WFSVersionNotSupported;
    mErrorMessage = tr( "WFS version %1 not supported" ).arg( mCaps.version );
    emit gotCapabilities();
    return;
  }

  // WFS 2.0 implementation are supposed to implement resultType=hits, and some
  // implementations (GeoServer) might advertize it, whereas others (MapServer) do not.
  // WFS 1.1 implementation too I think, but in the examples of the GetCapabilites
  // response of the WFS 1.1 standard (and in common implementations), this is
  // explictly advertized
  if ( mCaps.version.startsWith( "2.0" ) )
    mCaps.supportsHits = true;

  // Note: for conveniency, we do not use the elementsByTagNameNS() method as
  // the WFS and OWS namespaces URI are not the same in all versions

  // find <ows:OperationsMetadata>
  QDomElement operationsMetadataElem = doc.firstChildElement( "OperationsMetadata" );
  if ( !operationsMetadataElem.isNull() )
  {
    QDomNodeList contraintList = operationsMetadataElem.elementsByTagName( "Constraint" );
    for ( int i = 0; i < contraintList.size(); ++i )
    {
      QDomElement contraint = contraintList.at( i ).toElement();
      if ( contraint.attribute( "name" ) == "DefaultMaxFeatures" /* WFS 1.1 */ )
      {
        QDomElement value = contraint.firstChildElement( "Value" );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
        }
      }
      else if ( contraint.attribute( "name" ) == "CountDefault" /* WFS 2.0 (e.g. MapServer) */ )
      {
        QDomElement value = contraint.firstChildElement( "DefaultValue" );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
        }
      }
      else if ( contraint.attribute( "name" ) == "ImplementsResultPaging" /* WFS 2.0 */ )
      {
        QDomElement value = contraint.firstChildElement( "DefaultValue" );
        if ( !value.isNull() && value.text() == "TRUE" )
        {
          mCaps.supportsPaging = true;
          QgsDebugMsg( "Supports paging" );
        }
      }
    }

    // In WFS 2.0, max features can also be set in Operation.GetFeature (e.g. GeoServer)
    // and we are also interested by resultType=hits for WFS 1.1
    QDomNodeList operationList = operationsMetadataElem.elementsByTagName( "Operation" );
    for ( int i = 0; i < operationList.size(); ++i )
    {
      QDomElement operation = operationList.at( i ).toElement();
      if ( operation.attribute( "name" ) == "GetFeature" )
      {
        QDomNodeList operationContraintList = operation.elementsByTagName( "Constraint" );
        for ( int j = 0; j < operationContraintList.size(); ++j )
        {
          QDomElement contraint = operationContraintList.at( j ).toElement();
          if ( contraint.attribute( "name" ) == "CountDefault" )
          {
            QDomElement value = contraint.firstChildElement( "DefaultValue" );
            if ( !value.isNull() )
            {
              mCaps.maxFeatures = value.text().toInt();
              QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
            }
            break;
          }
        }

        QDomNodeList parameterList = operation.elementsByTagName( "Parameter" );
        for ( int j = 0; j < parameterList.size(); ++j )
        {
          QDomElement parameter = parameterList.at( j ).toElement();
          if ( parameter.attribute( "name" ) == "resultType" )
          {
            QDomNodeList valueList = parameter.elementsByTagName( "Value" );
            for ( int k = 0; k < valueList.size(); ++k )
            {
              QDomElement value = valueList.at( k ).toElement();
              if ( value.text() == "hits" )
              {
                mCaps.supportsHits = true;
                QgsDebugMsg( "Support hits" );
                break;
              }
            }
          }
        }

        break;
      }
    }
  }

  //go to <FeatureTypeList>
  QDomElement featureTypeListElem = doc.firstChildElement( "FeatureTypeList" );
  if ( featureTypeListElem.isNull() )
  {
    emit gotCapabilities();
    return;
  }

  // Parse operations supported for all feature types
  bool insertCap, updateCap, deleteCap;
  parseSupportedOperations( featureTypeListElem.firstChildElement( "Operations" ),
                            insertCap,
                            updateCap,
                            deleteCap );

  // get the <FeatureType> elements
  QDomNodeList featureTypeList = featureTypeListElem.elementsByTagName( "FeatureType" );
  for ( int i = 0; i < featureTypeList.size(); ++i )
  {
    FeatureType featureType;
    QDomElement featureTypeElem = featureTypeList.at( i ).toElement();

    //Name
    QDomNodeList nameList = featureTypeElem.elementsByTagName( "Name" );
    if ( nameList.length() > 0 )
    {
      featureType.name = nameList.at( 0 ).toElement().text();
    }
    //Title
    QDomNodeList titleList = featureTypeElem.elementsByTagName( "Title" );
    if ( titleList.length() > 0 )
    {
      featureType.title = titleList.at( 0 ).toElement().text();
    }
    //Abstract
    QDomNodeList abstractList = featureTypeElem.elementsByTagName( "Abstract" );
    if ( abstractList.length() > 0 )
    {
      featureType.abstract = abstractList.at( 0 ).toElement().text();
    }

    //DefaultSRS is always the first entry in the feature srs list
    QDomNodeList defaultCRSList = featureTypeElem.elementsByTagName( "DefaultSRS" );
    if ( defaultCRSList.length() == 0 )
      // In WFS 2.0, this is spelled DefaultCRS...
      defaultCRSList = featureTypeElem.elementsByTagName( "DefaultCRS" );
    if ( defaultCRSList.length() > 0 )
    {
      featureType.crslist.append( NormalizeSRSName( defaultCRSList.at( 0 ).toElement().text() ) );
    }

    //OtherSRS
    QDomNodeList otherCRSList = featureTypeElem.elementsByTagName( "OtherSRS" );
    if ( otherCRSList.length() == 0 )
      // In WFS 2.0, this is spelled OtherCRS...
      otherCRSList = featureTypeElem.elementsByTagName( "OtherCRS" );
    for ( int i = 0; i < otherCRSList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( otherCRSList.at( i ).toElement().text() ) );
    }

    //Support <SRS> for compatibility with older versions
    QDomNodeList srsList = featureTypeElem.elementsByTagName( "SRS" );
    for ( int i = 0; i < srsList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( srsList.at( i ).toElement().text() ) );
    }

    // Get BBox WFS 1.0 way
    QDomElement latLongBB = featureTypeElem.firstChildElement( "LatLongBoundingBox" );
    if ( latLongBB.hasAttributes() )
    {
      featureType.bboxLongLat = QgsRectangle(
                                  latLongBB.attribute( "minx" ).toDouble(),
                                  latLongBB.attribute( "miny" ).toDouble(),
                                  latLongBB.attribute( "maxx" ).toDouble(),
                                  latLongBB.attribute( "maxy" ).toDouble() );
    }
    else
    {
      // WFS 1.1 way
      latLongBB = featureTypeElem.firstChildElement( "WGS84BoundingBox" );
      if ( !latLongBB.isNull() )
      {
        QDomElement lowerCorner = latLongBB.firstChildElement( "LowerCorner" );
        QDomElement upperCorner = latLongBB.firstChildElement( "UpperCorner" );
        if ( !lowerCorner.isNull() && !upperCorner.isNull() )
        {
          QStringList lowerCornerList = lowerCorner.text().split( " ", QString::SkipEmptyParts );
          QStringList upperCornerList = upperCorner.text().split( " ", QString::SkipEmptyParts );
          if ( lowerCornerList.size() == 2 && upperCornerList.size() == 2 )
          {
            featureType.bboxLongLat = QgsRectangle(
                                        lowerCornerList[0].toDouble(),
                                        lowerCornerList[1].toDouble(),
                                        upperCornerList[0].toDouble(),
                                        upperCornerList[1].toDouble() );
          }
        }
      }
    }

    // Parse Operations specific to the type name
    parseSupportedOperations( featureTypeElem.firstChildElement( "Operations" ),
                              featureType.insertCap,
                              featureType.updateCap,
                              featureType.deleteCap );
    featureType.insertCap |= insertCap;
    featureType.updateCap |= updateCap;
    featureType.deleteCap |= deleteCap;

    mCaps.featureTypes.push_back( featureType );
  }

  emit gotCapabilities();
}

QString QgsWFSCapabilities::NormalizeSRSName( QString crsName )
{
  QRegExp re( "urn:ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re.exactMatch( crsName ) )
  {
    return re.cap( 1 ) + ':' + re.cap( 2 );
  }
  // urn:x-ogc:def:crs:EPSG:xxxx as returned by http://maps.warwickshire.gov.uk/gs/ows? in WFS 1.1
  QRegExp re2( "urn:x-ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re2.exactMatch( crsName ) )
  {
    return re2.cap( 1 ) + ':' + re2.cap( 2 );
  }
  return crsName;
}

int QgsWFSCapabilities::defaultExpirationInSec()
{
  QSettings s;
  return s.value( "/qgis/defaultCapabilitiesExpiry", "24" ).toInt() * 60 * 60;
}

void QgsWFSCapabilities::parseSupportedOperations( const QDomElement& operationsElem,
    bool& insertCap,
    bool& updateCap,
    bool& deleteCap )
{
  insertCap = false;
  updateCap = false;
  deleteCap = false;

  // TODO: remove me when WFS-T 1.1 or 2.0 is done
  if ( !mCaps.version.startsWith( "1.0" ) )
    return;

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
    if ( elemName == "Insert" )
    {
      insertCap = true;
    }
    else if ( elemName == "Update" )
    {
      updateCap = true;
    }
    else if ( elemName == "Delete" )
    {
      deleteCap = true;
    }
    /* WFS 1.1 */
    else if ( elemName == "Operation" )
    {
      QString elemText = elt.text();
      if ( elemText == "Insert" )
      {
        insertCap = true;
      }
      else if ( elemText == "Update" )
      {
        updateCap = true;
      }
      else if ( elemText == "Delete" )
      {
        deleteCap = true;
      }
    }
  }
}

QString QgsWFSCapabilities::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of capabilities failed: %1" ).arg( reason );
}
