
#include "qgswmscapabilities.h"
#include "qgswmsprovider.h"

#include <QFile>
#include <QDir>

#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"


// %%% copied from qgswmsprovider.cpp
static QString DEFAULT_LATLON_CRS = "CRS:84";



bool QgsWmsSettings::parseUri( QString uriString )
{
  QgsDebugMsg( "uriString = " + uriString );
  QgsDataSourceURI uri;
  uri.setEncodedUri( uriString );

  mTiled = false;
  mTileDimensionValues.clear();

  mHttpUri = uri.param( "url" );
  mBaseUrl = QgsWmsProvider::prepareUri( mHttpUri ); // must set here, setImageCrs is using that
  QgsDebugMsg( "mBaseUrl = " + mBaseUrl );

  mIgnoreGetMapUrl = uri.hasParam( "IgnoreGetMapUrl" );
  mIgnoreGetFeatureInfoUrl = uri.hasParam( "IgnoreGetFeatureInfoUrl" );
  mParserSettings.ignoreAxisOrientation = uri.hasParam( "IgnoreAxisOrientation" ); // must be before parsing!
  mParserSettings.invertAxisOrientation = uri.hasParam( "InvertAxisOrientation" ); // must be before parsing!
  mSmoothPixmapTransform = uri.hasParam( "SmoothPixmapTransform" );

  mDpiMode = uri.hasParam( "dpiMode" ) ? ( QgsWmsDpiMode ) uri.param( "dpiMode" ).toInt() : dpiAll;

  mAuth.mUserName = uri.param( "username" );
  QgsDebugMsg( "set username to " + mAuth.mUserName );

  mAuth.mPassword = uri.param( "password" );
  QgsDebugMsg( "set password to " + mAuth.mPassword );

  mAuth.mReferer = uri.param( "referer" );
  QgsDebugMsg( "set referer to " + mAuth.mReferer );

  mActiveSubLayers = uri.params( "layers" );
  mActiveSubStyles = uri.params( "styles" );
  QgsDebugMsg( "Entering: layers:" + mActiveSubLayers.join( ", " ) + ", styles:" + mActiveSubStyles.join( ", " ) );

  mImageMimeType = uri.param( "format" );
  QgsDebugMsg( "Setting image encoding to " + mImageMimeType + "." );

  mMaxWidth = 0;
  mMaxHeight = 0;
  if ( uri.hasParam( "maxWidth" ) && uri.hasParam( "maxHeight" ) )
  {
    mMaxWidth = uri.param( "maxWidth" ).toInt();
    mMaxHeight = uri.param( "maxHeight" ).toInt();
  }

  if ( uri.hasParam( "tileMatrixSet" ) )
  {
    mTiled = true;
    // tileMatrixSet may be empty if URI was converted from < 1.9 project file URI
    // in that case it means that the source is WMS-C
    mTileMatrixSetId = uri.param( "tileMatrixSet" );
  }

  if ( uri.hasParam( "tileDimensions" ) )
  {
    mTiled = true;
    foreach ( QString param, uri.param( "tileDimensions" ).split( ";" ) )
    {
      QStringList kv = param.split( "=" );
      if ( kv.size() == 1 )
      {
        mTileDimensionValues.insert( kv[0], QString::null );
      }
      else if ( kv.size() == 2 )
      {
        mTileDimensionValues.insert( kv[0], kv[1] );
      }
      else
      {
        QgsDebugMsg( QString( "skipped dimension %1" ).arg( param ) );
      }
    }
  }

  mCrsId = uri.param( "crs" );

  mEnableContextualLegend = uri.param( "contextualWMSLegend" ).toInt();
  QgsDebugMsg( QString( "Contextual legend: %1" ).arg( mEnableContextualLegend ) );

  mFeatureCount = uri.param( "featureCount" ).toInt(); // default to 0

  return true;
}


// ----------------------


QgsWmsCapabilities::QgsWmsCapabilities()
    : mValid( false )
    , mLayerCount( -1 )
    , mCapabilities()
{
}

bool QgsWmsCapabilities::parseResponse( const QByteArray& response, const QgsWmsParserSettings& settings )
{
  mParserSettings = settings;
  mValid = false;

  if ( response.isEmpty() )
  {
    if ( mError.isEmpty() )
    {
      mErrorFormat = "text/plain";
      mError = QObject::tr( "empty capabilities document" );
    }
    QgsDebugMsg( "response is empty" );
    return false;
  }

  if ( response.startsWith( "<html>" ) ||
       response.startsWith( "<HTML>" ) )
  {
    mErrorFormat = "text/html";
    mError = response;
    QgsDebugMsg( "starts with <html>" );
    return false;
  }


  QgsDebugMsg( "Converting to Dom." );

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
  foreach ( QString f, mCapabilities.capability.request.getFeatureInfo.format )
  {
    // Don't use mSupportedGetFeatureFormats, there are too many possibilities
    QgsDebugMsg( "supported format = " + f );
    // 1.0: MIME - server shall choose format, we presume it to be plain text
    //      GML.1, GML.2, or GML.3
    // 1.1.0, 1.3.0 - mime types, GML should use application/vnd.ogc.gml
    //      but in UMN Mapserver it may be also OUTPUTFORMAT, e.g. OGRGML
    QgsRaster::IdentifyFormat format = QgsRaster::IdentifyFormatUndefined;
    if ( f == "MIME" )
      format = QgsRaster::IdentifyFormatText; // 1.0
    else if ( f == "text/plain" )
      format = QgsRaster::IdentifyFormatText;
    else if ( f == "text/html" )
      format = QgsRaster::IdentifyFormatHtml;
    else if ( f.startsWith( "GML." ) )
      format = QgsRaster::IdentifyFormatFeature; // 1.0
    else if ( f == "application/vnd.ogc.gml" )
      format = QgsRaster::IdentifyFormatFeature;
    else if ( f == "application/json" )
      format = QgsRaster::IdentifyFormatFeature;
    else if ( f.contains( "gml", Qt::CaseInsensitive ) )
      format = QgsRaster::IdentifyFormatFeature;

    mIdentifyFormats.insert( format, f );
  }

  QgsDebugMsg( "exiting." );

  mValid = mError.isEmpty();
  return mValid;
}


