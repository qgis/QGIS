/***************************************************************************
  qgswmsprovider.cpp  -  QGIS Data provider for
                         OGC Web Map Service layers
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id$ */

#include "qgslogger.h"
#include "qgswmsprovider.h"

#include <math.h>

#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"

#include "qgshttptransaction.h"

#include <QUrl>
#include <QImage>
#include <QSet>
#include <QSettings>

#ifdef _MSC_VER
#include <float.h>
#define isfinite(x) _finite(x)
#endif

#ifdef QGISDEBUG
#include <QFile>
#endif

static QString WMS_KEY = "wms";
static QString WMS_DESCRIPTION = "OGC Web Map Service version 1.3 data provider";

static QString DEFAULT_LATLON_CRS = "CRS:84";


QgsWmsProvider::QgsWmsProvider( QString const & uri )
    : QgsRasterDataProvider( uri ),
    httpuri( uri ),
    httpcapabilitiesresponse( 0 ),
    imageCrs( DEFAULT_LATLON_CRS ),
    cachedImage( 0 ),
    cachedViewExtent( 0 ),
    cachedPixelWidth( 0 ),
    cachedPixelHeight( 0 ),
    mCoordinateTransform( 0 ),
    extentDirty( TRUE ),
    mGetFeatureInfoUrlBase( 0 ),
    mLayerCount( -1 )

{
  // URL may contain username/password information for a WMS
  // requiring authentication. In this case the URL is prefixed
  // with username=user,password=pass,url=http://xxx.xxx.xx/yyy...
  mUserName = "";
  mPassword = "";
  setAuthentication( httpuri );

  QgsDebugMsg( "QgsWmsProvider: constructing with uri '" + httpuri + "'." );

  // assume this is a valid layer until we determine otherwise
  valid = true;


  // URL can be in 3 forms:
  // 1) http://xxx.xxx.xx/yyy/yyy
  // 2) http://xxx.xxx.xx/yyy/yyy?
  // 3) http://xxx.xxx.xx/yyy/yyy?zzz=www

  baseUrl = prepareUri( httpuri );

  QgsDebugMsg( "baseUrl = " + baseUrl );

//  getServerCapabilities();

  //httpuri = "http://www.ga.gov.au/bin/getmap.pl?dataset=national&Service=WMS&Version=1.1.0&Request=GetMap&"
  //      "BBox=130,-40,160,-10&CRS=EPSG:4326&Width=400&Height=400&Layers=railways&Format=image/png";

  //httpuri = "http://www.ga.gov.au/bin/getmap.pl?dataset=national&";

  /*
    // 302-redirects to:
    uri = "http://www.ga.gov.au/bin/mapserv40?"
          "map=/public/http/www/docs/map/national/national.map&"
          "map_logo_status=off&map_coast_status=off&Service=WMS&Version=1.1.0&"
          "Request=GetMap&BBox=130,-40,160,-10&CRS=EPSG:4326&Width=800&"
          "Height=800&Layers=railways&Format=image/png";
  */

//  downloadMapURI(uri);


  QgsDebugMsg( "QgsWmsProvider: exiting constructor." );
}

void QgsWmsProvider::setAuthentication( QString uri )
{
  // Strip off and store the user name and password (if they exist)
  if ( ! uri.startsWith( " http:" ) )
  {
    // uri potentially contains username and password
    QStringList parts = uri.split( "," );
    QStringListIterator iter( parts );
    while ( iter.hasNext() )
    {
      QString item = iter.next();
      QgsDebugMsg( "QgsWmsProvider: Testing for creds: " + item );
      if ( item.startsWith( "username=" ) )
      {
        mUserName = item.mid( 9 );
        QgsDebugMsg( "QgsWmsProvider: Set username to " + mUserName );
      }
      else if ( item.startsWith( "password=" ) )
      {
        mPassword = item.mid( 9 );
        QgsDebugMsg( "QgsWmsProvider: Set password to " + mPassword );
      }
      else if ( item.startsWith( "url=" ) )
      {
        // strip the authentication information from the front of the uri
        httpuri = item.mid( 4 );
        QgsDebugMsg( "QgsWmsProvider: Set httpuri to " + httpuri );
      }
    }

  }

}
QString QgsWmsProvider::prepareUri( QString uri )
{
  if ( !( uri.contains( "?" ) ) )
  {
    uri.append( "?" );
  }
  else if (
    ( uri.right( 1 ) != "?" ) &&
    ( uri.right( 1 ) != "&" )
  )
  {
    uri.append( "&" );
  }

  return uri;
}

QgsWmsProvider::~QgsWmsProvider()
{
  QgsDebugMsg( "QgsWmsProvider: deconstructing." );

  // Dispose of any cached image as created by draw()
  if ( cachedImage )
  {
    delete cachedImage;
  }

  if ( mCoordinateTransform )
  {
    delete mCoordinateTransform;
  }

}



bool QgsWmsProvider::supportedLayers( QVector<QgsWmsLayerProperty> &layers )
{
  QgsDebugMsg( "Entering." );

  // Allow the provider to collect the capabilities first.
  if ( !retrieveServerCapabilities() )
  {
    return FALSE;
  }

  layers = layersSupported;

  QgsDebugMsg( "Exiting." );

  return TRUE;
}


QSet<QString> QgsWmsProvider::supportedCrsForLayers( QStringList const & layers )
{
  QSet<QString> crsCandidates;

  QStringList::const_iterator i;
  for ( i = layers.constBegin(); i != layers.constEnd(); ++i )
  {
    QVector<QString> crsVector = crsForLayer[*i];
    QSet<QString>    crsSet;

    // convert std::vector to std::set for set comparisons
    for ( int j = 0; j < crsVector.size(); j++ )
    {
      crsSet.insert( crsVector[j] );
    }

    // first time through?
    if ( i == layers.constBegin() )
    {
      // do initial population of set
      crsCandidates = crsSet;
    }
    else
    {
      // do lowest common denominator (set intersection)
      crsCandidates.intersect( crsSet );
    }

  }

  return crsCandidates;
}


size_t QgsWmsProvider::layerCount() const
{
  return 1;                   // XXX properly return actual number of layers
} // QgsWmsProvider::layerCount()


