/***************************************************************************
    qgswmscapabilities.cpp
    ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmscapabilities.h"
#include "qgswmsprovider.h"

#include <QFile>
#include <QDir>
#include <QNetworkCacheMetaData>

#include "qgssettings.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsunittypes.h"
#include "qgsexception.h"
#include "qgsapplication.h"

// %%% copied from qgswmsprovider.cpp
static QString DEFAULT_LATLON_CRS = QStringLiteral( "CRS:84" );



bool QgsWmsSettings::parseUri( const QString &uriString )
{
  QgsDebugMsg( "uriString = " + uriString );
  QgsDataSourceUri uri;
  uri.setEncodedUri( uriString );

  // Setup authentication
  mAuth.mUserName = uri.param( QStringLiteral( "username" ) );
  mAuth.mPassword = uri.param( QStringLiteral( "password" ) );

  if ( uri.hasParam( QStringLiteral( "authcfg" ) ) )
  {
    mAuth.mAuthCfg = uri.param( QStringLiteral( "authcfg" ) );
  }

  mAuth.mReferer = uri.param( QStringLiteral( "referer" ) );
  mXyz = false;  // assume WMS / WMTS

  if ( uri.param( QStringLiteral( "type" ) ) == QLatin1String( "xyz" ) )
  {
    // for XYZ tiles most of the things do not apply
    mTiled = true;
    mXyz = true;
    mTileDimensionValues.clear();
    mTileMatrixSetId = QStringLiteral( "tms0" );
    mMaxWidth = 0;
    mMaxHeight = 0;
    mHttpUri = uri.param( QStringLiteral( "url" ) );
    mBaseUrl = mHttpUri;
    mIgnoreGetMapUrl = false;
    mIgnoreGetFeatureInfoUrl = false;
    mSmoothPixmapTransform = true;
    mDpiMode = DpiNone; // does not matter what we set here
    mActiveSubLayers = QStringList( QStringLiteral( "xyz" ) );  // just a placeholder to have one sub-layer
    mActiveSubStyles = QStringList( QStringLiteral( "xyz" ) );  // just a placeholder to have one sub-style
    mActiveSubLayerVisibility.clear();
    mFeatureCount = 0;
    mImageMimeType.clear();
    mCrsId = QStringLiteral( "EPSG:3857" );
    mEnableContextualLegend = false;
    return true;
  }

  mTiled = false;
  mTileDimensionValues.clear();

  mHttpUri = uri.param( QStringLiteral( "url" ) );
  mBaseUrl = QgsWmsProvider::prepareUri( mHttpUri ); // must set here, setImageCrs is using that
  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  mIgnoreGetMapUrl = uri.hasParam( QStringLiteral( "IgnoreGetMapUrl" ) );
  mIgnoreGetFeatureInfoUrl = uri.hasParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ) );
  mParserSettings.ignoreAxisOrientation = uri.hasParam( QStringLiteral( "IgnoreAxisOrientation" ) ); // must be before parsing!
  mParserSettings.invertAxisOrientation = uri.hasParam( QStringLiteral( "InvertAxisOrientation" ) ); // must be before parsing!
  mSmoothPixmapTransform = uri.hasParam( QStringLiteral( "SmoothPixmapTransform" ) );

  mDpiMode = uri.hasParam( QStringLiteral( "dpiMode" ) ) ? static_cast< QgsWmsDpiMode >( uri.param( QStringLiteral( "dpiMode" ) ).toInt() ) : DpiAll;

  mActiveSubLayers = uri.params( QStringLiteral( "layers" ) );
  mActiveSubStyles = uri.params( QStringLiteral( "styles" ) );
  QgsDebugMsg( "Entering: layers:" + mActiveSubLayers.join( ", " ) + ", styles:" + mActiveSubStyles.join( ", " ) );

  mImageMimeType = uri.param( QStringLiteral( "format" ) );
  QgsDebugMsg( "Setting image encoding to " + mImageMimeType + '.' );

  mMaxWidth = 0;
  mMaxHeight = 0;
  if ( uri.hasParam( QStringLiteral( "maxWidth" ) ) && uri.hasParam( QStringLiteral( "maxHeight" ) ) )
  {
    mMaxWidth = uri.param( QStringLiteral( "maxWidth" ) ).toInt();
    mMaxHeight = uri.param( QStringLiteral( "maxHeight" ) ).toInt();
  }

  mStepWidth = 2000;
  mStepHeight = 2000;
  if ( uri.hasParam( QStringLiteral( "stepWidth" ) ) && uri.hasParam( QStringLiteral( "stepHeight" ) ) )
  {
    mStepWidth = uri.param( QStringLiteral( "stepWidth" ) ).toInt();
    mStepHeight = uri.param( QStringLiteral( "stepHeight" ) ).toInt();
  }

  if ( uri.hasParam( QStringLiteral( "tileMatrixSet" ) ) )
  {
    mTiled = true;
    // tileMatrixSet may be empty if URI was converted from < 1.9 project file URI
    // in that case it means that the source is WMS-C
    mTileMatrixSetId = uri.param( QStringLiteral( "tileMatrixSet" ) );
  }

  if ( uri.hasParam( QStringLiteral( "tileDimensions" ) ) )
  {
    mTiled = true;
    Q_FOREACH ( const QString &param, uri.param( "tileDimensions" ).split( ';' ) )
    {
      QStringList kv = param.split( '=' );
      if ( kv.size() == 1 )
      {
        mTileDimensionValues.insert( kv[0], QString() );
      }
      else if ( kv.size() == 2 )
      {
        mTileDimensionValues.insert( kv[0], kv[1] );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "skipped dimension %1" ).arg( param ) );
      }
    }
  }

  mCrsId = uri.param( QStringLiteral( "crs" ) );

  mEnableContextualLegend = uri.param( QStringLiteral( "contextualWMSLegend" ) ).toInt();
  QgsDebugMsg( QStringLiteral( "Contextual legend: %1" ).arg( mEnableContextualLegend ) );

  mFeatureCount = uri.param( QStringLiteral( "featureCount" ) ).toInt(); // default to 0

  return true;
}


// ----------------------


bool QgsWmsCapabilities::parseResponse( const QByteArray &response, QgsWmsParserSettings settings )
{
  mParserSettings = settings;
  mValid = false;

  if ( response.isEmpty() )
  {
    if ( mError.isEmpty() )
    {
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = QObject::tr( "empty capabilities document" );
    }
    QgsDebugMsg( QStringLiteral( "response is empty" ) );
    return false;
  }

  if ( response.startsWith( "<html>" ) ||
       response.startsWith( "<HTML>" ) )
  {
    mErrorFormat = QStringLiteral( "text/html" );
    mError = response;
    QgsDebugMsg( QStringLiteral( "starts with <html>" ) );
    return false;
  }


  QgsDebugMsg( QStringLiteral( "Converting to Dom." ) );

  bool domOK;
  domOK = parseCapabilitiesDom( response, mCapabilities );

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorCaption and mError are pre-filled by parseCapabilitiesDom

    // TODO[MD] mError += QObject::tr( "\nTried URL: %1" ).arg( url );

    QgsDebugMsg( "!domOK: " + mError );

    return false;
  }

  // get identify formats
  Q_FOREACH ( const QString &f, mCapabilities.capability.request.getFeatureInfo.format )
  {
    // Don't use mSupportedGetFeatureFormats, there are too many possibilities
    QgsDebugMsg( "supported format = " + f );
    // 1.0: MIME - server shall choose format, we presume it to be plain text
    //      GML.1, GML.2, or GML.3
    // 1.1.0, 1.3.0 - mime types, GML should use application/vnd.ogc.gml
    //      but in UMN Mapserver it may be also OUTPUTFORMAT, e.g. OGRGML
    QgsRaster::IdentifyFormat format = QgsRaster::IdentifyFormatUndefined;
    if ( f == QLatin1String( "MIME" ) )
      format = QgsRaster::IdentifyFormatText; // 1.0
    else if ( f == QLatin1String( "text/plain" ) )
      format = QgsRaster::IdentifyFormatText;
    else if ( f == QLatin1String( "text/html" ) )
      format = QgsRaster::IdentifyFormatHtml;
    else if ( f.startsWith( QLatin1String( "GML." ) ) )
      format = QgsRaster::IdentifyFormatFeature; // 1.0
    else if ( f == QLatin1String( "application/vnd.ogc.gml" ) )
      format = QgsRaster::IdentifyFormatFeature;
    else if ( f == QLatin1String( "application/json" ) )
      format = QgsRaster::IdentifyFormatFeature;
    else if ( f.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) )
      format = QgsRaster::IdentifyFormatFeature;

    mIdentifyFormats.insert( format, f );
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );

  mValid = mError.isEmpty();
  return mValid;
}


bool QgsWmsCapabilities::parseCapabilitiesDom( QByteArray const &xml, QgsWmsCapabilitiesProperty &capabilitiesProperty )
{

#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wmsprovider-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  // Convert completed document into a Dom
  QDomDocument capabilitiesDom;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = capabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = QObject::tr( "Could not get WMS capabilities: %1 at line %2 column %3\nThis is probably due to an incorrect WMS Server URL.\nResponse was:\n\n%4" )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElem = capabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WMS Capabilities document)
  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  if (
    docElem.tagName() != QLatin1String( "WMS_Capabilities" )  && // (1.3 vintage)
    docElem.tagName() != QLatin1String( "WMT_MS_Capabilities" ) && // (1.1.1 vintage)
    docElem.tagName() != QLatin1String( "Capabilities" )         // WMTS
  )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = QObject::tr( "Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found.\nThis might be due to an incorrect WMS Server URL.\nTag: %3\nResponse was:\n%4" )
             .arg( QStringLiteral( "WMS_Capabilities" ),
                   QStringLiteral( "WMT_MS_Capabilities" ),
                   docElem.tagName(),
                   QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilitiesProperty.version = docElem.attribute( QStringLiteral( "version" ) );

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      QgsDebugMsg( e.tagName() ); // the node really is an element.

      if ( e.tagName() == QLatin1String( "Service" ) || e.tagName() == QLatin1String( "ows:ServiceProvider" ) || e.tagName() == QLatin1String( "ows:ServiceIdentification" ) )
      {
        QgsDebugMsg( QStringLiteral( "  Service." ) );
        parseService( e, capabilitiesProperty.service );
      }
      else if ( e.tagName() == QLatin1String( "Capability" ) || e.tagName() == QLatin1String( "ows:OperationsMetadata" ) )
      {
        QgsDebugMsg( QStringLiteral( "  Capability." ) );
        parseCapability( e, capabilitiesProperty.capability );
      }
      else if ( e.tagName() == QLatin1String( "Contents" ) )
      {
        QgsDebugMsg( QStringLiteral( "  Contents." ) );
        parseWMTSContents( e );
      }
    }
    n = n.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );

  return true;
}


void QgsWmsCapabilities::parseService( QDomElement const &e, QgsWmsServiceProperty &serviceProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      // QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( QLatin1String( "ows:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Title" ) )
      {
        serviceProperty.title = e1.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        serviceProperty.abstract = e1.text();
      }
      else if ( tagName == QLatin1String( "KeywordList" ) || tagName == QLatin1String( "Keywords" ) )
      {
        parseKeywordList( e1, serviceProperty.keywordList );
      }
      else if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        parseOnlineResource( e1, serviceProperty.onlineResource );
      }
      else if ( tagName == QLatin1String( "ContactInformation" ) || tagName == QLatin1String( "ServiceContact" ) )
      {
        parseContactInformation( e1, serviceProperty.contactInformation );
      }
      else if ( tagName == QLatin1String( "Fees" ) )
      {
        serviceProperty.fees = e1.text();
      }
      else if ( tagName == QLatin1String( "AccessConstraints" ) )
      {
        serviceProperty.accessConstraints = e1.text();
      }
      else if ( tagName == QLatin1String( "LayerLimit" ) )
      {
        serviceProperty.layerLimit = e1.text().toUInt();
      }
      else if ( tagName == QLatin1String( "MaxWidth" ) )
      {
        serviceProperty.maxWidth = e1.text().toUInt();
      }
      else if ( tagName == QLatin1String( "MaxHeight" ) )
      {
        serviceProperty.maxHeight = e1.text().toUInt();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseOnlineResource( QDomElement const &e, QgsWmsOnlineResourceAttribute &onlineResourceAttribute )
{

  onlineResourceAttribute.xlinkHref = QUrl::fromEncoded( e.attribute( QStringLiteral( "xlink:href" ) ).toUtf8() ).toString();

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseKeywordList( QDomElement  const &e, QStringList &keywordListProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( QLatin1String( "ows:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Keyword" ) )
      {
        QgsDebugMsg( QStringLiteral( "      Keyword." ) );
        keywordListProperty += e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseContactInformation( QDomElement const &e, QgsWmsContactInformationProperty &contactInformationProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ContactPersonPrimary" ) )
      {
        parseContactPersonPrimary( e1, contactInformationProperty.contactPersonPrimary );
      }
      else if ( tagName == QLatin1String( "ContactPosition" ) || tagName == QLatin1String( "ows:PositionName" ) )
      {
        contactInformationProperty.contactPosition = e1.text();
      }
      else if ( tagName == QLatin1String( "ContactAddress" ) )
      {
        parseContactAddress( e1, contactInformationProperty.contactAddress );
      }
      else if ( tagName == QLatin1String( "ContactVoiceTelephone" ) )
      {
        contactInformationProperty.contactVoiceTelephone = e1.text();
      }
      else if ( tagName == QLatin1String( "ContactFacsimileTelephone" ) )
      {
        contactInformationProperty.contactFacsimileTelephone = e1.text();
      }
      else if ( tagName == QLatin1String( "ContactElectronicMailAddress" ) )
      {
        contactInformationProperty.contactElectronicMailAddress = e1.text();
      }
      else if ( tagName == QLatin1String( "ows:IndividualName" ) )
      {
        contactInformationProperty.contactPersonPrimary.contactPerson = e1.text();
      }
      else if ( tagName == QLatin1String( "ows:ProviderName" ) )
      {
        contactInformationProperty.contactPersonPrimary.contactOrganization = e1.text();
      }
      else if ( tagName == QLatin1String( "ows:ContactInfo" ) )
      {
        QDomNode n = n1.firstChildElement( QStringLiteral( "ows:Phone" ) );
        contactInformationProperty.contactVoiceTelephone        = n.firstChildElement( QStringLiteral( "ows:Voice" ) ).toElement().text();
        contactInformationProperty.contactFacsimileTelephone    = n.firstChildElement( QStringLiteral( "ows:Facsimile" ) ).toElement().text();

        n = n1.firstChildElement( QStringLiteral( "ows:Address" ) );
        contactInformationProperty.contactElectronicMailAddress   = n.firstChildElement( QStringLiteral( "ows:ElectronicMailAddress" ) ).toElement().text();
        contactInformationProperty.contactAddress.address         = n.firstChildElement( QStringLiteral( "ows:DeliveryPoint" ) ).toElement().text();
        contactInformationProperty.contactAddress.city            = n.firstChildElement( QStringLiteral( "ows:City" ) ).toElement().text();
        contactInformationProperty.contactAddress.stateOrProvince = n.firstChildElement( QStringLiteral( "ows:AdministrativeArea" ) ).toElement().text();
        contactInformationProperty.contactAddress.postCode        = n.firstChildElement( QStringLiteral( "ows:PostalCode" ) ).toElement().text();
        contactInformationProperty.contactAddress.country         = n.firstChildElement( QStringLiteral( "ows:Country" ) ).toElement().text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseContactPersonPrimary( QDomElement const &e, QgsWmsContactPersonPrimaryProperty &contactPersonPrimaryProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ContactPerson" ) )
      {
        contactPersonPrimaryProperty.contactPerson = e1.text();
      }
      else if ( tagName == QLatin1String( "ContactOrganization" ) )
      {
        contactPersonPrimaryProperty.contactOrganization = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseContactAddress( QDomElement const &e, QgsWmsContactAddressProperty &contactAddressProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "AddressType" ) )
      {
        contactAddressProperty.addressType = e1.text();
      }
      else if ( tagName == QLatin1String( "Address" ) )
      {
        contactAddressProperty.address = e1.text();
      }
      else if ( tagName == QLatin1String( "City" ) )
      {
        contactAddressProperty.city = e1.text();
      }
      else if ( tagName == QLatin1String( "StateOrProvince" ) )
      {
        contactAddressProperty.stateOrProvince = e1.text();
      }
      else if ( tagName == QLatin1String( "PostCode" ) )
      {
        contactAddressProperty.postCode = e1.text();
      }
      else if ( tagName == QLatin1String( "Country" ) )
      {
        contactAddressProperty.country = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseCapability( QDomElement const &e, QgsWmsCapabilityProperty &capabilityProperty )
{

  for ( QDomNode n1 = e.firstChild(); !n1.isNull(); n1 = n1.nextSibling() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( e1.isNull() )
      continue;

    QString tagName = e1.tagName();
    if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
      tagName = tagName.mid( 4 );

    QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

    if ( tagName == QLatin1String( "Request" ) )
    {
      parseRequest( e1, capabilityProperty.request );
    }
    else if ( tagName == QLatin1String( "Layer" ) )
    {
      QgsWmsLayerProperty layer;
      parseLayer( e1, layer );
      capabilityProperty.layers.push_back( layer );
    }
    else if ( tagName == QLatin1String( "VendorSpecificCapabilities" ) )
    {
      for ( int i = 0; i < e1.childNodes().size(); i++ )
      {
        QDomNode n2 = e1.childNodes().item( i );
        QDomElement e2 = n2.toElement();

        QString tagName = e2.tagName();
        if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
          tagName = tagName.mid( 4 );

        if ( tagName == QLatin1String( "TileSet" ) )
        {
          parseTileSetProfile( e2 );
        }
      }
    }
    else if ( tagName == QLatin1String( "ows:Operation" ) )
    {
      QString name = e1.attribute( QStringLiteral( "name" ) );
      QDomElement get = n1.firstChildElement( QStringLiteral( "ows:DCP" ) )
                        .firstChildElement( QStringLiteral( "ows:HTTP" ) )
                        .firstChildElement( QStringLiteral( "ows:Get" ) );

      QString href = get.attribute( QStringLiteral( "xlink:href" ) );

      QgsWmsDcpTypeProperty dcp;
      dcp.http.get.onlineResource.xlinkHref = href;

      QgsWmsOperationType *ot = nullptr;
      if ( href.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "http get missing from ows:Operation '%1'" ).arg( name ) );
      }
      else if ( name == QLatin1String( "GetTile" ) )
      {
        ot = &capabilityProperty.request.getTile;
      }
      else if ( name == QLatin1String( "GetFeatureInfo" ) )
      {
        ot = &capabilityProperty.request.getFeatureInfo;
      }
      else if ( name == QLatin1String( "GetLegendGraphic" ) || name == QLatin1String( "sld:GetLegendGraphic" ) )
      {
        ot = &capabilityProperty.request.getLegendGraphic;
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "ows:Operation %1 ignored" ).arg( name ) );
      }

      if ( ot )
      {
        ot->dcpType << dcp;
        ot->allowedEncodings.clear();
        for ( QDomElement e2 = get.firstChildElement( QStringLiteral( "ows:Constraint" ) ).firstChildElement( QStringLiteral( "ows:AllowedValues" ) ).firstChildElement( QStringLiteral( "ows:Value" ) );
              !e2.isNull();
              e2 = e1.nextSiblingElement( QStringLiteral( "ows:Value" ) ) )
        {
          ot->allowedEncodings << e2.text();
        }
      }
    }
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseRequest( QDomElement const &e, QgsWmsRequestProperty &requestProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString operation = e1.tagName();
      if ( operation == QLatin1String( "Operation" ) )
      {
        operation = e1.attribute( QStringLiteral( "name" ) );
      }

      if ( operation == QLatin1String( "GetMap" ) )
      {
        QgsDebugMsg( QStringLiteral( "      GetMap." ) );
        parseOperationType( e1, requestProperty.getMap );
      }
      else if ( operation == QLatin1String( "GetFeatureInfo" ) )
      {
        QgsDebugMsg( QStringLiteral( "      GetFeatureInfo." ) );
        parseOperationType( e1, requestProperty.getFeatureInfo );
      }
      else if ( operation == QLatin1String( "GetLegendGraphic" ) || operation == QLatin1String( "sld:GetLegendGraphic" ) )
      {
        QgsDebugMsg( QStringLiteral( "      GetLegendGraphic." ) );
        parseOperationType( e1, requestProperty.getLegendGraphic );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}



void QgsWmsCapabilities::parseLegendUrl( QDomElement const &e, QgsWmsLegendUrlProperty &legendUrlProperty )
{

  legendUrlProperty.width  = e.attribute( QStringLiteral( "width" ) ).toUInt();
  legendUrlProperty.height = e.attribute( QStringLiteral( "height" ) ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Format" ) )
      {
        legendUrlProperty.format = e1.text();
      }
      else if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        parseOnlineResource( e1, legendUrlProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseLayer( QDomElement const &e, QgsWmsLayerProperty &layerProperty,
                                     QgsWmsLayerProperty *parentProperty )
{

// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
#if 0
  // enforce WMS non-inheritance rules
  layerProperty.name = QString();
  layerProperty.title = QString();
  layerProperty.abstract = QString();
  layerProperty.keywordList.clear();
#endif

  layerProperty.orderId     = ++mLayerCount;
  layerProperty.queryable   = e.attribute( QStringLiteral( "queryable" ) ).toUInt();
  layerProperty.cascaded    = e.attribute( QStringLiteral( "cascaded" ) ).toUInt();
  layerProperty.opaque      = e.attribute( QStringLiteral( "opaque" ) ).toUInt();
  layerProperty.noSubsets   = e.attribute( QStringLiteral( "noSubsets" ) ).toUInt();
  layerProperty.fixedWidth  = e.attribute( QStringLiteral( "fixedWidth" ) ).toUInt();
  layerProperty.fixedHeight = e.attribute( QStringLiteral( "fixedHeight" ) ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      //QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Layer" ) )
      {
        //QgsDebugMsg( QStringLiteral( "      Nested layer." ) );

        QgsWmsLayerProperty subLayerProperty;

        // Inherit things into the sublayer
        //   Ref: 7.2.4.8 Inheritance of layer properties
        subLayerProperty.style                    = layerProperty.style;
        subLayerProperty.crs                      = layerProperty.crs;
        subLayerProperty.boundingBoxes            = layerProperty.boundingBoxes;
        subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
        // TODO

        parseLayer( e1, subLayerProperty, &layerProperty );

        layerProperty.layer.push_back( subLayerProperty );
      }
      else if ( tagName == QLatin1String( "Name" ) )
      {
        layerProperty.name = e1.text();
      }
      else if ( tagName == QLatin1String( "Title" ) )
      {
        layerProperty.title = e1.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        layerProperty.abstract = e1.text();
      }
      else if ( tagName == QLatin1String( "KeywordList" ) )
      {
        parseKeywordList( e1, layerProperty.keywordList );
      }
      else if ( tagName == QLatin1String( "SRS" ) || tagName == QLatin1String( "CRS" ) )
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        Q_FOREACH ( const QString &srs, e1.text().split( QRegExp( "\\s+" ) ) )
        {
          layerProperty.crs.push_back( srs );
        }
      }
      else if ( tagName == QLatin1String( "LatLonBoundingBox" ) )    // legacy from earlier versions of WMS
      {
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              e1.attribute( QStringLiteral( "minx" ) ).toDouble(),
              e1.attribute( QStringLiteral( "miny" ) ).toDouble(),
              e1.attribute( QStringLiteral( "maxx" ) ).toDouble(),
              e1.attribute( QStringLiteral( "maxy" ) ).toDouble()
            );

        if ( e1.hasAttribute( QStringLiteral( "SRS" ) ) && e1.attribute( QStringLiteral( "SRS" ) ) != DEFAULT_LATLON_CRS )
        {
          try
          {
            QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( e1.attribute( QStringLiteral( "SRS" ) ) );

            QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromOgcWmsCrs( DEFAULT_LATLON_CRS );

            Q_NOWARN_DEPRECATED_PUSH
            QgsCoordinateTransform ct( src, dst );
            Q_NOWARN_DEPRECATED_POP
            layerProperty.ex_GeographicBoundingBox = ct.transformBoundingBox( layerProperty.ex_GeographicBoundingBox );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse );
          }
        }
      }
      else if ( tagName == QLatin1String( "EX_GeographicBoundingBox" ) ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        if ( e1.tagName() == QLatin1String( "wms:EX_GeographicBoundingBox" ) )
        {
          wBoundLongitudeElem = n1.namedItem( QStringLiteral( "wms:westBoundLongitude" ) ).toElement();
          eBoundLongitudeElem = n1.namedItem( QStringLiteral( "wms:eastBoundLongitude" ) ).toElement();
          sBoundLatitudeElem = n1.namedItem( QStringLiteral( "wms:southBoundLatitude" ) ).toElement();
          nBoundLatitudeElem = n1.namedItem( QStringLiteral( "wms:northBoundLatitude" ) ).toElement();
        }
        else
        {
          wBoundLongitudeElem = n1.namedItem( QStringLiteral( "westBoundLongitude" ) ).toElement();
          eBoundLongitudeElem = n1.namedItem( QStringLiteral( "eastBoundLongitude" ) ).toElement();
          sBoundLatitudeElem = n1.namedItem( QStringLiteral( "southBoundLatitude" ) ).toElement();
          nBoundLatitudeElem = n1.namedItem( QStringLiteral( "northBoundLatitude" ) ).toElement();
        }

        double wBLong, eBLong, sBLat, nBLat;
        bool wBOk, eBOk, sBOk, nBOk;
        wBLong = wBoundLongitudeElem.text().toDouble( &wBOk );
        eBLong = eBoundLongitudeElem.text().toDouble( &eBOk );
        sBLat = sBoundLatitudeElem.text().toDouble( &sBOk );
        nBLat = nBoundLatitudeElem.text().toDouble( &nBOk );
        if ( wBOk && eBOk && sBOk && nBOk )
        {
          layerProperty.ex_GeographicBoundingBox = QgsRectangle( wBLong, sBLat, eBLong, nBLat );
        }
      }
      else if ( tagName == QLatin1String( "BoundingBox" ) )
      {
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( e1.attribute( QStringLiteral( "minx" ) ).toDouble(),
                                 e1.attribute( QStringLiteral( "miny" ) ).toDouble(),
                                 e1.attribute( QStringLiteral( "maxx" ) ).toDouble(),
                                 e1.attribute( QStringLiteral( "maxy" ) ).toDouble()
                               );
        if ( e1.hasAttribute( QStringLiteral( "CRS" ) ) || e1.hasAttribute( QStringLiteral( "SRS" ) ) )
        {
          if ( e1.hasAttribute( QStringLiteral( "CRS" ) ) )
            bbox.crs = e1.attribute( QStringLiteral( "CRS" ) );
          else if ( e1.hasAttribute( QStringLiteral( "SRS" ) ) )
            bbox.crs = e1.attribute( QStringLiteral( "SRS" ) );

          if ( shouldInvertAxisOrientation( bbox.crs ) )
          {
            QgsRectangle invAxisBbox( bbox.box.yMinimum(), bbox.box.xMinimum(),
                                      bbox.box.yMaximum(), bbox.box.xMaximum() );
            bbox.box = invAxisBbox;
          }

          // Overwrite existing bounding boxes with identical CRS
          bool inheritedOverwritten = false;
          for ( int i = 0; i < layerProperty.boundingBoxes.size(); i++ )
          {
            if ( layerProperty.boundingBoxes[i].crs == bbox.crs )
            {
              layerProperty.boundingBoxes[i] = bbox;
              inheritedOverwritten = true;
            }
          }
          if ( ! inheritedOverwritten )
            layerProperty.boundingBoxes << bbox;
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "CRS/SRS attribute not found in BoundingBox" ) );
        }
      }
      else if ( tagName == QLatin1String( "Dimension" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "Attribution" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "AuthorityURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "Identifier" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "MetadataURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "DataURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "FeatureListURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "Style" ) )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( e1, styleProperty );

        for ( int i = 0; i < layerProperty.style.size(); ++i )
        {
          if ( layerProperty.style.at( i ).name == styleProperty.name )
          {
            // override inherited parent's style if it has the same name
            // according to the WMS spec, it should not happen, but Mapserver
            // does it all the time.
            layerProperty.style.remove( i );
            break;
          }
        }
        layerProperty.style.push_back( styleProperty );
      }
      else if ( tagName == QLatin1String( "MinScaleDenominator" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "MaxScaleDenominator" ) )
      {
        // TODO
      }
      // If we got here then it's not in the WMS 1.3 standard

    }
    n1 = n1.nextSibling();
  }

  if ( parentProperty )
  {
    mLayerParents[ layerProperty.orderId ] = parentProperty->orderId;
  }

  if ( !layerProperty.name.isEmpty() )
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere

    // Store if the layer is queryable
    mQueryableForLayer[ layerProperty.name ] = layerProperty.queryable;

    // Insert into the local class' registry
    mLayersSupported.push_back( layerProperty );

    //if there are several <Layer> elements without a parent layer, the style list needs to be cleared
    if ( layerProperty.layer.empty() )
    {
      layerProperty.style.clear();
    }
  }

  if ( !layerProperty.layer.empty() )
  {
    mLayerParentNames[ layerProperty.orderId ] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
  }

  //QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseStyle( QDomElement const &e, QgsWmsStyleProperty &styleProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Name" ) )
      {
        styleProperty.name = e1.text();
      }
      else if ( tagName == QLatin1String( "Title" ) )
      {
        styleProperty.title = e1.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        styleProperty.abstract = e1.text();
      }
      else if ( tagName == QLatin1String( "LegendURL" ) )
      {
        styleProperty.legendUrl << QgsWmsLegendUrlProperty();
        parseLegendUrl( e1, styleProperty.legendUrl.last() );
      }
      else if ( tagName == QLatin1String( "StyleSheetURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "StyleURL" ) )
      {
        // TODO
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseOperationType( QDomElement const &e, QgsWmsOperationType &operationType )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Format" ) )
      {
        QgsDebugMsg( QStringLiteral( "      Format." ) );
        operationType.format += e1.text();
      }
      else if ( tagName == QLatin1String( "DCPType" ) )
      {
        QgsDebugMsg( QStringLiteral( "      DCPType." ) );
        QgsWmsDcpTypeProperty dcp;
        parseDcpType( e1, dcp );
        operationType.dcpType.push_back( dcp );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}


void QgsWmsCapabilities::parseDcpType( QDomElement const &e, QgsWmsDcpTypeProperty &dcpType )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == QLatin1String( "HTTP" ) )
      {
        QgsDebugMsg( QStringLiteral( "      HTTP." ) );
        parseHttp( e1, dcpType.http );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseHttp( QDomElement const &e, QgsWmsHttpProperty &httpProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Get" ) )
      {
        QgsDebugMsg( QStringLiteral( "      Get." ) );
        parseGet( e1, httpProperty.get );
      }
      else if ( tagName == QLatin1String( "Post" ) )
      {
        QgsDebugMsg( QStringLiteral( "      Post." ) );
        parsePost( e1, httpProperty.post );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseGet( QDomElement const &e, QgsWmsGetProperty &getProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        QgsDebugMsg( QStringLiteral( "      OnlineResource." ) );
        parseOnlineResource( e1, getProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parsePost( QDomElement const &e, QgsWmsPostProperty &postProperty )
{

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        QgsDebugMsg( QStringLiteral( "      OnlineResource." ) );
        parseOnlineResource( e1, postProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( QStringLiteral( "exiting." ) );
}

void QgsWmsCapabilities::parseTileSetProfile( QDomElement const &e )
{
  QStringList resolutions, layers, styles;
  QgsWmsBoundingBoxProperty boundingBox;
  QgsWmtsTileMatrixSet ms;
  QgsWmtsTileMatrix m;
  QgsWmtsTileLayer l;

  l.tileMode = WMSC;

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      QString tagName = e1.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Layers" ) )
      {
        layers << e1.text();
      }
      else if ( tagName == QLatin1String( "Styles" ) )
      {
        styles << e1.text();
      }
      else if ( tagName == QLatin1String( "Width" ) )
      {
        m.tileWidth = e1.text().toInt();
      }
      else if ( tagName == QLatin1String( "Height" ) )
      {
        m.tileHeight = e1.text().toInt();
      }
      else if ( tagName == QLatin1String( "SRS" ) )
      {
        ms.crs = e1.text();
      }
      else if ( tagName == QLatin1String( "Format" ) )
      {
        l.formats << e1.text();
      }
      else if ( tagName == QLatin1String( "BoundingBox" ) )
      {
        QgsWmsBoundingBoxProperty bb;
        bb.box = QgsRectangle(
                   e1.attribute( QStringLiteral( "minx" ) ).toDouble(),
                   e1.attribute( QStringLiteral( "miny" ) ).toDouble(),
                   e1.attribute( QStringLiteral( "maxx" ) ).toDouble(),
                   e1.attribute( QStringLiteral( "maxy" ) ).toDouble()
                 );
        if ( e1.hasAttribute( QStringLiteral( "SRS" ) ) )
          bb.crs = e1.attribute( QStringLiteral( "SRS" ) );
        else if ( e1.hasAttribute( QStringLiteral( "srs" ) ) )
          bb.crs = e1.attribute( QStringLiteral( "srs" ) );
        else if ( e1.hasAttribute( QStringLiteral( "CRS" ) ) )
          bb.crs = e1.attribute( QStringLiteral( "CRS" ) );
        else if ( e1.hasAttribute( QStringLiteral( "crs" ) ) )
          bb.crs = e1.attribute( QStringLiteral( "crs" ) );
        else
        {
          QgsDebugMsg( QStringLiteral( "crs of bounding box undefined" ) );
        }

        if ( !bb.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( bb.crs );
          if ( crs.isValid() )
            bb.crs = crs.authid();

          l.boundingBoxes << bb;
        }
      }
      else if ( tagName == QLatin1String( "Resolutions" ) )
      {
        resolutions = e1.text().trimmed().split( ' ', QString::SkipEmptyParts );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "tileset tag %1 ignored" ).arg( e1.tagName() ) );
      }
    }
    n1 = n1.nextSibling();
  }

  ms.identifier = QStringLiteral( "%1-wmsc-%2" ).arg( layers.join( QStringLiteral( "_" ) ) ).arg( mTileLayersSupported.size() );

  l.identifier = layers.join( QStringLiteral( "," ) );
  QgsWmtsStyle s;
  s.identifier = styles.join( QStringLiteral( "," ) );
  l.styles.insert( s.identifier, s );
  l.defaultStyle = s.identifier;

  QgsWmtsTileMatrixSetLink sl;
  sl.tileMatrixSet = ms.identifier;
  l.setLinks.insert( ms.identifier, sl );
  mTileLayersSupported.append( l );

  int i = 0;
  Q_FOREACH ( const QString &rS, resolutions )
  {
    double r = rS.toDouble();
    m.identifier = QString::number( i );
    Q_ASSERT( l.boundingBoxes.size() == 1 );
    m.matrixWidth  = std::ceil( l.boundingBoxes.at( 0 ).box.width() / m.tileWidth / r );
    m.matrixHeight = std::ceil( l.boundingBoxes.at( 0 ).box.height() / m.tileHeight / r );
    m.topLeft = QgsPointXY( l.boundingBoxes.at( 0 ).box.xMinimum(), l.boundingBoxes.at( 0 ).box.yMinimum() + m.matrixHeight * m.tileHeight * r );
    m.tres = r;
    ms.tileMatrices.insert( r, m );
    i++;
  }

  mTileMatrixSets.insert( ms.identifier, ms );
}

void QgsWmsCapabilities::parseWMTSContents( QDomElement const &e )
{
  //
  // tile matrix sets
  //

  mTileMatrixSets.clear();
  for ( QDomElement e0 = e.firstChildElement( QStringLiteral( "TileMatrixSet" ) );
        !e0.isNull();
        e0 = e0.nextSiblingElement( QStringLiteral( "TileMatrixSet" ) ) )
  {
    QgsWmtsTileMatrixSet s;
    s.identifier = e0.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
    s.title      = e0.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
    s.abstract   = e0.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
    parseKeywords( e0, s.keywords );

    QString supportedCRS = e0.firstChildElement( QStringLiteral( "ows:SupportedCRS" ) ).text();

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( supportedCRS );

    s.wkScaleSet = e0.firstChildElement( QStringLiteral( "WellKnownScaleSet" ) ).text();

    double metersPerUnit = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters );

    s.crs = crs.authid();

    bool invert = !mParserSettings.ignoreAxisOrientation && crs.hasAxisInverted();
    if ( mParserSettings.invertAxisOrientation )
      invert = !invert;

    QgsDebugMsg( QStringLiteral( "tilematrix set: %1 (supportedCRS:%2 crs:%3; metersPerUnit:%4 axisInverted:%5)" )
                 .arg( s.identifier,
                       supportedCRS,
                       s.crs )
                 .arg( metersPerUnit, 0, 'f' )
                 .arg( invert ? "yes" : "no" )
               );

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "TileMatrix" ) );
          !e1.isNull();
          e1 = e1.nextSiblingElement( QStringLiteral( "TileMatrix" ) ) )
    {
      QgsWmtsTileMatrix m;

      m.identifier = e1.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      m.title      = e1.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      m.abstract   = e1.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( e1, m.keywords );

      m.scaleDenom = e1.firstChildElement( QStringLiteral( "ScaleDenominator" ) ).text().toDouble();

      QStringList topLeft = e1.firstChildElement( QStringLiteral( "TopLeftCorner" ) ).text().split( ' ' );
      if ( topLeft.size() == 2 )
      {
        if ( invert )
        {
          m.topLeft.set( topLeft[1].toDouble(), topLeft[0].toDouble() );
        }
        else
        {
          m.topLeft.set( topLeft[0].toDouble(), topLeft[1].toDouble() );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Could not parse topLeft" ) );
        continue;
      }

      m.tileWidth    = e1.firstChildElement( QStringLiteral( "TileWidth" ) ).text().toInt();
      m.tileHeight   = e1.firstChildElement( QStringLiteral( "TileHeight" ) ).text().toInt();
      m.matrixWidth  = e1.firstChildElement( QStringLiteral( "MatrixWidth" ) ).text().toInt();
      m.matrixHeight = e1.firstChildElement( QStringLiteral( "MatrixHeight" ) ).text().toInt();

      // the magic number below is "standardized rendering pixel size" defined
      // in WMTS (and WMS 1.3) standard, being 0.28 pixel
      m.tres = m.scaleDenom * 0.00028 / metersPerUnit;

      QgsDebugMsg( QStringLiteral( " %1: scale=%2 res=%3 tile=%4x%5 matrix=%6x%7 topLeft=%8" )
                   .arg( m.identifier )
                   .arg( m.scaleDenom ).arg( m.tres )
                   .arg( m.tileWidth ).arg( m.tileHeight )
                   .arg( m.matrixWidth ).arg( m.matrixHeight )
                   .arg( m.topLeft.toString() )
                 );

      s.tileMatrices.insert( m.tres, m );
    }

    mTileMatrixSets.insert( s.identifier, s );
  }

  //
  // layers
  //

  mTileLayersSupported.clear();
  for ( QDomElement e0 = e.firstChildElement( QStringLiteral( "Layer" ) );
        !e0.isNull();
        e0 = e0.nextSiblingElement( QStringLiteral( "Layer" ) ) )
  {
#ifdef QGISDEBUG
    QString id = e0.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();  // clazy:exclude=unused-non-trivial-variable
    QgsDebugMsg( QStringLiteral( "Layer %1" ).arg( id ) );
#endif

    QgsWmtsTileLayer l;
    l.tileMode   = WMTS;
    l.identifier = e0.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
    l.title      = e0.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
    l.abstract   = e0.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
    parseKeywords( e0, l.keywords );

    QgsWmsBoundingBoxProperty bb;

    QDomElement bbox = e0.firstChildElement( QStringLiteral( "ows:WGS84BoundingBox" ) );
    if ( !bbox.isNull() )
    {
      QStringList ll = bbox.firstChildElement( QStringLiteral( "ows:LowerCorner" ) ).text().split( ' ' );
      QStringList ur = bbox.firstChildElement( QStringLiteral( "ows:UpperCorner" ) ).text().split( ' ' );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        bb.crs = DEFAULT_LATLON_CRS;
        bb.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ),
                               QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        l.boundingBoxes << bb;
      }
    }

    for ( bbox = e0.firstChildElement( QStringLiteral( "ows:BoundingBox" ) );
          !bbox.isNull();
          bbox = bbox.nextSiblingElement( QStringLiteral( "ows:BoundingBox" ) ) )
    {
      QStringList ll = bbox.firstChildElement( QStringLiteral( "ows:LowerCorner" ) ).text().split( ' ' );
      QStringList ur = bbox.firstChildElement( QStringLiteral( "ows:UpperCorner" ) ).text().split( ' ' );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        bb.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ),
                               QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        if ( bbox.hasAttribute( QStringLiteral( "SRS" ) ) )
          bb.crs = bbox.attribute( QStringLiteral( "SRS" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "srs" ) ) )
          bb.crs = bbox.attribute( QStringLiteral( "srs" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "CRS" ) ) )
          bb.crs = bbox.attribute( QStringLiteral( "CRS" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "crs" ) ) )
          bb.crs = bbox.attribute( QStringLiteral( "crs" ) );
        else
        {
          QgsDebugMsg( QStringLiteral( "crs of bounding box undefined" ) );
        }

        if ( !bb.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( bb.crs );
          if ( crs.isValid() )
          {
            bb.crs = crs.authid();

            bool invert = !mParserSettings.ignoreAxisOrientation && crs.hasAxisInverted();
            if ( mParserSettings.invertAxisOrientation )
              invert = !invert;

            if ( invert )
              bb.box.invert();

            l.boundingBoxes << bb;
          }
        }
      }
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "Style" ) );
          !e1.isNull();
          e1 = e1.nextSiblingElement( QStringLiteral( "Style" ) ) )
    {
      QgsWmtsStyle s;
      s.identifier = e1.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      s.title      = e1.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      s.abstract   = e1.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( e1, s.keywords );

      for ( QDomElement e2 = e1.firstChildElement( QStringLiteral( "ows:legendURL" ) );
            !e2.isNull();
            e2 = e2.nextSiblingElement( QStringLiteral( "ows:legendURL" ) ) )
      {
        QgsWmtsLegendURL u;

        u.format   = e2.firstChildElement( QStringLiteral( "format" ) ).text();
        u.minScale = e2.firstChildElement( QStringLiteral( "minScale" ) ).text().toDouble();
        u.maxScale = e2.firstChildElement( QStringLiteral( "maxScale" ) ).text().toDouble();
        u.href     = e2.firstChildElement( QStringLiteral( "href" ) ).text();
        u.width    = e2.firstChildElement( QStringLiteral( "width" ) ).text().toInt();
        u.height   = e2.firstChildElement( QStringLiteral( "height" ) ).text().toInt();

        s.legendURLs << u;
      }

      s.isDefault = e1.attribute( QStringLiteral( "isDefault" ) ) == QLatin1String( "true" );

      l.styles.insert( s.identifier, s );

      if ( s.isDefault )
        l.defaultStyle = s.identifier;
    }

    if ( l.styles.isEmpty() )
    {
      QgsWmtsStyle s;
      s.identifier = QStringLiteral( "default" );
      s.title      = QObject::tr( "Generated default style" );
      s.abstract   = QObject::tr( "Style was missing in capabilities" );
      l.styles.insert( s.identifier, s );
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "Format" ) ); !e1.isNull(); e1 = e1.nextSiblingElement( QStringLiteral( "Format" ) ) )
    {
      l.formats << e1.text();
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "InfoFormat" ) ); !e1.isNull(); e1 = e1.nextSiblingElement( QStringLiteral( "InfoFormat" ) ) )
    {
      QString format = e1.text();

      l.infoFormats << e1.text();

      QgsRaster::IdentifyFormat fmt = QgsRaster::IdentifyFormatUndefined;

      QgsDebugMsg( QStringLiteral( "format=%1" ).arg( format ) );

      if ( format == QLatin1String( "MIME" ) )
        fmt = QgsRaster::IdentifyFormatText; // 1.0
      else if ( format == QLatin1String( "text/plain" ) )
        fmt = QgsRaster::IdentifyFormatText;
      else if ( format == QLatin1String( "text/html" ) )
        fmt = QgsRaster::IdentifyFormatHtml;
      else if ( format.startsWith( QLatin1String( "GML." ) ) )
        fmt = QgsRaster::IdentifyFormatFeature; // 1.0
      else if ( format == QLatin1String( "application/vnd.ogc.gml" ) )
        fmt = QgsRaster::IdentifyFormatFeature;
      else  if ( format.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) )
        fmt = QgsRaster::IdentifyFormatFeature;
      else if ( format == QLatin1String( "application/json" ) )
        fmt = QgsRaster::IdentifyFormatFeature;
      else
      {
        QgsDebugMsg( QStringLiteral( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
        continue;
      }

      QgsDebugMsg( QStringLiteral( "fmt=%1" ).arg( fmt ) );
      mIdentifyFormats.insert( fmt, format );
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "Dimension" ) ); !e1.isNull(); e1 = e1.nextSiblingElement( QStringLiteral( "Dimension" ) ) )
    {
      QgsWmtsDimension d;

      d.identifier   = e1.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      if ( d.identifier.isEmpty() )
        continue;

      d.title        = e1.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      d.abstract     = e1.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( e1, d.keywords );

      d.UOM          = e1.firstChildElement( QStringLiteral( "UOM" ) ).text();
      d.unitSymbol   = e1.firstChildElement( QStringLiteral( "unitSymbol" ) ).text();
      d.defaultValue = e1.firstChildElement( QStringLiteral( "Default" ) ).text();
      d.current      = e1.firstChildElement( QStringLiteral( "current" ) ).text() == QLatin1String( "true" );

      for ( QDomElement e2 = e1.firstChildElement( QStringLiteral( "Value" ) );
            !e2.isNull();
            e2 = e2.nextSiblingElement( QStringLiteral( "Value" ) ) )
      {
        d.values << e2.text();
      }

      l.dimensions.insert( d.identifier, d );
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "TileMatrixSetLink" ) ); !e1.isNull(); e1 = e1.nextSiblingElement( QStringLiteral( "TileMatrixSetLink" ) ) )
    {
      QgsWmtsTileMatrixSetLink sl;

      sl.tileMatrixSet = e1.firstChildElement( QStringLiteral( "TileMatrixSet" ) ).text();

      if ( !mTileMatrixSets.contains( sl.tileMatrixSet ) )
      {
        QgsDebugMsg( QStringLiteral( "  TileMatrixSet %1 not found." ).arg( sl.tileMatrixSet ) );
        continue;
      }

      const QgsWmtsTileMatrixSet &tms = mTileMatrixSets[ sl.tileMatrixSet ];

      for ( QDomElement e2 = e1.firstChildElement( QStringLiteral( "TileMatrixSetLimits" ) ); !e2.isNull(); e2 = e2.nextSiblingElement( QStringLiteral( "TileMatrixSetLimits" ) ) )
      {
        for ( QDomElement e3 = e2.firstChildElement( QStringLiteral( "TileMatrixLimits" ) ); !e3.isNull(); e3 = e3.nextSiblingElement( QStringLiteral( "TileMatrixLimits" ) ) )
        {
          QgsWmtsTileMatrixLimits limit;

          QString id = e3.firstChildElement( QStringLiteral( "TileMatrix" ) ).text();

          bool isValid = false;
          int matrixWidth = -1, matrixHeight = -1;
          Q_FOREACH ( const QgsWmtsTileMatrix &m, tms.tileMatrices )
          {
            isValid = m.identifier == id;
            if ( isValid )
            {
              matrixWidth = m.matrixWidth;
              matrixHeight = m.matrixHeight;
              break;
            }
          }

          if ( isValid )
          {
            limit.minTileRow = e3.firstChildElement( QStringLiteral( "MinTileRow" ) ).text().toInt();
            limit.maxTileRow = e3.firstChildElement( QStringLiteral( "MaxTileRow" ) ).text().toInt();
            limit.minTileCol = e3.firstChildElement( QStringLiteral( "MinTileCol" ) ).text().toInt();
            limit.maxTileCol = e3.firstChildElement( QStringLiteral( "MaxTileCol" ) ).text().toInt();

            isValid =
              limit.minTileCol >= 0 && limit.minTileCol < matrixWidth &&
              limit.maxTileCol >= 0 && limit.maxTileCol < matrixWidth &&
              limit.minTileCol <= limit.maxTileCol &&
              limit.minTileRow >= 0 && limit.minTileRow < matrixHeight &&
              limit.maxTileRow >= 0 && limit.maxTileRow < matrixHeight &&
              limit.minTileRow <= limit.maxTileRow;
          }
          else
          {
            QgsDebugMsg( QStringLiteral( "   TileMatrix id:%1 not found." ).arg( id ) );
          }

          QgsDebugMsg( QStringLiteral( "   TileMatrixLimit id:%1 row:%2-%3 col:%4-%5 matrix:%6x%7 %8" )
                       .arg( id )
                       .arg( limit.minTileRow ).arg( limit.maxTileRow )
                       .arg( limit.minTileCol ).arg( limit.maxTileCol )
                       .arg( matrixWidth ).arg( matrixHeight )
                       .arg( isValid ? "valid" : "INVALID" )
                     );

          if ( isValid )
          {
            sl.limits.insert( id, limit );
          }
        }
      }

      l.setLinks.insert( sl.tileMatrixSet, sl );
    }

    for ( QDomElement e1 = e0.firstChildElement( QStringLiteral( "ResourceURL" ) ); !e1.isNull(); e1 = e1.nextSiblingElement( QStringLiteral( "ResourceURL" ) ) )
    {
      QString format       = nodeAttribute( e1, QStringLiteral( "format" ) );
      QString resourceType = nodeAttribute( e1, QStringLiteral( "resourceType" ) );
      QString tmpl         = nodeAttribute( e1, QStringLiteral( "template" ) );

      if ( format.isEmpty() || resourceType.isEmpty() || tmpl.isEmpty() )
      {
        QgsDebugMsg( QStringLiteral( "SKIPPING ResourceURL format=%1 resourceType=%2 template=%3" )
                     .arg( format,
                           resourceType,
                           tmpl ) );
        continue;
      }

      if ( resourceType == QLatin1String( "tile" ) )
      {
        l.getTileURLs.insert( format, tmpl );
      }
      else if ( resourceType == QLatin1String( "FeatureInfo" ) )
      {
        l.getFeatureInfoURLs.insert( format, tmpl );

        QgsRaster::IdentifyFormat fmt = QgsRaster::IdentifyFormatUndefined;

        QgsDebugMsg( QStringLiteral( "format=%1" ).arg( format ) );

        if ( format == QLatin1String( "MIME" ) )
          fmt = QgsRaster::IdentifyFormatText; // 1.0
        else if ( format == QLatin1String( "text/plain" ) )
          fmt = QgsRaster::IdentifyFormatText;
        else if ( format == QLatin1String( "text/html" ) )
          fmt = QgsRaster::IdentifyFormatHtml;
        else if ( format.startsWith( QLatin1String( "GML." ) ) )
          fmt = QgsRaster::IdentifyFormatFeature; // 1.0
        else if ( format == QLatin1String( "application/vnd.ogc.gml" ) )
          fmt = QgsRaster::IdentifyFormatFeature;
        else  if ( format.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) )
          fmt = QgsRaster::IdentifyFormatFeature;
        else if ( format == QLatin1String( "application/json" ) )
          fmt = QgsRaster::IdentifyFormatFeature;
        else
        {
          QgsDebugMsg( QStringLiteral( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
          continue;
        }

        QgsDebugMsg( QStringLiteral( "fmt=%1" ).arg( fmt ) );
        mIdentifyFormats.insert( fmt, format );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "UNEXPECTED resourceType in ResourcURL format=%1 resourceType=%2 template=%3" )
                     .arg( format,
                           resourceType,
                           tmpl ) );
      }
    }

    QgsDebugMsg( QStringLiteral( "add layer %1" ).arg( id ) );
    mTileLayersSupported << l;
  }

  //
  // themes
  //
  mTileThemes.clear();
  for ( QDomElement e0 = e.firstChildElement( QStringLiteral( "Themes" ) ).firstChildElement( QStringLiteral( "Theme" ) );
        !e0.isNull();
        e0 = e0.nextSiblingElement( QStringLiteral( "Theme" ) ) )
  {
    mTileThemes << QgsWmtsTheme();
    parseTheme( e0, mTileThemes.back() );
  }

  // make sure that all layers have a bounding box
  for ( QList<QgsWmtsTileLayer>::iterator it = mTileLayersSupported.begin(); it != mTileLayersSupported.end(); ++it )
  {
    QgsWmtsTileLayer &l = *it;

    if ( l.boundingBoxes.isEmpty() )
    {
      if ( !detectTileLayerBoundingBox( l ) )
      {
        QgsDebugMsg( "failed to detect bounding box for " + l.identifier + " - using extent of the whole world" );

        QgsWmsBoundingBoxProperty bb;
        bb.crs = DEFAULT_LATLON_CRS;
        bb.box = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
        l.boundingBoxes << bb;
      }
    }
  }
}


void QgsWmsCapabilities::parseKeywords( const QDomNode &e, QStringList &keywords )
{
  keywords.clear();

  for ( QDomElement e1 = e.firstChildElement( QStringLiteral( "ows:Keywords" ) ).firstChildElement( QStringLiteral( "ows:Keyword" ) );
        !e1.isNull();
        e1 = e1.nextSiblingElement( QStringLiteral( "ows:Keyword" ) ) )
  {
    keywords << e1.text();
  }
}

void QgsWmsCapabilities::parseTheme( const QDomElement &e, QgsWmtsTheme &t )
{
  t.identifier = e.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
  t.title      = e.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
  t.abstract   = e.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
  parseKeywords( e, t.keywords );

  QDomElement sl = e.firstChildElement( QStringLiteral( "ows:Theme" ) );
  if ( !sl.isNull() )
  {
    t.subTheme = new QgsWmtsTheme;
    parseTheme( sl, *t.subTheme );
  }
  else
  {
    t.subTheme = nullptr;
  }

  t.layerRefs.clear();
  for ( QDomElement e1 = e.firstChildElement( QStringLiteral( "ows:LayerRef" ) );
        !e1.isNull();
        e1 = e1.nextSiblingElement( QStringLiteral( "ows:LayerRef" ) ) )
  {
    t.layerRefs << e1.text();
  }
}

QString QgsWmsCapabilities::nodeAttribute( const QDomElement &e, const QString &name, const QString &defValue )
{
  if ( e.hasAttribute( name ) )
    return e.attribute( name );

  QDomNamedNodeMap map( e.attributes() );
  for ( int i = 0; i < map.size(); i++ )
  {
    QDomAttr attr( map.item( i ).toElement().toAttr() );
    if ( attr.name().compare( name, Qt::CaseInsensitive ) == 0 )
      return attr.value();
  }

  return defValue;
}


bool QgsWmsCapabilities::detectTileLayerBoundingBox( QgsWmtsTileLayer &l )
{
  if ( l.setLinks.isEmpty() )
    return false;

  // take first supported tile matrix set
  const QgsWmtsTileMatrixSetLink &setLink = l.setLinks.constBegin().value();

  QHash<QString, QgsWmtsTileMatrixSet>::const_iterator tmsIt = mTileMatrixSets.constFind( setLink.tileMatrixSet );
  if ( tmsIt == mTileMatrixSets.constEnd() )
    return false;

  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tmsIt->crs );
  if ( !crs.isValid() )
    return false;

  // take most coarse tile matrix ...
  QMap<double, QgsWmtsTileMatrix>::const_iterator tmIt = tmsIt->tileMatrices.constEnd() - 1;
  if ( tmIt == tmsIt->tileMatrices.constEnd() )
    return false;

  const QgsWmtsTileMatrix &tm = *tmIt;
  double metersPerUnit = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters );
  // the magic number below is "standardized rendering pixel size" defined
  // in WMTS (and WMS 1.3) standard, being 0.28 pixel
  double res = tm.scaleDenom * 0.00028 / metersPerUnit;
  QgsPointXY bottomRight( tm.topLeft.x() + res * tm.tileWidth * tm.matrixWidth,
                          tm.topLeft.y() - res * tm.tileHeight * tm.matrixHeight );

  QgsDebugMsg( QStringLiteral( "detecting WMTS layer bounding box: tileset %1 matrix %2 crs %3 res %4" )
               .arg( tmsIt->identifier, tm.identifier, tmsIt->crs ).arg( res ) );

  QgsRectangle extent( tm.topLeft, bottomRight );
  extent.normalize();

  QgsWmsBoundingBoxProperty bb;
  bb.box = extent;
  bb.crs = crs.authid();
  l.boundingBoxes << bb;

  return true;
}


bool QgsWmsCapabilities::shouldInvertAxisOrientation( const QString &ogcCrs )
{
  //according to the WMS spec for 1.3, some CRS have inverted axis
  bool changeXY = false;
  if ( !mParserSettings.ignoreAxisOrientation && ( mCapabilities.version == QLatin1String( "1.3.0" ) || mCapabilities.version == QLatin1String( "1.3" ) ) )
  {
    //have we already checked this crs?
    if ( mCrsInvertAxis.contains( ogcCrs ) )
    {
      //if so, return previous result to save time
      return mCrsInvertAxis[ ogcCrs ];
    }

    //create CRS from string
    QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( ogcCrs );
    if ( srs.isValid() && srs.hasAxisInverted() )
    {
      changeXY = true;
    }

    //cache result to speed up future checks
    mCrsInvertAxis[ ogcCrs ] = changeXY;
  }

  if ( mParserSettings.invertAxisOrientation )
    changeXY = !changeXY;

  return changeXY;
}

int QgsWmsCapabilities::identifyCapabilities() const
{
  int capability = QgsRasterInterface::NoCapabilities;

  for ( auto it = mIdentifyFormats.constBegin(); it != mIdentifyFormats.constEnd(); ++it )
  {
    capability |= QgsRasterDataProvider::identifyFormatToCapability( it.key() );
  }

  return capability;
}



// -----------------

QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mIsAborted( false )
  , mForceRefresh( forceRefresh )
{
}

QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( const QString &baseUrl, const QgsWmsAuthorization &auth, bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mBaseUrl( baseUrl )
  , mAuth( auth )
  , mIsAborted( false )
  , mForceRefresh( forceRefresh )
{
}

QgsWmsCapabilitiesDownload::~QgsWmsCapabilitiesDownload()
{
  abort();
}

bool QgsWmsCapabilitiesDownload::downloadCapabilities( const QString &baseUrl, const QgsWmsAuthorization &auth )
{
  mBaseUrl = baseUrl;
  mAuth = auth;
  return downloadCapabilities();
}

bool QgsWmsCapabilitiesDownload::downloadCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "entering: forceRefresh=%1" ).arg( mForceRefresh ), 2 );
  abort(); // cancel previous
  mIsAborted = false;

  QString url = mBaseUrl;
  if ( !url.contains( QLatin1String( "SERVICE=WMTS" ), Qt::CaseInsensitive ) &&
       !url.contains( QLatin1String( "/WMTSCapabilities.xml" ), Qt::CaseInsensitive ) )
  {
    url += QLatin1String( "SERVICE=WMS&REQUEST=GetCapabilities" );
  }
  QgsDebugMsgLevel( QStringLiteral( "url = %1" ).arg( url ), 2 );

  mError.clear();

  QNetworkRequest request( url );
  if ( !mAuth.setAuthorization( request ) )
  {
    mError = tr( "Download of capabilities failed: network request update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    return false;
  }
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  if ( !mAuth.setAuthorizationReply( mCapabilitiesReply ) )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
    mError = tr( "Download of capabilities failed: network reply update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    return false;
  }
  connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyFinished, Qt::DirectConnection );
  connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyProgress, Qt::DirectConnection );

  QEventLoop loop;
  connect( this, &QgsWmsCapabilitiesDownload::downloadFinished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mError.isEmpty();
}

void QgsWmsCapabilitiesDownload::abort()
{
  mIsAborted = true;
  if ( mCapabilitiesReply )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
  }
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyFinished()
{
  if ( !mIsAborted && mCapabilitiesReply )
  {
    if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsg( QStringLiteral( "reply OK" ) );
      QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {
        emit statusChanged( tr( "Capabilities request redirected." ) );

        const QUrl &toUrl = redirect.toUrl();
        mCapabilitiesReply->request();
        if ( toUrl == mCapabilitiesReply->url() )
        {
          mError = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
          QgsMessageLog::logMessage( mError, tr( "WMS" ) );
          mHttpCapabilitiesResponse.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );
          if ( !mAuth.setAuthorization( request ) )
          {
            mHttpCapabilitiesResponse.clear();
            mError = tr( "Download of capabilities failed: network request update failed for authentication config" );
            QgsMessageLog::logMessage( mError, tr( "WMS" ) );
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mCapabilitiesReply->deleteLater();
          mCapabilitiesReply = nullptr;

          QgsDebugMsg( QStringLiteral( "redirected getcapabilities: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ) );
          mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

          if ( !mAuth.setAuthorizationReply( mCapabilitiesReply ) )
          {
            mHttpCapabilitiesResponse.clear();
            mCapabilitiesReply->deleteLater();
            mCapabilitiesReply = nullptr;
            mError = tr( "Download of capabilities failed: network reply update failed for authentication config" );
            QgsMessageLog::logMessage( mError, tr( "WMS" ) );
            emit downloadFinished();
            return;
          }

          connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyFinished, Qt::DirectConnection );
          connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyProgress, Qt::DirectConnection );
          return;
        }
      }
      else
      {
        const QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

        if ( nam->cache() )
        {
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mCapabilitiesReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          Q_FOREACH ( const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders() )
          {
            if ( h.first != "Cache-Control" )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsg( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ) );
          if ( cmd.expirationDate().isNull() )
          {
            QgsSettings s;
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( QStringLiteral( "qgis/defaultCapabilitiesExpiry" ), "24" ).toInt() * 60 * 60 ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "No cache for capabilities!" ) );
        }

#ifdef QGISDEBUG
        bool fromCache = mCapabilitiesReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsg( QStringLiteral( "Capabilities reply was cached: %1" ).arg( fromCache ) );
#endif

        mHttpCapabilitiesResponse = mCapabilitiesReply->readAll();

        if ( mHttpCapabilitiesResponse.isEmpty() )
        {
          mError = tr( "empty of capabilities: %1" ).arg( mCapabilitiesReply->errorString() );
        }
      }
    }
    else
    {
      mError = tr( "Download of capabilities failed: %1" ).arg( mCapabilitiesReply->errorString() );
      QgsMessageLog::logMessage( mError, tr( "WMS" ) );
      mHttpCapabilitiesResponse.clear();
    }
  }

  if ( mCapabilitiesReply )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
  }

  emit downloadFinished();
}

QRectF QgsWmtsTileMatrix::tileRect( int col, int row ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;
  return QRectF( topLeft.x() + col * twMap, topLeft.y() - ( row + 1 ) * thMap, twMap, thMap );
}

QgsRectangle QgsWmtsTileMatrix::tileBBox( int col, int row ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;
  return QgsRectangle(
           topLeft.x() +         col * twMap,
           topLeft.y() - ( row + 1 ) * thMap,
           topLeft.x() + ( col + 1 ) * twMap,
           topLeft.y() -         row * thMap );
}

void QgsWmtsTileMatrix::viewExtentIntersection( const QgsRectangle &viewExtent, const QgsWmtsTileMatrixLimits *tml, int &col0, int &row0, int &col1, int &row1 ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;

  int minTileCol = 0;
  int maxTileCol = matrixWidth - 1;
  int minTileRow = 0;
  int maxTileRow = matrixHeight - 1;

  if ( tml )
  {
    minTileCol = tml->minTileCol;
    maxTileCol = tml->maxTileCol;
    minTileRow = tml->minTileRow;
    maxTileRow = tml->maxTileRow;
    //QgsDebugMsg( QStringLiteral( "%1 %2: TileMatrixLimits col %3-%4 row %5-%6" )
    //             .arg( tileMatrixSet->identifier, identifier )
    //             .arg( minTileCol ).arg( maxTileCol )
    //             .arg( minTileRow ).arg( maxTileRow ) );
  }

  col0 = qBound( minTileCol, ( int ) std::floor( ( viewExtent.xMinimum() - topLeft.x() ) / twMap ), maxTileCol );
  row0 = qBound( minTileRow, ( int ) std::floor( ( topLeft.y() - viewExtent.yMaximum() ) / thMap ), maxTileRow );
  col1 = qBound( minTileCol, ( int ) std::floor( ( viewExtent.xMaximum() - topLeft.x() ) / twMap ), maxTileCol );
  row1 = qBound( minTileRow, ( int ) std::floor( ( topLeft.y() - viewExtent.yMinimum() ) / thMap ), maxTileRow );
}

const QgsWmtsTileMatrix *QgsWmtsTileMatrixSet::findNearestResolution( double vres ) const
{
  QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = tileMatrices.constBegin();
  while ( it != tileMatrices.constEnd() && it.key() < vres )
  {
    //QgsDebugMsg( QStringLiteral( "res:%1 >= %2" ).arg( it.key() ).arg( vres ) );
    prev = it;
    ++it;
  }

  if ( it == tileMatrices.constEnd() ||
       ( it != tileMatrices.constBegin() && vres - prev.key() < it.key() - vres ) )
  {
    //QgsDebugMsg( QStringLiteral( "back to previous res" ) );
    it = prev;
  }

  return &it.value();
}

const QgsWmtsTileMatrix *QgsWmtsTileMatrixSet::findOtherResolution( double tres, int offset ) const
{
  QMap<double, QgsWmtsTileMatrix>::const_iterator it = tileMatrices.constFind( tres );
  if ( it == tileMatrices.constEnd() )
    return nullptr;
  while ( true )
  {
    if ( offset > 0 )
    {
      ++it;
      --offset;
    }
    else if ( offset < 0 )
    {
      if ( it == tileMatrices.constBegin() )
        return nullptr;
      --it;
      ++offset;
    }
    else
      break;

    if ( it == tileMatrices.constEnd() )
      return nullptr;
  }
  return &it.value();
}