bool QgsWmsCapabilities::parseCapabilitiesDom( QByteArray const &xml, QgsWmsCapabilitiesProperty& capabilitiesProperty )
{
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wmsprovider-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly ) )
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
    mErrorFormat = "text/plain";
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
    docElem.tagName() != "WMS_Capabilities"  &&   // (1.3 vintage)
    docElem.tagName() != "WMT_MS_Capabilities" && // (1.1.1 vintage)
    docElem.tagName() != "Capabilities"           // WMTS
  )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = "text/plain";
    mError = QObject::tr( "Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found.\nThis might be due to an incorrect WMS Server URL.\nTag:%3\nResponse was:\n%4" )
             .arg( "WMS_Capabilities" )
             .arg( "WMT_MS_Capabilities" )
             .arg( docElem.tagName() )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilitiesProperty.version = docElem.attribute( "version" );

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      QgsDebugMsg( e.tagName() ); // the node really is an element.

      if ( e.tagName() == "Service" || e.tagName() == "ows:ServiceProvider" || e.tagName() == "ows:ServiceIdentification" )
      {
        QgsDebugMsg( "  Service." );
        parseService( e, capabilitiesProperty.service );
      }
      else if ( e.tagName() == "Capability" || e.tagName() == "ows:OperationsMetadata" )
      {
        QgsDebugMsg( "  Capability." );
        parseCapability( e, capabilitiesProperty.capability );
      }
      else if ( e.tagName() == "Contents" )
      {
        QgsDebugMsg( "  Contents." );
        parseWMTSContents( e );
      }
    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return true;
}