void QgsWmsProvider::addLayers( QStringList const &layers,
                                QStringList const &styles )
{
  QgsDebugMsg( "Entering with layer list of " + layers.join( ", " )
               + " and style list of " + styles.join( ", " ) );

  // TODO: Make activeSubLayers a std::map in order to avoid duplicates

  activeSubLayers += layers;
  activeSubStyles += styles;

  // Set the visibility of these new layers on by default
  for ( QStringList::const_iterator it  = layers.begin();
        it != layers.end();
        ++it )
  {
    activeSubLayerVisibility[*it] = TRUE;

    QgsDebugMsg( "set visibility of layer '" + ( *it ) + "' to TRUE." );
  }

  // now that the layers have changed, the extent will as well.
  extentDirty = TRUE;

  QgsDebugMsg( "Exiting." );
}

void QgsWmsProvider::setConnectionName( QString const &connName )
{
  connectionName = connName;
}

void QgsWmsProvider::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsg( "Entering." );

  activeSubLayers = layers;

  QgsDebugMsg( "Exiting." );
}


void QgsWmsProvider::setSubLayerVisibility( QString const & name, bool vis )
{
  activeSubLayerVisibility[name] = vis;
}


QString QgsWmsProvider::imageEncoding() const
{
  return imageMimeType;
}


void QgsWmsProvider::setImageEncoding( QString const & mimeType )
{
  QgsDebugMsg( "Setting image encoding to " + mimeType + "." );
  imageMimeType = mimeType;
}


void QgsWmsProvider::setImageCrs( QString const & crs )
{
  QgsDebugMsg( "Setting image CRS to " + crs + "." );

  if (
    ( crs != imageCrs )
    &&
    ( ! crs.isEmpty() )
  )
  {
    // delete old coordinate transform as it is no longer valid
    if ( mCoordinateTransform )
    {
      delete mCoordinateTransform;
    }

    extentDirty = TRUE;

    imageCrs = crs;
  }

}

QImage* QgsWmsProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  QgsDebugMsg( "Entering." );

  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  // Can we reuse the previously cached image?
  if (
    ( cachedImage ) &&
    ( cachedViewExtent == viewExtent ) &&
    ( cachedPixelWidth == pixelWidth ) &&
    ( cachedPixelHeight == pixelHeight )
  )
  {
    return cachedImage;
  }

  // Bounding box in WMS format
  QString bbox;

  //according to the WMS spec for 1.3, the order of x - and y - coordinates is inverted for geographical CRS
  bool changeXY = false;
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    //create CRS from string
    bool conversionOk;
    int epsgNr = imageCrs.section( ":", 1, 1 ).toInt( &conversionOk );
    if ( conversionOk )
    {
      QgsCoordinateReferenceSystem theSrs;
      theSrs.createFromEpsg( epsgNr );
      if ( theSrs.geographicFlag() )
      {
        changeXY = true;
      }
    }
  }


  // Warning: does not work with scientific notation
  if ( changeXY )
  {
    bbox = QString( "%1,%2,%3,%4" ).
           arg( viewExtent.yMinimum(), 0, 'f' ).
           arg( viewExtent.xMinimum(), 0, 'f' ).
           arg( viewExtent.yMaximum(), 0, 'f' ).
           arg( viewExtent.xMaximum(), 0, 'f' );
  }
  else
  {
    bbox = QString( "%1,%2,%3,%4" ).
           arg( viewExtent.xMinimum(), 0, 'f' ).
           arg( viewExtent.yMinimum(), 0, 'f' ).
           arg( viewExtent.xMaximum(), 0, 'f' ).
           arg( viewExtent.yMaximum(), 0, 'f' );
  }

  // Width in WMS format
  QString width;
  width = width.setNum( pixelWidth );

  // Height in WMS format
  QString height;
  height = height.setNum( pixelHeight );

  // Calculate active layers that are also visible.

  QgsDebugMsg( "Active layer list of "  + activeSubLayers.join( ", " )
               + " and style list of "  + activeSubStyles.join( ", " ) );

  QStringList visibleLayers = QStringList();
  QStringList visibleStyles = QStringList();

  QStringList::Iterator it2  = activeSubStyles.begin();

  for ( QStringList::Iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    if ( activeSubLayerVisibility.find( *it ).value() )
    {
      visibleLayers += *it;
      visibleStyles += *it2;
    }

    ++it2;
  }

  QgsDebugMsg( "Visible layer list of " + visibleLayers.join( ", " )
               + " and style list of "  + visibleStyles.join( ", " ) );

  QString layers = QUrl::toPercentEncoding( visibleLayers.join( "," ) );
  QString styles = QUrl::toPercentEncoding( visibleStyles.join( "," ) );

  // compose the URL query string for the WMS server.

  QString crsKey = "SRS"; //SRS in 1.1.1 and CRS in 1.3.0
  if ( mCapabilities.version == "1.3.0" || mCapabilities.version == "1.3" )
  {
    crsKey = "CRS";
  }

  QString url;
  QVector<QgsWmsDcpTypeProperty> dcpType = mCapabilities.capability.request.getMap.dcpType;
  if ( dcpType.size() < 1 )
  {
    url = baseUrl;
  }
  else
  {
    url = prepareUri( dcpType.front().http.get.onlineResource.xlinkHref );
  }

  url += "SERVICE=WMS";
  url += "&";
  url += "VERSION=" + mCapabilities.version;
  url += "&";
  url += "REQUEST=GetMap";
  url += "&";
  url += "BBOX=" + bbox;
  url += "&";
  url += crsKey + "=" + imageCrs;
  url += "&";
  url += "WIDTH=" + width;
  url += "&";
  url += "HEIGHT=" + height;
  url += "&";
  url += "LAYERS=" + layers;
  url += "&";
  url += "STYLES=" + styles;
  url += "&";
  url += "FORMAT=" + imageMimeType;

  //DPI parameter is accepted by QGIS mapserver (and ignored by the other WMS servers)
  if ( mDpi != -1 )
  {
    url += "&DPI=" + QString::number( mDpi );
  }

  //MH: jpeg does not support transparency and some servers complain if jpg and transparent=true
  if ( !imageMimeType.contains( "jpeg", Qt::CaseInsensitive ) && !imageMimeType.contains( "jpg", Qt::CaseInsensitive ) )
  {
    url += "&";
    url += "TRANSPARENT=TRUE";
  }

  dcpType = mCapabilities.capability.request.getFeatureInfo.dcpType;
  if ( dcpType.size() < 1 )
  {
    mGetFeatureInfoUrlBase = baseUrl;
  }
  else
  {
    mGetFeatureInfoUrlBase = prepareUri( dcpType.front().http.get.onlineResource.xlinkHref );
  }

  // cache some details for if the user wants to do an identifyAsHtml() later
  mGetFeatureInfoUrlBase += "SERVICE=WMS";
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "VERSION=" + mCapabilities.version;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "REQUEST=GetFeatureInfo";
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "BBOX=" + bbox;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += crsKey + "=" + imageCrs;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "WIDTH=" + width;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "HEIGHT=" + height;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "LAYERS=" + layers;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "STYLES=" + styles;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "FORMAT=" + imageMimeType;

  if ( !imageMimeType.contains( "jpeg", Qt::CaseInsensitive ) && !imageMimeType.contains( "jpg", Qt::CaseInsensitive ) )
  {
    mGetFeatureInfoUrlBase += "&";
    mGetFeatureInfoUrlBase += "TRANSPARENT=TRUE";
  }

  QByteArray imagesource;
  imagesource = retrieveUrl( url );

  if ( imagesource.isEmpty() )
  {
    return 0;
  }

#if 0
  QgsHttpTransaction http( url, httpproxyhost, httpproxyport );

  // Do a passthrough for the status bar text
  connect(
    &http, SIGNAL( statusChanged( QString ) ),
    this,   SLOT( showStatusMessage( QString ) )
  );

  bool httpOk;

  httpOk = http.getSynchronously( imagesource );

  if ( !httpOk )
  {
    // We had an HTTP exception

    mErrorCaption = tr( "HTTP Exception" );
    mError = http.errorString();

    mError += "\n" + tr( "Tried URL: " ) + url;

    return 0;
  }

  if ( http.responseContentType() == "application/vnd.ogc.se_xml" )
  {
    // We had a Service Exception from the WMS

    QgsDebugMsg( "got Service Exception as:\n"  + QString( imagesource ) );

    mErrorCaption = tr( "WMS Service Exception" );

    // set mError with the following:
    parseServiceExceptionReportDom( imagesource );

    mError += "\n" + tr( "Tried URL: " ) + url;

    QgsDebugMsg( "composed error message '" + mError + "'." );

    return 0;
  }
#endif

  QgsDebugMsg( "Response received." );

#ifdef QGISDEBUG
  //QFile file( "/tmp/qgis-wmsprovider-draw-raw.png" );
  //if ( file.open( QIODevice::WriteOnly ) )
  //{
  //  file.writeBlock(imagesource);
  //  file.close();
  //}
#endif

  // Load into the final QImage.

  //bool success = i.loadFromData(imagesource);
  if ( cachedImage )
  {
    delete cachedImage;
  }

  //Create a local image from source then convert it to RGBA, so we can set the transparency later
  QImage myLocalImage = QImage::fromData( imagesource );
  cachedImage = new QImage( myLocalImage.convertToFormat( QImage::Format_ARGB32 ) );

  // Remember settings for useful caching next time.
  cachedViewExtent = viewExtent;
  cachedPixelWidth = pixelWidth;
  cachedPixelHeight = pixelHeight;

#ifdef QGISDEBUG
  // Get what we can support

// Commented out for now, causes segfaults.
//  supportedFormats();

#endif



  QgsDebugMsg( "Exiting." );


  // TODO: bit depth conversion to the client's expectation
//  return *(i.convertDepth(32));
  return cachedImage;

}

#if 0
void QgsWmsProvider::getServerCapabilities()
{
  QgsDebugMsg( "entering." );

  retrieveServerCapabilities();

  // TODO: Return generic server capabilities here

  QgsDebugMsg( "exiting." );

}
#endif

bool QgsWmsProvider::retrieveServerCapabilities( bool forceRefresh )
{
  QgsDebugMsg( "entering." );

  if ( httpcapabilitiesresponse.isNull() ||
       forceRefresh )
  {

    QString url = baseUrl + "SERVICE=WMS&REQUEST=GetCapabilities";

    httpcapabilitiesresponse = retrieveUrl( url );

    if ( httpcapabilitiesresponse.isEmpty() )
    {
      return FALSE;
    }

#if 0
    QgsHttpTransaction http( url, httpproxyhost, httpproxyport );

    // Do a passthrough for the status bar text
    connect(
      &http, SIGNAL( statusChanged( QString ) ),
      this,   SLOT( showStatusMessage( QString ) )
    );

    bool httpOk;
    httpOk = http.getSynchronously( httpcapabilitiesresponse );

    if ( !httpOk )
    {
      // We had an HTTP exception

      mErrorCaption = tr( "HTTP Exception" );
      mError = http.errorString();

      mError += "\n" + tr( "Tried URL: " ) + url;

      QgsDebugMsg( "!httpOK: "  + mError );

      return FALSE;
    }
#endif

    QgsDebugMsg( "Converting to Dom." );

    bool domOK;
    domOK = parseCapabilitiesDom( httpcapabilitiesresponse, mCapabilities );

    if ( !domOK )
    {
      // We had an Dom exception -
      // mErrorCaption and mError are pre-filled by parseCapabilitiesDom

      mError += tr( "\nTried URL: %1" ).arg( url );

      QgsDebugMsg( "!domOK: " + mError );

      return FALSE;
    }

  }

  QgsDebugMsg( "exiting." );

  return TRUE;

}


QByteArray QgsWmsProvider::retrieveUrl( QString url )
{
  QgsDebugMsg( "WMS request Url: " + url );
  QgsHttpTransaction http( url );
  QgsDebugMsg( "Setting creds: " + mUserName + "/" + mPassword );
  http.setCredentials( mUserName, mPassword );

  // Do a passthrough for the status bar text
  connect(
    &http, SIGNAL( statusChanged( QString ) ),
    this,   SLOT( showStatusMessage( QString ) )
  );

  QByteArray httpResponse;
  bool httpOk;

  httpOk = http.getSynchronously( httpResponse );

  if ( !httpOk )
  {
    // We had an HTTP exception

    mErrorCaption = tr( "HTTP Exception" );
    mError = http.errorString();

    mError += tr( "\nTried URL: %1" ).arg( url );

    return QByteArray( "" );
  }

  if ( http.responseContentType() == "application/vnd.ogc.se_xml" )
  {
    // We had a Service Exception from the WMS

    QgsDebugMsg( "got Service Exception as:\n" + httpResponse );

    mErrorCaption = tr( "WMS Service Exception" );

    // set mError with the following:
    parseServiceExceptionReportDom( httpResponse );

    mError += tr( "\nTried URL: %1" ).arg( url );

    QgsDebugMsg( "composed error message '" + mError + "'." );

    return QByteArray( "" );
  }

  return httpResponse;
}