void QgsWmsCapabilities::parseService( QDomElement const & e, QgsWmsServiceProperty& serviceProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      // QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( "ows:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Title" )
      {
        serviceProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        serviceProperty.abstract = e1.text();
      }
      else if ( tagName == "KeywordList" || tagName == "Keywords" )
      {
        parseKeywordList( e1, serviceProperty.keywordList );
      }
      else if ( tagName == "OnlineResource" )
      {
        parseOnlineResource( e1, serviceProperty.onlineResource );
      }
      else if ( tagName == "ContactInformation" || tagName == "ServiceContact" )
      {
        parseContactInformation( e1, serviceProperty.contactInformation );
      }
      else if ( tagName == "Fees" )
      {
        serviceProperty.fees = e1.text();
      }
      else if ( tagName == "AccessConstraints" )
      {
        serviceProperty.accessConstraints = e1.text();
      }
      else if ( tagName == "LayerLimit" )
      {
        serviceProperty.layerLimit = e1.text().toUInt();
      }
      else if ( tagName == "MaxWidth" )
      {
        serviceProperty.maxWidth = e1.text().toUInt();
      }
      else if ( tagName == "MaxHeight" )
      {
        serviceProperty.maxHeight = e1.text().toUInt();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseOnlineResource( QDomElement const & e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute )
{
  QgsDebugMsg( "entering." );

  onlineResourceAttribute.xlinkHref = QUrl::fromEncoded( e.attribute( "xlink:href" ).toUtf8() ).toString();

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseKeywordList( QDomElement  const & e, QStringList& keywordListProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( "ows:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Keyword" )
      {
        QgsDebugMsg( "      Keyword." );
        keywordListProperty += e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parseContactInformation( QDomElement const & e, QgsWmsContactInformationProperty& contactInformationProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPersonPrimary" )
      {
        parseContactPersonPrimary( e1, contactInformationProperty.contactPersonPrimary );
      }
      else if ( tagName == "ContactPosition" || tagName == "ows:PositionName" )
      {
        contactInformationProperty.contactPosition = e1.text();
      }
      else if ( tagName == "ContactAddress" )
      {
        parseContactAddress( e1, contactInformationProperty.contactAddress );
      }
      else if ( tagName == "ContactVoiceTelephone" )
      {
        contactInformationProperty.contactVoiceTelephone = e1.text();
      }
      else if ( tagName == "ContactFacsimileTelephone" )
      {
        contactInformationProperty.contactFacsimileTelephone = e1.text();
      }
      else if ( tagName == "ContactElectronicMailAddress" )
      {
        contactInformationProperty.contactElectronicMailAddress = e1.text();
      }
      else if ( tagName == "ows:IndividualName" )
      {
        contactInformationProperty.contactPersonPrimary.contactPerson = e1.text();
      }
      else if ( tagName == "ows:ProviderName" )
      {
        contactInformationProperty.contactPersonPrimary.contactOrganization = e1.text();
      }
      else if ( tagName == "ows:ContactInfo" )
      {
        QDomNode n = n1.firstChildElement( "ows:Phone" );
        contactInformationProperty.contactVoiceTelephone        = n.firstChildElement( "ows:Voice" ).toElement().text();
        contactInformationProperty.contactFacsimileTelephone    = n.firstChildElement( "ows:Facsimile" ).toElement().text();

        n = n1.firstChildElement( "ows:Address" );
        contactInformationProperty.contactElectronicMailAddress   = n.firstChildElement( "ows:ElectronicMailAddress" ).toElement().text();
        contactInformationProperty.contactAddress.address         = n.firstChildElement( "ows:DeliveryPoint" ).toElement().text();
        contactInformationProperty.contactAddress.city            = n.firstChildElement( "ows:City" ).toElement().text();
        contactInformationProperty.contactAddress.stateOrProvince = n.firstChildElement( "ows:AdministrativeArea" ).toElement().text();
        contactInformationProperty.contactAddress.postCode        = n.firstChildElement( "ows:PostalCode" ).toElement().text();
        contactInformationProperty.contactAddress.country         = n.firstChildElement( "ows:Country" ).toElement().text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parseContactPersonPrimary( QDomElement const & e, QgsWmsContactPersonPrimaryProperty& contactPersonPrimaryProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPerson" )
      {
        contactPersonPrimaryProperty.contactPerson = e1.text();
      }
      else if ( tagName == "ContactOrganization" )
      {
        contactPersonPrimaryProperty.contactOrganization = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseContactAddress( QDomElement const & e, QgsWmsContactAddressProperty& contactAddressProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "AddressType" )
      {
        contactAddressProperty.addressType = e1.text();
      }
      else if ( tagName == "Address" )
      {
        contactAddressProperty.address = e1.text();
      }
      else if ( tagName == "City" )
      {
        contactAddressProperty.city = e1.text();
      }
      else if ( tagName == "StateOrProvince" )
      {
        contactAddressProperty.stateOrProvince = e1.text();
      }
      else if ( tagName == "PostCode" )
      {
        contactAddressProperty.postCode = e1.text();
      }
      else if ( tagName == "Country" )
      {
        contactAddressProperty.country = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseCapability( QDomElement const & e, QgsWmsCapabilityProperty& capabilityProperty )
{
  QgsDebugMsg( "entering." );

  for ( QDomNode n1 = e.firstChild(); !n1.isNull(); n1 = n1.nextSibling() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( e1.isNull() )
      continue;

    QString tagName = e1.tagName();
    if ( tagName.startsWith( "wms:" ) )
      tagName = tagName.mid( 4 );

    QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

    if ( tagName == "Request" )
    {
      parseRequest( e1, capabilityProperty.request );
    }
    else if ( tagName == "Layer" )
    {
      parseLayer( e1, capabilityProperty.layer );
    }
    else if ( tagName == "VendorSpecificCapabilities" )
    {
      for ( int i = 0; i < e1.childNodes().size(); i++ )
      {
        QDomNode n2 = e1.childNodes().item( i );
        QDomElement e2 = n2.toElement();

        QString tagName = e2.tagName();
        if ( tagName.startsWith( "wms:" ) )
          tagName = tagName.mid( 4 );

        if ( tagName == "TileSet" )
        {
          parseTileSetProfile( e2 );
        }
      }
    }
    else if ( tagName == "ows:Operation" )
    {
      QString name = e1.attribute( "name" );
      QDomElement get = n1.firstChildElement( "ows:DCP" )
                        .firstChildElement( "ows:HTTP" )
                        .firstChildElement( "ows:Get" );

      QString href = get.attribute( "xlink:href" );

      QgsWmsDcpTypeProperty dcp;
      dcp.http.get.onlineResource.xlinkHref = href;

      QgsWmsOperationType *ot = 0;
      if ( href.isNull() )
      {
        QgsDebugMsg( QString( "http get missing from ows:Operation '%1'" ).arg( name ) );
      }
      else if ( name == "GetTile" )
      {
        ot = &capabilityProperty.request.getTile;
      }
      else if ( name == "GetFeatureInfo" )
      {
        ot = &capabilityProperty.request.getFeatureInfo;
      }
      else if ( name == "GetLegendGraphic" || name == "sld:GetLegendGraphic" )
      {
        ot = &capabilityProperty.request.getLegendGraphic;
      }
      else
      {
        QgsDebugMsg( QString( "ows:Operation %1 ignored" ).arg( name ) );
      }

      if ( ot )
      {
        ot->dcpType << dcp;
        ot->allowedEncodings.clear();
        for ( QDomElement e2 = get.firstChildElement( "ows:Constraint" ).firstChildElement( "ows:AllowedValues" ).firstChildElement( "ows:Value" );
              !e2.isNull();
              e2 = e1.nextSiblingElement( "ows:Value" ) )
        {
          ot->allowedEncodings << e2.text();
        }
      }
    }
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseRequest( QDomElement const & e, QgsWmsRequestProperty& requestProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString operation = e1.tagName();
      if ( operation == "Operation" )
      {
        operation = e1.attribute( "name" );
      }

      if ( operation == "GetMap" )
      {
        QgsDebugMsg( "      GetMap." );
        parseOperationType( e1, requestProperty.getMap );
      }
      else if ( operation == "GetFeatureInfo" )
      {
        QgsDebugMsg( "      GetFeatureInfo." );
        parseOperationType( e1, requestProperty.getFeatureInfo );
      }
      else if ( operation == "GetLegendGraphic" || operation == "sld:GetLegendGraphic" )
      {
        QgsDebugMsg( "      GetLegendGraphic." );
        parseOperationType( e1, requestProperty.getLegendGraphic );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}



void QgsWmsCapabilities::parseLegendUrl( QDomElement const & e, QgsWmsLegendUrlProperty& legendUrlProperty )
{
  QgsDebugMsg( "entering." );

  legendUrlProperty.width  = e.attribute( "width" ).toUInt();
  legendUrlProperty.height = e.attribute( "height" ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format" )
      {
        legendUrlProperty.format = e1.text();
      }
      else if ( tagName == "OnlineResource" )
      {
        parseOnlineResource( e1, legendUrlProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parseLayer( QDomElement const & e, QgsWmsLayerProperty& layerProperty,
                                     QgsWmsLayerProperty *parentProperty )
{
  //QgsDebugMsg( "entering." );

// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
//  // enforce WMS non-inheritance rules
//  layerProperty.name =        QString::null;
//  layerProperty.title =       QString::null;
//  layerProperty.abstract =    QString::null;
//  layerProperty.keywordList.clear();
  layerProperty.orderId     = ++mLayerCount;
  layerProperty.queryable   = e.attribute( "queryable" ).toUInt();
  layerProperty.cascaded    = e.attribute( "cascaded" ).toUInt();
  layerProperty.opaque      = e.attribute( "opaque" ).toUInt();
  layerProperty.noSubsets   = e.attribute( "noSubsets" ).toUInt();
  layerProperty.fixedWidth  = e.attribute( "fixedWidth" ).toUInt();
  layerProperty.fixedHeight = e.attribute( "fixedHeight" ).toUInt();

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      //QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layer" )
      {
        //QgsDebugMsg( "      Nested layer." );

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
      else if ( tagName == "Name" )
      {
        layerProperty.name = e1.text();
      }
      else if ( tagName == "Title" )
      {
        layerProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        layerProperty.abstract = e1.text();
      }
      else if ( tagName == "KeywordList" )
      {
        parseKeywordList( e1, layerProperty.keywordList );
      }
      else if ( tagName == "SRS" || tagName == "CRS" )
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        foreach ( QString srs, e1.text().split( QRegExp( "\\s+" ) ) )
        {
          layerProperty.crs.push_back( srs );
        }
      }
      else if ( tagName == "LatLonBoundingBox" )      // legacy from earlier versions of WMS
      {
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              e1.attribute( "minx" ).toDouble(),
              e1.attribute( "miny" ).toDouble(),
              e1.attribute( "maxx" ).toDouble(),
              e1.attribute( "maxy" ).toDouble()
            );

        if ( e1.hasAttribute( "SRS" ) && e1.attribute( "SRS" ) != DEFAULT_LATLON_CRS )
        {
          try
          {
            QgsCoordinateReferenceSystem src;
            src.createFromOgcWmsCrs( e1.attribute( "SRS" ) );

            QgsCoordinateReferenceSystem dst;
            dst.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );

            QgsCoordinateTransform ct( src, dst );
            layerProperty.ex_GeographicBoundingBox = ct.transformBoundingBox( layerProperty.ex_GeographicBoundingBox );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse );
          }
        }
      }
      else if ( tagName == "EX_GeographicBoundingBox" ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        if ( e1.tagName() == "wms:EX_GeographicBoundingBox" )
        {
          wBoundLongitudeElem = n1.namedItem( "wms:westBoundLongitude" ).toElement();
          eBoundLongitudeElem = n1.namedItem( "wms:eastBoundLongitude" ).toElement();
          sBoundLatitudeElem = n1.namedItem( "wms:southBoundLatitude" ).toElement();
          nBoundLatitudeElem = n1.namedItem( "wms:northBoundLatitude" ).toElement();
        }
        else
        {
          wBoundLongitudeElem = n1.namedItem( "westBoundLongitude" ).toElement();
          eBoundLongitudeElem = n1.namedItem( "eastBoundLongitude" ).toElement();
          sBoundLatitudeElem = n1.namedItem( "southBoundLatitude" ).toElement();
          nBoundLatitudeElem = n1.namedItem( "northBoundLatitude" ).toElement();
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
      else if ( tagName == "BoundingBox" )
      {
        // TODO: overwrite inherited
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( e1.attribute( "minx" ).toDouble(),
                                 e1.attribute( "miny" ).toDouble(),
                                 e1.attribute( "maxx" ).toDouble(),
                                 e1.attribute( "maxy" ).toDouble()
                               );
        if ( e1.hasAttribute( "CRS" ) || e1.hasAttribute( "SRS" ) )
        {
          if ( e1.hasAttribute( "CRS" ) )
            bbox.crs = e1.attribute( "CRS" );
          else if ( e1.hasAttribute( "SRS" ) )
            bbox.crs = e1.attribute( "SRS" );

          if ( shouldInvertAxisOrientation( bbox.crs ) )
          {
            QgsRectangle invAxisBbox( bbox.box.yMinimum(), bbox.box.xMinimum(),
                                      bbox.box.yMaximum(), bbox.box.xMaximum() );
            bbox.box = invAxisBbox;
          }

          layerProperty.boundingBoxes << bbox;
        }
        else
        {
          QgsDebugMsg( "CRS/SRS attribute not found in BoundingBox" );
        }
      }
      else if ( tagName == "Dimension" )
      {
        // TODO
      }
      else if ( tagName == "Attribution" )
      {
        // TODO
      }
      else if ( tagName == "AuthorityURL" )
      {
        // TODO
      }
      else if ( tagName == "Identifier" )
      {
        // TODO
      }
      else if ( tagName == "MetadataURL" )
      {
        // TODO
      }
      else if ( tagName == "DataURL" )
      {
        // TODO
      }
      else if ( tagName == "FeatureListURL" )
      {
        // TODO
      }
      else if ( tagName == "Style" )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( e1, styleProperty );

        layerProperty.style.push_back( styleProperty );
      }
      else if ( tagName == "MinScaleDenominator" )
      {
        // TODO
      }
      else if ( tagName == "MaxScaleDenominator" )
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

    // Store the available Coordinate Reference Systems for the layer so that it
    // can be combined with others later in supportedCrsForLayers()
    mCrsForLayer[ layerProperty.name ] = layerProperty.crs;

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

  if ( !parentProperty )
  {
    // Why clear()? I need top level access. Seems to work in standard select dialog without clear.
    //layerProperty.layer.clear();
    layerProperty.crs.clear();
  }

  //QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseStyle( QDomElement const & e, QgsWmsStyleProperty& styleProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Name" )
      {
        styleProperty.name = e1.text();
      }
      else if ( tagName == "Title" )
      {
        styleProperty.title = e1.text();
      }
      else if ( tagName == "Abstract" )
      {
        styleProperty.abstract = e1.text();
      }
      else if ( tagName == "LegendURL" )
      {
        styleProperty.legendUrl << QgsWmsLegendUrlProperty();
        parseLegendUrl( e1, styleProperty.legendUrl.last() );
      }
      else if ( tagName == "StyleSheetURL" )
      {
        // TODO
      }
      else if ( tagName == "StyleURL" )
      {
        // TODO
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseOperationType( QDomElement const & e, QgsWmsOperationType& operationType )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format" )
      {
        QgsDebugMsg( "      Format." );
        operationType.format += e1.text();
      }
      else if ( tagName == "DCPType" )
      {
        QgsDebugMsg( "      DCPType." );
        QgsWmsDcpTypeProperty dcp;
        parseDcpType( e1, dcp );
        operationType.dcpType.push_back( dcp );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsCapabilities::parseDcpType( QDomElement const & e, QgsWmsDcpTypeProperty& dcpType )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "HTTP" )
      {
        QgsDebugMsg( "      HTTP." );
        parseHttp( e1, dcpType.http );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parseHttp( QDomElement const & e, QgsWmsHttpProperty& httpProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Get" )
      {
        QgsDebugMsg( "      Get." );
        parseGet( e1, httpProperty.get );
      }
      else if ( tagName == "Post" )
      {
        QgsDebugMsg( "      Post." );
        parsePost( e1, httpProperty.post );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parseGet( QDomElement const & e, QgsWmsGetProperty& getProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, getProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}

void QgsWmsCapabilities::parsePost( QDomElement const & e, QgsWmsPostProperty& postProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QString tagName = e1.tagName();
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, postProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
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
      if ( tagName.startsWith( "wms:" ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layers" )
      {
        layers << e1.text();
      }
      else if ( tagName == "Styles" )
      {
        styles << e1.text();
      }
      else if ( tagName == "Width" )
      {
        m.tileWidth = e1.text().toInt();
      }
      else if ( tagName == "Height" )
      {
        m.tileHeight = e1.text().toInt();
      }
      else if ( tagName == "SRS" )
      {
        ms.crs = e1.text();
      }
      else if ( tagName == "Format" )
      {
        l.formats << e1.text();
      }
      else if ( tagName == "BoundingBox" )
      {
        QgsWmsBoundingBoxProperty bb;
        bb.box = QgsRectangle(
                   e1.attribute( "minx" ).toDouble(),
                   e1.attribute( "miny" ).toDouble(),
                   e1.attribute( "maxx" ).toDouble(),
                   e1.attribute( "maxy" ).toDouble()
                 );
        if ( e1.hasAttribute( "SRS" ) )
          bb.crs = e1.attribute( "SRS" );
        else if ( e1.hasAttribute( "srs" ) )
          bb.crs = e1.attribute( "srs" );
        else if ( e1.hasAttribute( "CRS" ) )
          bb.crs = e1.attribute( "CRS" );
        else if ( e1.hasAttribute( "crs" ) )
          bb.crs = e1.attribute( "crs" );
        else
        {
          QgsDebugMsg( "crs of bounding box undefined" );
        }

        if ( !bb.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs;
          crs.createFromOgcWmsCrs( bb.crs );
          if ( crs.isValid() )
            bb.crs = crs.authid();

          l.boundingBoxes << bb;
        }
      }
      else if ( tagName == "Resolutions" )
      {
        resolutions = e1.text().trimmed().split( " ", QString::SkipEmptyParts );
      }
      else
      {
        QgsDebugMsg( QString( "tileset tag %1 ignored" ).arg( e1.tagName() ) );
      }
    }
    n1 = n1.nextSibling();
  }

  ms.identifier = QString( "%1-wmsc-%2" ).arg( layers.join( "_" ) ).arg( mTileLayersSupported.size() );

  l.identifier = layers.join( "," );
  QgsWmtsStyle s;
  s.identifier = styles.join( "," );
  l.styles.insert( s.identifier, s );
  l.defaultStyle = s.identifier;

  QgsWmtsTileMatrixSetLink sl;
  sl.tileMatrixSet = ms.identifier;
  l.setLinks.insert( ms.identifier, sl );
  mTileLayersSupported.append( l );

  int i = 0;
  foreach ( QString rS, resolutions )
  {
    double r = rS.toDouble();
    m.identifier = QString::number( i );
    Q_ASSERT( l.boundingBoxes.size() == 1 );
    m.matrixWidth  = ceil( l.boundingBoxes[0].box.width() / m.tileWidth / r );
    m.matrixHeight = ceil( l.boundingBoxes[0].box.height() / m.tileHeight / r );
    m.topLeft = QgsPoint( l.boundingBoxes[0].box.xMinimum(), l.boundingBoxes[0].box.yMinimum() + m.matrixHeight * m.tileHeight * r );
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
  for ( QDomElement e0 = e.firstChildElement( "TileMatrixSet" );
        !e0.isNull();
        e0 = e0.nextSiblingElement( "TileMatrixSet" ) )
  {
    QgsWmtsTileMatrixSet s;
    s.identifier = e0.firstChildElement( "ows:Identifier" ).text();
    s.title      = e0.firstChildElement( "ows:Title" ).text();
    s.abstract   = e0.firstChildElement( "ows:Abstract" ).text();
    parseKeywords( e0, s.keywords );

    QString supportedCRS = e0.firstChildElement( "ows:SupportedCRS" ).text();

    QgsCoordinateReferenceSystem crs;
    crs.createFromOgcWmsCrs( supportedCRS );

    s.wkScaleSet = e0.firstChildElement( "WellKnownScaleSet" ).text();

    double metersPerUnit = QGis::fromUnitToUnitFactor( crs.mapUnits(), QGis::Meters );

    s.crs = crs.authid();

    bool invert = !mParserSettings.ignoreAxisOrientation && crs.axisInverted();
    if ( mParserSettings.invertAxisOrientation )
      invert = !invert;

    QgsDebugMsg( QString( "tilematrix set: %1 (supportedCRS:%2 crs:%3; metersPerUnit:%4 axisInverted:%5)" )
                 .arg( s.identifier )
                 .arg( supportedCRS )
                 .arg( s.crs )
                 .arg( metersPerUnit, 0, 'f' )
                 .arg( invert ? "yes" : "no" )
               );

    for ( QDomElement e1 = e0.firstChildElement( "TileMatrix" );
          !e1.isNull();
          e1 = e1.nextSiblingElement( "TileMatrix" ) )
    {
      QgsWmtsTileMatrix m;

      m.identifier = e1.firstChildElement( "ows:Identifier" ).text();
      m.title      = e1.firstChildElement( "ows:Title" ).text();
      m.abstract   = e1.firstChildElement( "ows:Abstract" ).text();
      parseKeywords( e1, m.keywords );

      m.scaleDenom = e1.firstChildElement( "ScaleDenominator" ).text().toDouble();

      QStringList topLeft = e1.firstChildElement( "TopLeftCorner" ).text().split( " " );
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
        QgsDebugMsg( "Could not parse topLeft" );
        continue;
      }

      m.tileWidth    = e1.firstChildElement( "TileWidth" ).text().toInt();
      m.tileHeight   = e1.firstChildElement( "TileHeight" ).text().toInt();
      m.matrixWidth  = e1.firstChildElement( "MatrixWidth" ).text().toInt();
      m.matrixHeight = e1.firstChildElement( "MatrixHeight" ).text().toInt();

      double res = m.scaleDenom * 0.00028 / metersPerUnit;

      QgsDebugMsg( QString( " %1: scale=%2 res=%3 tile=%4x%5 matrix=%6x%7 topLeft=%8" )
                   .arg( m.identifier )
                   .arg( m.scaleDenom ).arg( res )
                   .arg( m.tileWidth ).arg( m.tileHeight )
                   .arg( m.matrixWidth ).arg( m.matrixHeight )
                   .arg( m.topLeft.toString() )
                 );

      s.tileMatrices.insert( res, m );
    }

    mTileMatrixSets.insert( s.identifier, s );
  }

  //
  // layers
  //

  mTileLayersSupported.clear();
  for ( QDomElement e0 = e.firstChildElement( "Layer" );
        !e0.isNull();
        e0 = e0.nextSiblingElement( "Layer" ) )
  {
    QString id = e0.firstChildElement( "ows:Identifier" ).text();
    QgsDebugMsg( QString( "Layer %1" ).arg( id ) );

    QgsWmtsTileLayer l;
    l.tileMode   = WMTS;
    l.identifier = e0.firstChildElement( "ows:Identifier" ).text();
    l.title      = e0.firstChildElement( "ows:Title" ).text();
    l.abstract   = e0.firstChildElement( "ows:Abstract" ).text();
    parseKeywords( e0, l.keywords );

    QgsWmsBoundingBoxProperty bb;

    QDomElement bbox = e0.firstChildElement( "ows:WGS84BoundingBox" );
    if ( !bbox.isNull() )
    {
      QStringList ll = bbox.firstChildElement( "ows:LowerCorner" ).text().split( " " );
      QStringList ur = bbox.firstChildElement( "ows:UpperCorner" ).text().split( " " );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        bb.crs = DEFAULT_LATLON_CRS;
        bb.box = QgsRectangle( QgsPoint( ll[0].toDouble(), ll[1].toDouble() ),
                               QgsPoint( ur[0].toDouble(), ur[1].toDouble() ) );

        l.boundingBoxes << bb;
      }
    }

    for ( bbox = e0.firstChildElement( "ows:BoundingBox" );
          !bbox.isNull();
          bbox = bbox.nextSiblingElement( "ows:BoundingBox" ) )
    {
      QStringList ll = bbox.firstChildElement( "ows:LowerCorner" ).text().split( " " );
      QStringList ur = bbox.firstChildElement( "ows:UpperCorner" ).text().split( " " );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        bb.box = QgsRectangle( QgsPoint( ll[0].toDouble(), ll[1].toDouble() ),
                               QgsPoint( ur[0].toDouble(), ur[1].toDouble() ) );

        if ( bbox.hasAttribute( "SRS" ) )
          bb.crs = bbox.attribute( "SRS" );
        else if ( bbox.hasAttribute( "srs" ) )
          bb.crs = bbox.attribute( "srs" );
        else if ( bbox.hasAttribute( "CRS" ) )
          bb.crs = bbox.attribute( "CRS" );
        else if ( bbox.hasAttribute( "crs" ) )
          bb.crs = bbox.attribute( "crs" );
        else
        {
          QgsDebugMsg( "crs of bounding box undefined" );
        }

        if ( !bb.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs;
          crs.createFromOgcWmsCrs( bb.crs );
          if ( crs.isValid() )
          {
            bb.crs = crs.authid();

            bool invert = !mParserSettings.ignoreAxisOrientation && crs.axisInverted();
            if ( mParserSettings.invertAxisOrientation )
              invert = !invert;

            if ( invert )
              bb.box.invert();

            l.boundingBoxes << bb;
          }
        }
      }
    }

    for ( QDomElement e1 = e0.firstChildElement( "Style" );
          !e1.isNull();
          e1 = e1.nextSiblingElement( "Style" ) )
    {
      QgsWmtsStyle s;
      s.identifier = e1.firstChildElement( "ows:Identifier" ).text();
      s.title      = e1.firstChildElement( "ows:Title" ).text();
      s.abstract   = e1.firstChildElement( "ows:Abstract" ).text();
      parseKeywords( e1, s.keywords );

      for ( QDomElement e2 = e1.firstChildElement( "ows:legendURL" );
            !e2.isNull();
            e2 = e2.nextSiblingElement( "ows:legendURL" ) )
      {
        QgsWmtsLegendURL u;

        u.format   = e2.firstChildElement( "format" ).text();
        u.minScale = e2.firstChildElement( "minScale" ).text().toDouble();
        u.maxScale = e2.firstChildElement( "maxScale" ).text().toDouble();
        u.href     = e2.firstChildElement( "href" ).text();
        u.width    = e2.firstChildElement( "width" ).text().toInt();
        u.height   = e2.firstChildElement( "height" ).text().toInt();

        s.legendURLs << u;
      }

      s.isDefault = e1.attribute( "isDefault" ) == "true";

      l.styles.insert( s.identifier, s );

      if ( s.isDefault )
        l.defaultStyle = s.identifier;
    }

    if ( l.styles.isEmpty() )
    {
      QgsWmtsStyle s;
      s.identifier = "default";
      s.title      = QObject::tr( "Generated default style" );
      s.abstract   = QObject::tr( "Style was missing in capabilities" );
      l.styles.insert( s.identifier, s );
    }

    for ( QDomElement e1 = e0.firstChildElement( "Format" ); !e1.isNull(); e1 = e1.nextSiblingElement( "Format" ) )
    {
      l.formats << e1.text();
    }

    for ( QDomElement e1 = e0.firstChildElement( "InfoFormat" ); !e1.isNull(); e1 = e1.nextSiblingElement( "InfoFormat" ) )
    {
      QString format = e1.text();

      l.infoFormats << e1.text();

      QgsRaster::IdentifyFormat fmt = QgsRaster::IdentifyFormatUndefined;

      QgsDebugMsg( QString( "format=%1" ).arg( format ) );

      if ( format == "MIME" )
        fmt = QgsRaster::IdentifyFormatText; // 1.0
      else if ( format == "text/plain" )
        fmt = QgsRaster::IdentifyFormatText;
      else if ( format == "text/html" )
        fmt = QgsRaster::IdentifyFormatHtml;
      else if ( format.startsWith( "GML." ) )
        fmt = QgsRaster::IdentifyFormatFeature; // 1.0
      else if ( format == "application/vnd.ogc.gml" )
        fmt = QgsRaster::IdentifyFormatFeature;
      else  if ( format.contains( "gml", Qt::CaseInsensitive ) )
        fmt = QgsRaster::IdentifyFormatFeature;
      else if ( format == "application/json" )
        fmt = QgsRaster::IdentifyFormatFeature;
      else
      {
        QgsDebugMsg( QString( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
        continue;
      }

      QgsDebugMsg( QString( "fmt=%1" ).arg( fmt ) );
      mIdentifyFormats.insert( fmt, format );
    }

    for ( QDomElement e1 = e0.firstChildElement( "Dimension" ); !e1.isNull(); e1 = e1.nextSiblingElement( "Dimension" ) )
    {
      QgsWmtsDimension d;

      d.identifier   = e1.firstChildElement( "ows:Identifier" ).text();
      if ( d.identifier.isEmpty() )
        continue;

      d.title        = e1.firstChildElement( "ows:Title" ).text();
      d.abstract     = e1.firstChildElement( "ows:Abstract" ).text();
      parseKeywords( e1, d.keywords );

      d.UOM          = e1.firstChildElement( "UOM" ).text();
      d.unitSymbol   = e1.firstChildElement( "unitSymbol" ).text();
      d.defaultValue = e1.firstChildElement( "Default" ).text();
      d.current      = e1.firstChildElement( "current" ).text() == "true";

      for ( QDomElement e2 = e1.firstChildElement( "Value" );
            !e2.isNull();
            e2 = e2.nextSiblingElement( "Value" ) )
      {
        d.values << e2.text();
      }

      l.dimensions.insert( d.identifier, d );
    }

    for ( QDomElement e1 = e0.firstChildElement( "TileMatrixSetLink" ); !e1.isNull(); e1 = e1.nextSiblingElement( "TileMatrixSetLink" ) )
    {
      QgsWmtsTileMatrixSetLink sl;

      sl.tileMatrixSet = e1.firstChildElement( "TileMatrixSet" ).text();

      if ( !mTileMatrixSets.contains( sl.tileMatrixSet ) )
      {
        QgsDebugMsg( QString( "  TileMatrixSet %1 not found." ).arg( sl.tileMatrixSet ) );
        continue;
      }

      const QgsWmtsTileMatrixSet &tms = mTileMatrixSets[ sl.tileMatrixSet ];

      for ( QDomElement e2 = e1.firstChildElement( "TileMatrixSetLimits" ); !e2.isNull(); e2 = e2.nextSiblingElement( "TileMatrixSetLimits" ) )
      {
        for ( QDomElement e3 = e2.firstChildElement( "TileMatrixLimits" ); !e3.isNull(); e3 = e3.nextSiblingElement( "TileMatrixLimits" ) )
        {
          QgsWmtsTileMatrixLimits limit;

          QString id = e3.firstChildElement( "TileMatrix" ).text();

          bool isValid = false;
          int matrixWidth = -1, matrixHeight = -1;
          foreach ( const QgsWmtsTileMatrix &m, tms.tileMatrices )
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
            limit.minTileRow = e3.firstChildElement( "MinTileRow" ).text().toInt();
            limit.maxTileRow = e3.firstChildElement( "MaxTileRow" ).text().toInt();
            limit.minTileCol = e3.firstChildElement( "MinTileCol" ).text().toInt();
            limit.maxTileCol = e3.firstChildElement( "MaxTileCol" ).text().toInt();

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
            QgsDebugMsg( QString( "   TileMatrix id:%1 not found." ).arg( id ) );
          }

          QgsDebugMsg( QString( "   TileMatrixLimit id:%1 row:%2-%3 col:%4-%5 matrix:%6x%7 %8" )
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

    for ( QDomElement e1 = e0.firstChildElement( "ResourceURL" ); !e1.isNull(); e1 = e1.nextSiblingElement( "ResourceURL" ) )
    {
      QString format       = nodeAttribute( e1, "format" );
      QString resourceType = nodeAttribute( e1, "resourceType" );
      QString tmpl         = nodeAttribute( e1, "template" );

      if ( format.isEmpty() || resourceType.isEmpty() || tmpl.isEmpty() )
      {
        QgsDebugMsg( QString( "SKIPPING ResourceURL format=%1 resourceType=%2 template=%3" )
                     .arg( format )
                     .arg( resourceType )
                     .arg( tmpl ) );
        continue;
      }

      if ( resourceType == "tile" )
      {
        l.getTileURLs.insert( format, tmpl );
      }
      else if ( resourceType == "FeatureInfo" )
      {
        l.getFeatureInfoURLs.insert( format, tmpl );

        QgsRaster::IdentifyFormat fmt = QgsRaster::IdentifyFormatUndefined;

        QgsDebugMsg( QString( "format=%1" ).arg( format ) );

        if ( format == "MIME" )
          fmt = QgsRaster::IdentifyFormatText; // 1.0
        else if ( format == "text/plain" )
          fmt = QgsRaster::IdentifyFormatText;
        else if ( format == "text/html" )
          fmt = QgsRaster::IdentifyFormatHtml;
        else if ( format.startsWith( "GML." ) )
          fmt = QgsRaster::IdentifyFormatFeature; // 1.0
        else if ( format == "application/vnd.ogc.gml" )
          fmt = QgsRaster::IdentifyFormatFeature;
        else  if ( format.contains( "gml", Qt::CaseInsensitive ) )
          fmt = QgsRaster::IdentifyFormatFeature;
        else if ( format == "application/json" )
          fmt = QgsRaster::IdentifyFormatFeature;
        else
        {
          QgsDebugMsg( QString( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
          continue;
        }

        QgsDebugMsg( QString( "fmt=%1" ).arg( fmt ) );
        mIdentifyFormats.insert( fmt, format );
      }
      else
      {
        QgsDebugMsg( QString( "UNEXPECTED resourceType in ResourcURL format=%1 resourceType=%2 template=%3" )
                     .arg( format )
                     .arg( resourceType )
                     .arg( tmpl ) );
      }
    }

    QgsDebugMsg( QString( "add layer %1" ).arg( id ) );
    mTileLayersSupported << l;
  }

  //
  // themes
  //
  mTileThemes.clear();
  for ( QDomElement e0 = e.firstChildElement( "Themes" ).firstChildElement( "Theme" );
        !e0.isNull();
        e0 = e0.nextSiblingElement( "Theme" ) )
  {
    mTileThemes << QgsWmtsTheme();
    parseTheme( e0, mTileThemes.back() );
  }

  // make sure that all layers have a bounding box
  for ( QList<QgsWmtsTileLayer>::iterator it = mTileLayersSupported.begin(); it != mTileLayersSupported.end(); ++it )
  {
    QgsWmtsTileLayer& l = *it;

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

  for ( QDomElement e1 = e.firstChildElement( "ows:Keywords" ).firstChildElement( "ows:Keyword" );
        !e1.isNull();
        e1 = e1.nextSiblingElement( "ows:Keyword" ) )
  {
    keywords << e1.text();
  }
}

void QgsWmsCapabilities::parseTheme( const QDomElement &e, QgsWmtsTheme &t )
{
  t.identifier = e.firstChildElement( "ows:Identifier" ).text();
  t.title      = e.firstChildElement( "ows:Title" ).text();
  t.abstract   = e.firstChildElement( "ows:Abstract" ).text();
  parseKeywords( e, t.keywords );

  QDomElement sl = e.firstChildElement( "ows:Theme" );
  if ( !sl.isNull() )
  {
    t.subTheme = new QgsWmtsTheme;
    parseTheme( sl, *t.subTheme );
  }
  else
  {
    t.subTheme = 0;
  }

  t.layerRefs.clear();
  for ( QDomElement e1 = e.firstChildElement( "ows:LayerRef" );
        !e1.isNull();
        e1 = e1.nextSiblingElement( "ows:LayerRef" ) )
  {
    t.layerRefs << e1.text();
  }
}

QString QgsWmsCapabilities::nodeAttribute( const QDomElement &e, QString name, QString defValue )
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


bool QgsWmsCapabilities::detectTileLayerBoundingBox( QgsWmtsTileLayer& l )
{
  if ( l.setLinks.isEmpty() )
    return false;

  // take first supported tile matrix set
  const QgsWmtsTileMatrixSetLink& setLink = l.setLinks.constBegin().value();

  QHash<QString, QgsWmtsTileMatrixSet>::const_iterator tmsIt = mTileMatrixSets.constFind( setLink.tileMatrixSet );
  if ( tmsIt == mTileMatrixSets.constEnd() )
    return false;

  QgsCoordinateReferenceSystem crs;
  if ( !crs.createFromOgcWmsCrs( tmsIt->crs ) )
    return false;

  // take most coarse tile matrix ...
  QMap<double, QgsWmtsTileMatrix>::const_iterator tmIt = tmsIt->tileMatrices.constEnd() - 1;
  if ( tmIt == tmsIt->tileMatrices.constEnd() )
    return false;

  const QgsWmtsTileMatrix& tm = *tmIt;
  double metersPerUnit = QGis::fromUnitToUnitFactor( crs.mapUnits(), QGis::Meters );
  double res = tm.scaleDenom * 0.00028 / metersPerUnit;
  QgsPoint bottomRight( tm.topLeft.x() + res * tm.tileWidth * tm.matrixWidth,
                        tm.topLeft.y() - res * tm.tileHeight * tm.matrixHeight );

  QgsDebugMsg( QString( "detecting WMTS layer bounding box: tileset %1 matrix %2 crs %3 res %4" )
               .arg( tmsIt->identifier ).arg( tm.identifier ).arg( tmsIt->crs ).arg( res ) );

  QgsRectangle extent( tm.topLeft, bottomRight );
  extent.normalize();

  QgsWmsBoundingBoxProperty bb;
  bb.box = extent;
  bb.crs = crs.authid();
  l.boundingBoxes << bb;

  return true;
}


bool QgsWmsCapabilities::shouldInvertAxisOrientation( const QString& ogcCrs )
{
  //according to the WMS spec for 1.3, some CRS have inverted axis
  bool changeXY = false;
  if ( !mParserSettings.ignoreAxisOrientation && ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" ) )
  {
    //have we already checked this crs?
    if ( mCrsInvertAxis.contains( ogcCrs ) )
    {
      //if so, return previous result to save time
      return mCrsInvertAxis[ ogcCrs ];
    }

    //create CRS from string
    QgsCoordinateReferenceSystem theSrs;
    if ( theSrs.createFromOgcWmsCrs( ogcCrs ) && theSrs.axisInverted() )
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

  foreach ( QgsRaster::IdentifyFormat f, mIdentifyFormats.keys() )
  {
    capability |= QgsRasterDataProvider::identifyFormatToCapability( f );
  }

  return capability;
}



// -----------------


QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( const QString& baseUrl, const QgsWmsAuthorization& auth, QObject *parent )
    : QObject( parent )
    , mBaseUrl( baseUrl )
    , mAuth( auth )
    , mCapabilitiesReply( NULL )
{
}


bool QgsWmsCapabilitiesDownload::downloadCapabilities()
{
  QgsDebugMsg( "entering." );

  QString url = mBaseUrl;
  QgsDebugMsg( "url = " + url );
  if ( !url.contains( "SERVICE=WMTS" ) &&
       !url.contains( "/WMTSCapabilities.xml" ) )
  {
    url += "SERVICE=WMS&REQUEST=GetCapabilities";
  }

  mError.clear();

  QNetworkRequest request( url );
  mAuth.setAuthorization( request );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QgsDebugMsg( QString( "getcapabilities: %1" ).arg( url ) );
  // This is causing Qt warning: "Cannot create children for a parent that is in a different thread."
  // but it only means that the reply will have no parent
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

  connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ), Qt::DirectConnection );
  connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ), Qt::DirectConnection );

  QEventLoop loop;
  connect( this, SIGNAL( downloadFinished() ), &loop, SLOT( quit() ) );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mError.isEmpty();
}



void QgsWmsCapabilitiesDownload::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyFinished()
{
  QgsDebugMsg( "entering." );
  if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
  {
    QgsDebugMsg( "reply ok" );
    QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      emit statusChanged( tr( "Capabilities request redirected." ) );

      const QUrl& toUrl = redirect.toUrl();
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
        mAuth.setAuthorization( request );
        request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
        request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

        mCapabilitiesReply->deleteLater();
        QgsDebugMsg( QString( "redirected getcapabilities: %1" ).arg( redirect.toString() ) );
        mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

        connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
        connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
        return;
      }
    }
    else
    {
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

  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = 0;

  emit downloadFinished();
}