bool QgsWmsProvider::parseCapabilitiesDom( QByteArray const & xml, QgsWmsCapabilitiesProperty& capabilitiesProperty )
{
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  //test the content of the QByteArray.
  // There is a bug in Qt4.1.2, due for fixing in 4.2.0, where
  // QString(QByteArray) uses strlen to find the length of the
  // QByteArray. However, there are no guarantees that the QByteArray
  // has a terminating \0, and hence this can cause a crash, so we supply
  // the qbytearray to qstring in a way that ensures that there is a
  // terminating \0, and just to be safe, we also give it the actual
  // size.

  // Also, the Qt qWarning() function has a limit of 8192 bytes per
  // message, which can easily be exceeded by wms capability
  // documents, so we use the qgs logger stuff instead which doesn't
  // have that limitation.

  QString responsestring( QString::fromAscii( xml.constData(), xml.size() ) );
  QgsLogger::debug( "QgsWmsProvider::parseCapabilitiesDom, received the following data: " + responsestring );

  //QFile file( "/tmp/qgis-wmsprovider-capabilities.xml" );
  //if ( file.open( QIODevice::WriteOnly ) )
  //{
  //  file.writeBlock(xml);
  //  file.close();
  //}
#endif

  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = capabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = tr( "Dom Exception" );
    mError = tr( "Could not get WMS capabilities: %1 at line %2 column %3\n" )
             .arg( errorMsg ).arg( errorLine ).arg( errorColumn )
             + tr( "This is probably due to an incorrect WMS Server URL." );

    QgsLogger::debug( "Dom Exception: " + mError );

    return FALSE;
  }

  QDomElement docElem = capabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WMS Capabilities document)
  QgsDebugMsg( "testing tagName " + docElem.tagName() );

  if ( !
       (
         ( docElem.tagName() == "WMS_Capabilities" )   // (1.3 vintage)
         ||
         ( docElem.tagName() == "WMT_MS_Capabilities" )  // (1.1.1 vintage)
       )
     )
  {
    mErrorCaption = tr( "Dom Exception" );
    mError = tr( "Could not get WMS capabilities in the "
                 "expected format (DTD): no %1 or %2 found\n" )
             .arg( "WMS_Capabilities" ).arg( "WMT_MS_Capabilities" )
             + tr( "This is probably due to an incorrect WMS Server URL." );

    QgsLogger::debug( "Dom Exception: " + mError );

    return FALSE;
  }

  capabilitiesProperty.version = docElem.attribute( "version" );

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      //QgsDebugMsg(e.tagName() ); // the node really is an element.

      if ( e.tagName() == "Service" )
      {
        QgsDebugMsg( "  Service." );
        parseService( e, capabilitiesProperty.service );
      }
      else if ( e.tagName() == "Capability" )
      {
        QgsDebugMsg( "  Capability." );
        parseCapability( e, capabilitiesProperty.capability );
      }

    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return TRUE;
}


void QgsWmsProvider::parseService( QDomElement const & e, QgsWmsServiceProperty& serviceProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      // QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Title" )
      {
        serviceProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        serviceProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "KeywordList" )
      {
        parseKeywordList( e1, serviceProperty.keywordList );
      }
      else if ( e1.tagName() == "OnlineResource" )
      {
        parseOnlineResource( e1, serviceProperty.onlineResource );
      }
      else if ( e1.tagName() == "ContactInformation" )
      {
        parseContactInformation( e1, serviceProperty.contactInformation );
      }
      else if ( e1.tagName() == "Fees" )
      {
        serviceProperty.fees = e1.text();
      }
      else if ( e1.tagName() == "AccessConstraints" )
      {
        serviceProperty.accessConstraints = e1.text();
      }
      else if ( e1.tagName() == "LayerLimit" )
      {
        serviceProperty.layerLimit = e1.text().toUInt();
      }
      else if ( e1.tagName() == "MaxWidth" )
      {
        serviceProperty.maxWidth = e1.text().toUInt();
      }
      else if ( e1.tagName() == "MaxHeight" )
      {
        serviceProperty.maxHeight = e1.text().toUInt();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseCapability( QDomElement const & e, QgsWmsCapabilityProperty& capabilityProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      QgsDebugMsg( "  "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Request" )
      {
        parseRequest( e1, capabilityProperty.request );
      }
      else if ( e1.tagName() == "Layer" )
      {
        parseLayer( e1, capabilityProperty.layer );
      }

    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactPersonPrimary( QDomElement const & e, QgsWmsContactPersonPrimaryProperty& contactPersonPrimaryProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "ContactPerson" )
      {
        contactPersonPrimaryProperty.contactPerson = e1.text();
      }
      else if ( e1.tagName() == "ContactOrganization" )
      {
        contactPersonPrimaryProperty.contactOrganization = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactAddress( QDomElement const & e, QgsWmsContactAddressProperty& contactAddressProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "AddressType" )
      {
        contactAddressProperty.addressType = e1.text();
      }
      else if ( e1.tagName() == "Address" )
      {
        contactAddressProperty.address = e1.text();
      }
      else if ( e1.tagName() == "City" )
      {
        contactAddressProperty.city = e1.text();
      }
      else if ( e1.tagName() == "StateOrProvince" )
      {
        contactAddressProperty.stateOrProvince = e1.text();
      }
      else if ( e1.tagName() == "PostCode" )
      {
        contactAddressProperty.postCode = e1.text();
      }
      else if ( e1.tagName() == "Country" )
      {
        contactAddressProperty.country = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseContactInformation( QDomElement const & e, QgsWmsContactInformationProperty& contactInformationProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "ContactPersonPrimary" )
      {
        parseContactPersonPrimary( e1, contactInformationProperty.contactPersonPrimary );
      }
      else if ( e1.tagName() == "ContactPosition" )
      {
        contactInformationProperty.contactPosition = e1.text();
      }
      else if ( e1.tagName() == "ContactAddress" )
      {
        parseContactAddress( e1, contactInformationProperty.contactAddress );
      }
      else if ( e1.tagName() == "ContactVoiceTelephone" )
      {
        contactInformationProperty.contactVoiceTelephone = e1.text();
      }
      else if ( e1.tagName() == "ContactFacsimileTelephone" )
      {
        contactInformationProperty.contactFacsimileTelephone = e1.text();
      }
      else if ( e1.tagName() == "ContactElectronicMailAddress" )
      {
        contactInformationProperty.contactElectronicMailAddress = e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseOnlineResource( QDomElement const & e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute )
{
  QgsDebugMsg( "entering." );

  onlineResourceAttribute.xlinkHref = e.attribute( "xlink:href" );

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseKeywordList( QDomElement  const & e, QStringList& keywordListProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "Keyword" )
      {
        QgsDebugMsg( "      Keyword." );
        keywordListProperty += e1.text();
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseGet( QDomElement const & e, QgsWmsGetProperty& getProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, getProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parsePost( QDomElement const & e, QgsWmsPostProperty& postProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "OnlineResource" )
      {
        QgsDebugMsg( "      OnlineResource." );
        parseOnlineResource( e1, postProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseHttp( QDomElement const & e, QgsWmsHttpProperty& httpProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "Get" )
      {
        QgsDebugMsg( "      Get." );
        parseGet( e1, httpProperty.get );
      }
      else if ( e1.tagName() == "Post" )
      {
        QgsDebugMsg( "      Post." );
        parsePost( e1, httpProperty.post );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseDcpType( QDomElement const & e, QgsWmsDcpTypeProperty& dcpType )
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


void QgsWmsProvider::parseOperationType( QDomElement const & e, QgsWmsOperationType& operationType )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "Format" )
      {
        QgsDebugMsg( "      Format." );
        operationType.format += e1.text();
      }
      else if ( e1.tagName() == "DCPType" )
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


void QgsWmsProvider::parseRequest( QDomElement const & e, QgsWmsRequestProperty& requestProperty )
{
  QgsDebugMsg( "entering." );

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "GetMap" )
      {
        QgsDebugMsg( "      GetMap." );
        parseOperationType( e1, requestProperty.getMap );
      }
      else if ( e1.tagName() == "GetFeatureInfo" )
      {
        QgsDebugMsg( "      GetFeatureInfo." );
        parseOperationType( e1, requestProperty.getFeatureInfo );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseLegendUrl( QDomElement const & e, QgsWmsLegendUrlProperty& legendUrlProperty )
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
      if ( e1.tagName() == "Format" )
      {
        legendUrlProperty.format = e1.text();
      }
      else if ( e1.tagName() == "OnlineResource" )
      {
        parseOnlineResource( e1, legendUrlProperty.onlineResource );
      }
    }
    n1 = n1.nextSibling();
  }

  QgsDebugMsg( "exiting." );
}


void QgsWmsProvider::parseStyle( QDomElement const & e, QgsWmsStyleProperty& styleProperty )
{
//  QgsDebugMsg("entering.");

  QDomNode n1 = e.firstChild();
  while ( !n1.isNull() )
  {
    QDomElement e1 = n1.toElement(); // try to convert the node to an element.
    if ( !e1.isNull() )
    {
      if ( e1.tagName() == "Name" )
      {
        styleProperty.name = e1.text();
      }
      else if ( e1.tagName() == "Title" )
      {
        styleProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        styleProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "LegendURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "StyleSheetURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "StyleURL" )
      {
        // TODO
      }
    }
    n1 = n1.nextSibling();
  }

//  QgsDebugMsg("exiting.");
}


void QgsWmsProvider::parseLayer( QDomElement const & e, QgsWmsLayerProperty& layerProperty,
                                 QgsWmsLayerProperty *parentProperty )
{
//  QgsDebugMsg("entering.");

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
      QgsDebugMsg( "    "  + e1.tagName() ); // the node really is an element.

      if ( e1.tagName() == "Layer" )
      {
        QgsDebugMsg( "      Nested layer." );

        QgsWmsLayerProperty subLayerProperty;

        // Inherit things into the sublayer
        //   Ref: 7.2.4.8 Inheritance of layer properties
        subLayerProperty.style                    = layerProperty.style;
        subLayerProperty.crs                      = layerProperty.crs;
        subLayerProperty.boundingBox              = layerProperty.boundingBox;
        subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
        // TODO

        parseLayer( e1, subLayerProperty, &layerProperty );

        layerProperty.layer.push_back( subLayerProperty );
      }
      else if ( e1.tagName() == "Name" )
      {
        layerProperty.name = e1.text();
      }
      else if ( e1.tagName() == "Title" )
      {
        layerProperty.title = e1.text();
      }
      else if ( e1.tagName() == "Abstract" )
      {
        layerProperty.abstract = e1.text();
      }
      else if ( e1.tagName() == "KeywordList" )
      {
        parseKeywordList( e1, layerProperty.keywordList );
      }
      else if ( e1.tagName() == "CRS" )
      {
        layerProperty.crs.push_back( e1.text() );
      }
      else if ( e1.tagName() == "SRS" )      // legacy from earlier versions of WMS
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        QStringList srsList = e1.text().split( QRegExp( "\\s+" ) );

        QStringList::const_iterator i;
        for ( i = srsList.constBegin(); i != srsList.constEnd(); ++i )
        {
          layerProperty.crs.push_back( *i );
        }
      }
      else if ( e1.tagName() == "LatLonBoundingBox" )      // legacy from earlier versions of WMS
      {
#if 0
        QgsDebugMsg( "      LLBB is: '"  + e1.attribute( "minx" )  + "'." );
        QgsDebugMsg( "      LLBB is: '"  + e1.attribute( "miny" )  + "'." );
        QgsDebugMsg( "      LLBB is: '"  + e1.attribute( "maxx" )  + "'." );
        QgsDebugMsg( "      LLBB is: '"  + e1.attribute( "maxy" )  + "'." );
#endif

        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              e1.attribute( "minx" ).toDouble(),
              e1.attribute( "miny" ).toDouble(),
              e1.attribute( "maxx" ).toDouble(),
              e1.attribute( "maxy" ).toDouble()
            );
      }
      else if ( e1.tagName() == "EX_GeographicBoundingBox" ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem = n1.namedItem( "westBoundLongitude" ).toElement();
        QDomElement eBoundLongitudeElem = n1.namedItem( "eastBoundLongitude" ).toElement();
        QDomElement sBoundLatitudeElem = n1.namedItem( "southBoundLatitude" ).toElement();
        QDomElement nBoundLatitudeElem = n1.namedItem( "northBoundLatitude" ).toElement();
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
      else if ( e1.tagName() == "BoundingBox" )
      {
        // TODO: overwrite inherited
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( e1.attribute( "minx" ).toDouble(),
                                 e1.attribute( "miny" ).toDouble(),
                                 e1.attribute( "maxx" ).toDouble(),
                                 e1.attribute( "maxy" ).toDouble()
                               );
        bbox.crs = e1.attribute( "CRS" );
        layerProperty.boundingBox.push_back( bbox );
      }
      else if ( e1.tagName() == "Dimension" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Attribution" )
      {
        // TODO
      }
      else if ( e1.tagName() == "AuthorityURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Identifier" )
      {
        // TODO
      }
      else if ( e1.tagName() == "MetadataURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "DataURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "FeatureListURL" )
      {
        // TODO
      }
      else if ( e1.tagName() == "Style" )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( e1, styleProperty );

        layerProperty.style.push_back( styleProperty );
      }
      else if ( e1.tagName() == "MinScaleDenominator" )
      {
        // TODO
      }
      else if ( e1.tagName() == "MaxScaleDenominator" )
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

  if ( layerProperty.layer.empty() )
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere

    // Store if the layer is queryable
    mQueryableForLayer[ layerProperty.name ] = layerProperty.queryable;

    // Store the available Coordinate Reference Systems for the layer so that it
    // can be combined with others later in supportedCrsForLayers()
    crsForLayer[ layerProperty.name ] = layerProperty.crs;

    // Store the WGS84 (CRS:84) extent so that it can be combined with others later
    // in calculateExtent()

    // Apply the coarse bounding box first
    extentForLayer[ layerProperty.name ] = layerProperty.ex_GeographicBoundingBox;

    // see if we can refine the bounding box with the CRS-specific bounding boxes
    for ( int i = 0; i < layerProperty.boundingBox.size(); i++ )
    {
      QgsDebugMsg( "testing bounding box CRS which is "
                   + layerProperty.boundingBox[i].crs + "." );

      if ( layerProperty.boundingBox[i].crs == DEFAULT_LATLON_CRS )
      {
        extentForLayer[ layerProperty.name ] =
          layerProperty.boundingBox[i].box;
      }
    }

    QgsDebugMsg( "extent for "
                 + layerProperty.name  + " is "
                 + extentForLayer[ layerProperty.name ].toString( 3 )  + "." );

    // Insert into the local class' registry
    layersSupported.push_back( layerProperty );

    //if there are several <Layer> elements without a parent layer, the style list needs to be cleared
    if ( layerProperty.layer.empty() )
    {
      layerProperty.style.clear();
    }
  }
  else
  {
    mLayerParentNames[ layerProperty.orderId ] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
  }

//  QgsDebugMsg("exiting.");
}

void QgsWmsProvider::layerParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const
{
  parents = mLayerParents;
  parentNames = mLayerParentNames;
}

bool QgsWmsProvider::parseServiceExceptionReportDom( QByteArray const & xml )
{
  QgsDebugMsg( "entering." );

#ifdef QGISDEBUG
  //test the content of the QByteArray
  QString responsestring( xml );
  QgsDebugMsg( "received the following data: " + responsestring );
#endif

  // Convert completed document into a Dom
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = serviceExceptionReportDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = tr( "Dom Exception" );
    mError = tr( "Could not get WMS Service Exception at %1: %2 at line %3 column %4" )
             .arg( baseUrl )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn );

    QgsLogger::debug( "Dom Exception: " + mError );

    return FALSE;
  }

  QDomElement docElem = serviceExceptionReportDom.documentElement();

  // TODO: Assert the docElem.tagName() is "ServiceExceptionReport"

  // serviceExceptionProperty.version = docElem.attribute("version");

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if ( !e.isNull() )
    {
      //QgsDebugMsg(e.tagName() ); // the node really is an element.

      if ( e.tagName() == "ServiceException" )
      {
        QgsDebugMsg( "  ServiceException." );
        parseServiceException( e );
      }

    }
    n = n.nextSibling();
  }

  QgsDebugMsg( "exiting." );

  return TRUE;
}


void QgsWmsProvider::parseServiceException( QDomElement const & e )
{
  QgsDebugMsg( "entering." );

  QString seCode = e.attribute( "code" );
  QString seText = e.text();

  // set up friendly descriptions for the service exception
  if ( seCode == "InvalidFormat" )
  {
    mError = tr( "Request contains a Format not offered by the server." );
  }
  else if ( seCode == "InvalidCRS" )
  {
    mError = tr( "Request contains a CRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "InvalidSRS" )  // legacy WMS < 1.3.0
  {
    mError = tr( "Request contains a SRS not offered by the server for one or more of the Layers in the request." );
  }
  else if ( seCode == "LayerNotDefined" )
  {
    mError = tr( "GetMap request is for a Layer not offered by the server, "
                 "or GetFeatureInfo request is for a Layer not shown on the map." );
  }
  else if ( seCode == "StyleNotDefined" )
  {
    mError = tr( "Request is for a Layer in a Style not offered by the server." );
  }
  else if ( seCode == "LayerNotQueryable" )
  {
    mError = tr( "GetFeatureInfo request is applied to a Layer which is not declared queryable." );
  }
  else if ( seCode == "InvalidPoint" )
  {
    mError = tr( "GetFeatureInfo request contains invalid X or Y value." );
  }
  else if ( seCode == "CurrentUpdateSequence" )
  {
    mError = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                 "current value of service metadata update sequence number." );
  }
  else if ( seCode == "InvalidUpdateSequence" )
  {
    mError = tr( "Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                 "than current value of service metadata update sequence number." );
  }
  else if ( seCode == "MissingDimensionValue" )
  {
    mError = tr( "Request does not include a sample dimension value, and the server did not declare a "
                 "default value for that dimension." );
  }
  else if ( seCode == "InvalidDimensionValue" )
  {
    mError = tr( "Request contains an invalid sample dimension value." );
  }
  else if ( seCode == "OperationNotSupported" )
  {
    mError = tr( "Request is for an optional operation that is not supported by the server." );
  }
  else if ( seCode.isEmpty() )
  {
    mError = tr( "(No error code was reported)" );
  }
  else
  {
    mError = seCode + " " + tr( "(Unknown error code)" );
  }

  mError += "\n" + tr( "The WMS vendor also reported: " );
  mError += seText;

  // TODO = e.attribute("locator");

  QgsDebugMsg( "composed error message '"  + mError  + "'." );
  QgsDebugMsg( "exiting." );
}



QgsRectangle QgsWmsProvider::extent()
{
  if ( extentDirty )
  {
    if ( calculateExtent() )
    {
      extentDirty = FALSE;
    }
  }

  return layerExtent;
}

void QgsWmsProvider::begin()
{
  // TODO
}

bool QgsWmsProvider::isValid()
{
  return valid;
}


QString QgsWmsProvider::wmsVersion()
{
  // TODO
  return NULL;
}

QStringList QgsWmsProvider::supportedImageEncodings()
{
  return mCapabilities.capability.request.getMap.format;
}


QStringList QgsWmsProvider::subLayers() const
{
  return activeSubLayers;
}


QStringList QgsWmsProvider::subLayerStyles() const
{
  return activeSubStyles;
}


void QgsWmsProvider::showStatusMessage( QString const & theMessage )
{
  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit statusChanged( theMessage );
}


bool QgsWmsProvider::calculateExtent()
{
  //! \todo Make this handle non-geographic CRSs (e.g. floor plans) as per WMS spec

  QgsDebugMsg( "entered." );

  // Make sure we know what extents are available
  if ( !retrieveServerCapabilities() )
  {
    return FALSE;
  }

  // Set up the coordinate transform from the WMS standard CRS:84 bounding
  // box to the user's selected CRS
  if ( !mCoordinateTransform )
  {
    QgsCoordinateReferenceSystem qgisSrsSource;
    QgsCoordinateReferenceSystem qgisSrsDest;

    qgisSrsSource.createFromOgcWmsCrs( DEFAULT_LATLON_CRS );
    qgisSrsDest  .createFromOgcWmsCrs( imageCrs );

    mCoordinateTransform = new QgsCoordinateTransform( qgisSrsSource, qgisSrsDest );
  }

  bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
  for ( QStringList::Iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    QgsDebugMsg( "Sublayer Iterator: " + *it );
    // This is the extent for the layer name in *it
    QgsRectangle extent = extentForLayer.find( *it ).value();

    // Convert to the user's CRS as required
    try
    {
      extent = mCoordinateTransform->transformBoundingBox( extent, QgsCoordinateTransform::ForwardTransform );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      continue; //ignore extents of layers which cannot be transformed info the required CRS
    }

    //make sure extent does not contain 'inf' or 'nan'
    if ( !extent.isFinite() )
    {
      continue;
    }

    // add to the combined extent of all the active sublayers
    if ( firstLayer )
    {
      layerExtent = extent;
    }
    else
    {
      layerExtent.combineExtentWith( &extent );
    }

    firstLayer = false;

    QgsDebugMsg( "combined extent is '"  + layerExtent.toString()
                 + "' after '"  + ( *it ) + "'." );

  }

  QgsDebugMsg( "exiting with '"  + layerExtent.toString() + "'." );

  return TRUE;

}


int QgsWmsProvider::capabilities() const
{
  int capability = 0;
  bool canIdentify = FALSE;

  QgsDebugMsg( "entering." );

  // Test for the ability to use the Identify map tool
  for ( QStringList::const_iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    // Is sublayer visible?
    if ( activeSubLayerVisibility.find( *it ).value() )
    {
      // Is sublayer queryable?
      if ( mQueryableForLayer.find( *it ).value() )
      {
        QgsDebugMsg( "'"  + ( *it )  + "' is queryable." );
        canIdentify = TRUE;
      }
    }
  }

  // Collect all the test results into one bitmask
  if ( canIdentify )
  {
    capability = ( capability | QgsRasterDataProvider::Identify );
  }

  QgsDebugMsg( "exiting with '"  + QString( capability )  + "'." );

  return capability;
}


QString QgsWmsProvider::metadata()
{

  QString myMetadataQString = "";

  // Server Properties section
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Server Properties:" );
  myMetadataQString += "</td></tr>";

  // Use a nested table
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += "<table width=\"100%\">";

  // Table header
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
  myMetadataQString += "</th></tr>";

  // WMS Version
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "WMS Version" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.version;
  myMetadataQString += "</td></tr>";

  // Service Title
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Title" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.title;
  myMetadataQString += "</td></tr>";

  // Service Abstract
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Abstract" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.abstract;
  myMetadataQString += "</td></tr>";

  // Service Keywords
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Keywords" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.keywordList.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Service Online Resource
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Online Resource" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += "-";
  myMetadataQString += "</td></tr>";

  // Service Contact Information
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Contact Person" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactPerson;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPosition;
  myMetadataQString += "<br />";
  myMetadataQString += mCapabilities.service.contactInformation.contactPersonPrimary.contactOrganization;
  myMetadataQString += "</td></tr>";

  // Service Fees
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Fees" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.fees;
  myMetadataQString += "</td></tr>";

  // Service Access Constraints
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Access Constraints" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.accessConstraints;
  myMetadataQString += "</td></tr>";

  // GetMap Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Image Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getMap.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // GetFeatureInfo Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Identify Formats" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getFeatureInfo.format.join( "<br />" );
  myMetadataQString += "</td></tr>";

  // Layer Count (as managed by this provider)
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Layer Count" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += QString::number( layersSupported.size() );
  myMetadataQString += "</td></tr>";

  // Base URL
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "GetFeatureInfoUrl" );
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mGetFeatureInfoUrlBase;
  myMetadataQString += "</td></tr>";

  // Close the nested table
  myMetadataQString += "</table>";
  myMetadataQString += "</td></tr>";

  // Layer properties
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr( "Layer Properties:" );
  myMetadataQString += "</td></tr>";

  // Iterate through layers

  for ( int i = 0; i < layersSupported.size(); i++ )
  {

    // TODO: Handle nested layers
    QString layerName = layersSupported[i].name;   // for aesthetic convenience

    // Layer Properties section
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += layerName;
    myMetadataQString += "</td></tr>";

    // Use a nested table
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += "<table width=\"100%\">";

    // Table header
    myMetadataQString += "<tr><th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr( "Property" ) + "</font>";
    myMetadataQString += "</th>";
    myMetadataQString += "<th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr( "Value" ) + "</font>";
    myMetadataQString += "</th></tr>";

    // Layer Selectivity (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Selected" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += ( activeSubLayers.indexOf( layerName ) >= 0 ) ?
                         tr( "Yes" ) : tr( "No" );
    myMetadataQString += "</td></tr>";

    // Layer Visibility (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Visibility" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += ( activeSubLayers.indexOf( layerName ) >= 0 ) ?
                         (
                           ( activeSubLayerVisibility.find( layerName ).value() ) ?
                           tr( "Visible" ) : tr( "Hidden" )
                         ) :
                             tr( "n/a" );
    myMetadataQString += "</td></tr>";

    // Layer Title
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Title" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].title;
    myMetadataQString += "</td></tr>";

    // Layer Abstract
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Abstract" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].abstract;
    myMetadataQString += "</td></tr>";

    // Layer Queryability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can Identify" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].queryable ) ? tr( "Yes" ) : tr( "No" ) );
    myMetadataQString += "</td></tr>";

    // Layer Opacity
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can be Transparent" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].opaque ) ? tr( "No" ) : tr( "Yes" ) );
    myMetadataQString += "</td></tr>";

    // Layer Subsetability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Can Zoom In" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (( layersSupported[i].noSubsets ) ? tr( "No" ) : tr( "Yes" ) );
    myMetadataQString += "</td></tr>";

    // Layer Server Cascade Count
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Cascade Count" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].cascaded;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Width
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Fixed Width" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].fixedWidth;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "Fixed Height" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].fixedHeight;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr( "WGS 84 Bounding Box" );
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += extentForLayer[ layerName ].toString().toLocal8Bit().data();
    myMetadataQString += "</td></tr>";

    // Layer Coordinate Reference Systems
    /* MH: disable this as it causes performance problems if the server supports many CRS (e.g. QGIS mapserver)
    for ( int j = 0; j < layersSupported[i].crs.size(); j++ )
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in CRS" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].crs[j];
      myMetadataQString += "</td></tr>";
    }*/

    // Layer Styles
    for ( int j = 0; j < layersSupported[i].style.size(); j++ )
{
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Available in style" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td>";

      // Nested table.
      myMetadataQString += "<table width=\"100%\">";

      // Layer Style Name
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Name" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].name;
      myMetadataQString += "</td></tr>";

      // Layer Style Title
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Title" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].title;
      myMetadataQString += "</td></tr>";

      // Layer Style Abstract
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr( "Abstract" );
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].abstract;
      myMetadataQString += "</td></tr>";

      // Close the nested table
      myMetadataQString += "</table>";
      myMetadataQString += "</td></tr>";
    }

    // Close the nested table
    myMetadataQString += "</table>";
    myMetadataQString += "</td></tr>";

  } // for each layer

  QgsDebugMsg( "exiting with '"  + myMetadataQString  + "'." );

  return myMetadataQString;
}


QString QgsWmsProvider::identifyAsText( const QgsPoint& point )
{
  QgsDebugMsg( "Entering." );

  // Collect which layers to query on

  QStringList queryableLayers = QStringList();
  QString text = "";

  // Test for which layers are suitable for querying with
  for ( QStringList::const_iterator it  = activeSubLayers.begin();
        it != activeSubLayers.end();
        ++it )
  {
    // Is sublayer visible?
    if ( activeSubLayerVisibility.find( *it ).value() )
    {
      // Is sublayer queryable?
      if ( mQueryableForLayer.find( *it ).value() )
      {
        QgsDebugMsg( "Layer '" + *it + "' is queryable." );
        // Compose request to WMS server

        QString requestUrl = mGetFeatureInfoUrlBase;
        QString layer = QUrl::toPercentEncoding( *it );

        //! \todo Need to tie this into the options provided by GetCapabilities
        requestUrl += QString( "&QUERY_LAYERS=%1" ).arg( layer );
        requestUrl += QString( "&INFO_FORMAT=text/plain&X=%1&Y=%2" )
                      .arg( point.x() ).arg( point.y() );

// X,Y in WMS 1.1.1; I,J in WMS 1.3.0
//   requestUrl += QString( "&I=%1&J=%2" ).arg( point.x() ).arg( point.y() );

        text += "---------------\n" + QString::fromUtf8( retrieveUrl( requestUrl ) );
      }
    }
  }


  if ( text.isEmpty() )
  {
    // No layers were queryably. This can happen if identify tool was
    // active when this non-queriable layer was selected.
    // Return a descriptive text.

    text = tr( "Layer cannot be queried." );
  }

  QgsDebugMsg( "Exiting with: " + text );
  return text;
}


QgsCoordinateReferenceSystem QgsWmsProvider::crs()
{
  // TODO: implement
  return QgsCoordinateReferenceSystem();
}


QString QgsWmsProvider::lastErrorTitle()
{
  return mErrorCaption;
}


QString QgsWmsProvider::lastError()
{
  QgsDebugMsg( "returning '" + mError  + "'." );
  return mError;
}


QString  QgsWmsProvider::name() const
{
  return WMS_KEY;
} //  QgsWmsProvider::name()



QString  QgsWmsProvider::description() const
{
  return WMS_DESCRIPTION;
} //  QgsWmsProvider::description()





/**
 * Class factory to return a pointer to a newly created
 * QgsWmsProvider object
 */
QGISEXTERN QgsWmsProvider * classFactory( const QString *uri )
{
  return new QgsWmsProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return WMS_KEY;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return WMS_DESCRIPTION;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

