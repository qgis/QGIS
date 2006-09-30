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

#include "qgsconfig.h"

#ifdef HAVE_STDISFINITE
  #include <cmath>
#else
  #include <math.h>
#endif

#include "qgslogger.h"
#include "qgswmsprovider.h"

#include <fstream>
#include <iostream>

#include "qgsrect.h"
#include "qgsspatialrefsys.h"

#include "qgshttptransaction.h"

#include <Q3Url>


#ifdef QGISDEBUG
#include <QFile>
#endif

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif


static QString WMS_KEY = "wms";
static QString WMS_DESCRIPTION = "OGC Web Map Service version 1.3 data provider";

static QString DEFAULT_LATLON_CRS = "CRS:84";


QgsWmsProvider::QgsWmsProvider(QString const & uri)
  : QgsRasterDataProvider(uri),
    httpuri(uri),
    mHttpProxyHost(0),
    mHttpProxyPort(80),
    mHttpProxyUser(0),
    mHttpProxyPass(0),
    httpcapabilitiesresponse(0),
    cachedImage(0),
    cachedViewExtent(0),
    cachedPixelWidth(0),
    cachedPixelHeight(0),
    mCoordinateTransform(0),
    extentDirty(TRUE),
    imageCrs(DEFAULT_LATLON_CRS),
    mGetFeatureInfoUrlBase(0)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider: constructing with uri '" << uri.toLocal8Bit().data() << "'." << std::endl;
#endif

  // assume this is a valid layer until we determine otherwise
  valid = true;


  // URL can be in 3 forms:
  // 1) http://xxx.xxx.xx/yyy/yyy
  // 2) http://xxx.xxx.xx/yyy/yyy?
  // 3) http://xxx.xxx.xx/yyy/yyy?zzz=www

  // Prepare the URI so that we can later simply append param=value
  baseUrl = httpuri;

  if ( !(baseUrl.contains("?")) ) 
  {
    baseUrl.append("?");
  }
  else if (
           (baseUrl.right(1) != "?" )
           &&
           (baseUrl.right(1) != "&" )
          )
  {
    baseUrl.append("&");
  }

#ifdef QGISDEBUG
  std::cout << "baseUrl = " << baseUrl.toLocal8Bit().data() << std::endl;
#endif

//  getServerCapabilities();

  //httpuri = "http://www.ga.gov.au/bin/getmap.pl?dataset=national&Service=WMS&Version=1.1.0&Request=GetMap&"
  //      "BBox=130,-40,160,-10&SRS=EPSG:4326&Width=400&Height=400&Layers=railways&Format=image/png";

  //httpuri = "http://www.ga.gov.au/bin/getmap.pl?dataset=national&";
        
/*
  // 302-redirects to:        
  uri = "http://www.ga.gov.au/bin/mapserv40?"
        "map=/public/http/www/docs/map/national/national.map&"
        "map_logo_status=off&map_coast_status=off&Service=WMS&Version=1.1.0&"
        "Request=GetMap&BBox=130,-40,160,-10&SRS=EPSG:4326&Width=800&"
        "Height=800&Layers=railways&Format=image/png";
*/
  
//  downloadMapURI(uri);


#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider: exiting constructor." << std::endl;
#endif
  
}

QgsWmsProvider::~QgsWmsProvider()
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider: deconstructing." << std::endl;
#endif

  // Dispose of any cached image as created by draw()
  if (cachedImage)
  {
    delete cachedImage;
  }

  if (mCoordinateTransform)
  {
    delete mCoordinateTransform;
  }

}


QString QgsWmsProvider::proxyHost() const
{
  return mHttpProxyHost;
}


int QgsWmsProvider::proxyPort() const
{
  return mHttpProxyPort;
}


QString QgsWmsProvider::proxyUser() const
{
  return mHttpProxyUser;
}


QString QgsWmsProvider::proxyPass() const
{
  return mHttpProxyPass;
}


bool QgsWmsProvider::setProxy(QString const & host,
                                          int port,
                              QString const & user,
                              QString const & pass)
{
  mHttpProxyHost = host;
  mHttpProxyPort = port;
  mHttpProxyUser = user;
  mHttpProxyPass = pass;

  return TRUE;
}

bool QgsWmsProvider::supportedLayers(std::vector<QgsWmsLayerProperty> & layers)
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::supportedLayers: Entering." << std::endl;
#endif

  // Allow the provider to collect the capabilities first.
  if (!retrieveServerCapabilities())
  {
    return FALSE;
  }

  layers = layersSupported;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::supportedLayers: Exiting." << std::endl;
#endif

  return TRUE;
}


QSet<QString> QgsWmsProvider::supportedCrsForLayers(QStringList const & layers)
{
  QSet<QString> crsCandidates;

  QStringList::const_iterator i;
  for (i = layers.constBegin(); i != layers.constEnd(); ++i)
  {
    std::vector<QString> crsVector = crsForLayer[*i];
    QSet<QString>    crsSet;

    // convert std::vector to std::set for set comparisons
    for (int j = 0; j < crsVector.size(); j++)
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
      crsCandidates.intersect(crsSet);
    }

  }

  return crsCandidates;
}


size_t QgsWmsProvider::layerCount() const
{
    return 1;                   // XXX properly return actual number of layers
} // QgsWmsProvider::layerCount()


void QgsWmsProvider::addLayers(QStringList const &  layers,
                               QStringList const &  styles)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: Entering" <<
                  " with layer list of " << layers.join(", ").toLocal8Bit().data() <<
                   " and style list of " << styles.join(", ").toLocal8Bit().data() <<
               std::endl;
#endif

  // TODO: Make activeSubLayers a std::map in order to avoid duplicates

  activeSubLayers += layers;
  activeSubStyles += styles;

  // Set the visibility of these new layers on by default
  for ( QStringList::const_iterator it  = layers.begin(); 
        it != layers.end(); 
        ++it ) 
  {

    activeSubLayerVisibility[*it] = TRUE;
 
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: set visibility of layer '" << (*it).toLocal8Bit().data() << "' to TRUE." << std::endl;
#endif
  }

  // now that the layers have changed, the extent will as well.
  extentDirty = TRUE;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: Exiting." << std::endl;
#endif
}
      

void QgsWmsProvider::setLayerOrder(QStringList const &  layers)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setLayerOrder: Entering." << std::endl;
#endif
  
  activeSubLayers = layers;
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setLayerOrder: Exiting." << std::endl;
#endif
}


void QgsWmsProvider::setSubLayerVisibility(QString const & name, bool vis)
{
  activeSubLayerVisibility[name] = vis;
}


QString QgsWmsProvider::imageEncoding() const
{
  return imageMimeType;
}


void QgsWmsProvider::setImageEncoding(QString const & mimeType)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setImageEncoding: Setting image encoding to " << mimeType.toLocal8Bit().data() << "." <<
               std::endl;
#endif
  imageMimeType = mimeType;
}


void QgsWmsProvider::setImageCrs(QString const & crs)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setImageCrs: Setting image CRS to " << crs.toLocal8Bit().data() << "." <<
               std::endl;
#endif

  if (
      (crs != imageCrs)
      &&
      (! crs.isEmpty() )
     )
  {
    // delete old coordinate transform as it is no longer valid
    if (mCoordinateTransform)
    {
      delete mCoordinateTransform;
    }

    extentDirty = TRUE;

    imageCrs = crs;
  }

}


QImage* QgsWmsProvider::draw(QgsRect  const & viewExtent, int pixelWidth, int pixelHeight)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: Entering." << std::endl;
#endif

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: pixelWidth = " << pixelWidth << std::endl;
  std::cout << "QgsWmsProvider::draw: pixelHeight = " << pixelHeight << std::endl;

  std::cout << "QgsWmsProvider::draw: viewExtent.xMin() = " << viewExtent.xMin() << std::endl;
  std::cout << "QgsWmsProvider::draw: viewExtent.yMin() = " << viewExtent.yMin() << std::endl;
  std::cout << "QgsWmsProvider::draw: viewExtent.xMax() = " << viewExtent.xMax() << std::endl;
  std::cout << "QgsWmsProvider::draw: viewExtent.yMax() = " << viewExtent.yMax() << std::endl;
#endif

  // Can we reuse the previously cached image?
  if (
      (cachedImage) &&
      (cachedViewExtent == viewExtent) &&
      (cachedPixelWidth == pixelWidth) &&
      (cachedPixelHeight == pixelHeight)
     )
  {
    return cachedImage;
  }

  // Bounding box in WMS format
  QString bbox;
  // Warning: does not work with scientific notation
  bbox = QString("%1,%2,%3,%4").
          arg(viewExtent.xMin(),0,'f').
          arg(viewExtent.yMin(),0,'f').
          arg(viewExtent.xMax(),0,'f').
          arg(viewExtent.yMax(),0,'f');
          
  // Width in WMS format
  QString width;
  width = width.setNum(pixelWidth);

  // Height in WMS format
  QString height;
  height = height.setNum(pixelHeight);

  
  // Calculate active layers that are also visible.

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: Active" <<
                       " layer list of " << activeSubLayers.join(", ").toLocal8Bit().data() <<
                   " and style list of " << activeSubStyles.join(", ").toLocal8Bit().data() <<
               std::endl;
#endif


  QStringList visibleLayers = QStringList();
  QStringList visibleStyles = QStringList();

  QStringList::Iterator it2  = activeSubStyles.begin(); 

  for ( QStringList::Iterator it  = activeSubLayers.begin(); 
                              it != activeSubLayers.end(); 
                            ++it ) 
  {
    if (TRUE == activeSubLayerVisibility.find( *it )->second)
    {
      visibleLayers += *it;
      visibleStyles += *it2;
    }

    ++it2;
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: Visible" <<
                       " layer list of " << visibleLayers.join(", ").toLocal8Bit().data() <<
                   " and style list of " << visibleStyles.join(", ").toLocal8Bit().data() <<
               std::endl;
#endif


  QString layers = visibleLayers.join(",");
  Q3Url::encode( layers );

  QString styles = visibleStyles.join(",");
  Q3Url::encode( styles );


  // compose the URL query string for the WMS server.

  QString url = baseUrl;

  url += "SERVICE=WMS";
  url += "&";
  url += "VERSION=1.1.0";
  url += "&";
  url += "REQUEST=GetMap";
  url += "&";
  url += "BBOX=" + bbox;
  url += "&";
  url += "SRS=" + imageCrs;
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
  url += "&";
  url += "TRANSPARENT=TRUE";

  // cache some details for if the user wants to do an identifyAsHtml() later
  mGetFeatureInfoUrlBase = baseUrl;
  mGetFeatureInfoUrlBase += "SERVICE=WMS";
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "VERSION=1.1.0";
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "REQUEST=GetFeatureInfo";
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "BBOX=" + bbox;
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "SRS=" + imageCrs;
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
  mGetFeatureInfoUrlBase += "&";
  mGetFeatureInfoUrlBase += "TRANSPARENT=TRUE";


  QByteArray imagesource;
  imagesource = retrieveUrl(url);

  if (imagesource.isEmpty())
  {
    return 0;
  }

/*
  QgsHttpTransaction http(url, httpproxyhost, httpproxyport);

  // Do a passthrough for the status bar text
  connect(
          &http, SIGNAL(setStatus        (QString)),
           this,   SLOT(showStatusMessage(QString))
         );

  bool httpOk;

  httpOk = http.getSynchronously(imagesource);

  if (!httpOk)
  {
    // We had an HTTP exception

    mErrorCaption = tr("HTTP Exception");
    mError = http.errorString();

    mError += "\n" + tr("Tried URL: ") + url;

    return 0;
  }

  if (http.responseContentType() == "application/vnd.ogc.se_xml")
  {
    // We had a Service Exception from the WMS

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: got Service Exception as:\n" << QString(imagesource).toLocal8Bit().data() << std::endl;
#endif

    mErrorCaption = tr("WMS Service Exception");

    // set mError with the following:
    parseServiceExceptionReportDOM(imagesource);

    mError += "\n" + tr("Tried URL: ") + url;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: composed error message '" 
            << mError.toLocal8Bit().data() << "'." << std::endl;
#endif

    return 0;
  }
*/

#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::draw: Response received." << std::endl;
#endif

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
  if (cachedImage)
  {
    delete cachedImage;
  }
  cachedImage = new QImage(imagesource);

  // Remember settings for useful caching next time.
  cachedViewExtent = viewExtent;
  cachedPixelWidth = pixelWidth;
  cachedPixelHeight = pixelHeight;

#ifdef QGISDEBUG
  // Get what we can support

// Commented out for now, causes segfaults.
//  supportedFormats();

#endif

  
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: Exiting." << std::endl;
#endif


  // TODO: bit depth conversion to the client's expectation
//  return *(i.convertDepth(32));
  return cachedImage;

}

/*
void QgsWmsProvider::getServerCapabilities()
{
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::getServerCapabilities: entering." << std::endl;
#endif

  retrieveServerCapabilities();

  // TODO: Return generic server capabilities here

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::getServerCapabilities: exiting." << std::endl;
#endif

}
*/

bool QgsWmsProvider::retrieveServerCapabilities(bool forceRefresh)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::retrieveServerCapabilities: entering." << std::endl;
#endif

  if ( httpcapabilitiesresponse.isNull() or
       forceRefresh )
  {

    QString url = baseUrl + "SERVICE=WMS&REQUEST=GetCapabilities";

    httpcapabilitiesresponse = retrieveUrl(url);

    if (httpcapabilitiesresponse.isEmpty())
    {
      return FALSE;
    }
/*
    QgsHttpTransaction http(url, httpproxyhost, httpproxyport);

    // Do a passthrough for the status bar text
    connect(
            &http, SIGNAL(setStatus        (QString)),
             this,   SLOT(showStatusMessage(QString))
           );

    bool httpOk;
    httpOk = http.getSynchronously(httpcapabilitiesresponse);

    if (!httpOk)
    {
      // We had an HTTP exception

      mErrorCaption = tr("HTTP Exception");
      mError = http.errorString();

      mError += "\n" + tr("Tried URL: ") + url;

#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::getServerCapabilities: !httpOK: " << 
                mError.toLocal8Bit().data() << std::endl;
#endif

      return FALSE;
    }
*/

#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::getServerCapabilities: Converting to DOM." << std::endl;
#endif

    bool domOK;
    domOK = parseCapabilitiesDOM(httpcapabilitiesresponse, mCapabilities);

    if (!domOK)
    {
      // We had an DOM exception - 
      // mErrorCaption and mError are pre-filled by parseCapabilitiesDOM

      mError += "\n" + tr("Tried URL: ") + url;

#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::getServerCapabilities: !domOK: " << 
                mError.toLocal8Bit().data() << std::endl;
#endif

      return FALSE;
    }

  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::getServerCapabilities: exiting." << std::endl;
#endif

  return TRUE;

}


QByteArray QgsWmsProvider::retrieveUrl(QString url)
{
  QgsDebugMsg("WMS request Url: " + url);
  QgsHttpTransaction http(
    url,
    mHttpProxyHost,
    mHttpProxyPort,
    mHttpProxyUser,
    mHttpProxyPass);

  // Do a passthrough for the status bar text
  connect(
          &http, SIGNAL(setStatus        (QString)),
           this,   SLOT(showStatusMessage(QString))
         );

  QByteArray httpResponse;
  bool httpOk;

  httpOk = http.getSynchronously(httpResponse);

  if (!httpOk)
  {
    // We had an HTTP exception

    mErrorCaption = tr("HTTP Exception");
    mError = http.errorString();

    mError += "\n" + tr("Tried URL: ") + url;

    return QByteArray("");
  }

  if (http.responseContentType() == "application/vnd.ogc.se_xml")
  {
    // We had a Service Exception from the WMS

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::retrieveUrl: got Service Exception as:\n"
            << QString(httpResponse).toLocal8Bit().data() << std::endl;
#endif

    mErrorCaption = tr("WMS Service Exception");

    // set mError with the following:
    parseServiceExceptionReportDOM(httpResponse);

    mError += "\n" + tr("Tried URL: ") + url;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::retrieveUrl: composed error message '"
            << mError.toLocal8Bit().data() << "'." << std::endl;
#endif

    return QByteArray("");
  }

  return httpResponse;
}

// deprecated
/*
bool QgsWmsProvider::downloadCapabilitiesURI(QString const & uri)
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: Entered with '" << uri.toLocal8Bit().data() << "'" << std::endl;
#endif

  QgsHttpTransaction http(uri, httpproxyhost, httpproxyport);

  // Do a passthrough for the status bar text
  connect(
          &http, SIGNAL(setStatus        (QString)),
           this,   SLOT(showStatusMessage(QString))
         );

  bool httpOk;
  httpOk = http.getSynchronously(httpcapabilitiesresponse);

  if (!httpOk)
  {
    // We had an HTTP exception

    mErrorCaption = tr("HTTP Exception");
    mError = http.errorString();

    mError += "\n" + tr("Tried URL: ") + uri;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: !httpOK: " << 
               mError.toLocal8Bit().data() << std::endl;
#endif

    return FALSE;
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: Converting to DOM." << std::endl;
#endif

  bool domOK;
  domOK = parseCapabilitiesDOM(httpcapabilitiesresponse, capabilities);

  if (!domOK)
  {
    // We had an DOM exception - 
    // mErrorCaption and mError are pre-filled by parseCapabilitiesDOM

    mError += "\n" + tr("Tried URL: ") + uri;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: !domOK: " << 
               mError.toLocal8Bit().data() << std::endl;
#endif

    return FALSE;
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: exiting." << std::endl;
#endif

  return TRUE;

}
*/
#include <fstream>
bool QgsWmsProvider::parseCapabilitiesDOM(QByteArray const & xml, QgsWmsCapabilitiesProperty& capabilitiesProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapabilitiesDOM: entering." << std::endl;

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

  QString responsestring(QString::fromAscii(xml.constData(), xml.size()));
  QgsLogger::debug("QgsWmsProvider::parseCapabilitiesDOM, received the following data: "+responsestring);
  
  //QFile file( "/tmp/qgis-wmsprovider-capabilities.xml" );
  //if ( file.open( QIODevice::WriteOnly ) ) 
  //{
  //  file.writeBlock(xml);
  //  file.close();
  //}
#endif
  
  // Convert completed document into a DOM
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = capabilitiesDOM.setContent(xml, false, &errorMsg, &errorLine, &errorColumn);

  if (!contentSuccess)
  {
    mErrorCaption = tr("DOM Exception");
    mError = QString(tr("Could not get WMS capabilities: %1 at line %2 column %3")
                .arg(errorMsg)
                .arg(errorLine)
                .arg(errorColumn) );

    mError += "\n" + tr("This is probably due to an incorrect WMS Server URL.");

  QgsLogger::debug("DOM Exception: "+mError);

    return FALSE;
  }

  QDomElement docElem = capabilitiesDOM.documentElement();

  // Assert that the DTD is what we expected (i.e. a WMS Capabilities document)
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapabilitiesDOM: testing tagName " 
            << docElem.tagName().toLocal8Bit().data() << std::endl;
#endif

  if (!
      (
       (docElem.tagName() == "WMS_Capabilities")     // (1.3 vintage)
       or
       (docElem.tagName() == "WMT_MS_Capabilities")  // (1.1.1 vintage)
      )
     )
  {
    mErrorCaption = tr("DOM Exception");
    mError = QString(tr("Could not get WMS capabilities in the "
                        "expected format (DTD): no %1 or %2 found")
                .arg("WMS_Capabilities")
                .arg("WMT_MS_Capabilities")
             );

    mError += "\n" + tr("This is probably due to an incorrect WMS Server URL.");

  QgsLogger::debug("DOM Exception: "+mError);

    return FALSE;
  }

  capabilitiesProperty.version = docElem.attribute("version");

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() ) {
          //std::cout << e.tagName() << std::endl; // the node really is an element.

          if      (e.tagName() == "Service")
          {
#ifdef QGISDEBUG
            std::cout << "  Service." << std::endl;
#endif
            parseService(e, capabilitiesProperty.service);
          }
          else if (e.tagName() == "Capability")
          {
#ifdef QGISDEBUG
            std::cout << "  Capability." << std::endl;
#endif
            parseCapability(e, capabilitiesProperty.capability);
          }

      }
      n = n.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapabilitiesDOM: exiting." << std::endl;
#endif

  return TRUE;
}


void QgsWmsProvider::parseService(QDomElement const & e, QgsWmsServiceProperty& serviceProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseService: entering." << std::endl;
#endif
  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          //std::cout << "  " << e1.tagName() << std::endl; // the node really is an element.

          if      (e1.tagName() == "Title")
          {
            serviceProperty.title = e1.text();
          }
          else if (e1.tagName() == "Abstract")
          {
            serviceProperty.abstract = e1.text();
          }
          else if (e1.tagName() == "KeywordList")
          {
            parseKeywordList(e1, serviceProperty.keywordList);
          }
          else if (e1.tagName() == "OnlineResource")
          {
            parseOnlineResource(e1, serviceProperty.onlineResource);
          }
          else if (e1.tagName() == "ContactInformation")
          {
            parseContactInformation(e1, serviceProperty.contactInformation);
          }
          else if (e1.tagName() == "Fees")
          {
            serviceProperty.fees = e1.text();
          }
          else if (e1.tagName() == "AccessConstraints")
          {
            serviceProperty.accessConstraints = e1.text();
          }
          else if (e1.tagName() == "LayerLimit")
          {
            serviceProperty.layerLimit = e1.text().toUInt();
          }
          else if (e1.tagName() == "MaxWidth")
          {
            serviceProperty.maxWidth = e1.text().toUInt();
          }
          else if (e1.tagName() == "MaxHeight")
          {
            serviceProperty.maxHeight = e1.text().toUInt();
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseService: exiting." << std::endl;
#endif

}


void QgsWmsProvider::parseCapability(QDomElement const & e, QgsWmsCapabilityProperty& capabilityProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapability: entering." << std::endl;
#endif
  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          //std::cout << "  " << e1.tagName() << std::endl; // the node really is an element.
      
          if (e1.tagName() == "Request")
          {
            parseRequest(e1, capabilityProperty.request);
          }
          else if (e1.tagName() == "Layer")
          {
            parseLayer(e1, capabilityProperty.layer);
          }
      
      }
      n1 = n1.nextSibling();
  }    

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapability: exiting." << std::endl;
#endif

}


void QgsWmsProvider::parseContactPersonPrimary(QDomElement const & e, QgsWmsContactPersonPrimaryProperty& contactPersonPrimaryProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactPersonPrimary: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "ContactPerson")
          {
            contactPersonPrimaryProperty.contactPerson = e1.text();
          }
          else if (e1.tagName() == "ContactOrganization")
          {
            contactPersonPrimaryProperty.contactOrganization = e1.text();
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactPersonPrimary: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseContactAddress(QDomElement const & e, QgsWmsContactAddressProperty& contactAddressProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactAddress: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "AddressType")
          {
            contactAddressProperty.addressType = e1.text();
          }
          else if (e1.tagName() == "Address")
          {
            contactAddressProperty.address = e1.text();
          }
          else if (e1.tagName() == "City")
          {
            contactAddressProperty.city = e1.text();
          }
          else if (e1.tagName() == "StateOrProvince")
          {
            contactAddressProperty.stateOrProvince = e1.text();
          }
          else if (e1.tagName() == "PostCode")
          {
            contactAddressProperty.postCode = e1.text();
          }
          else if (e1.tagName() == "Country")
          {
            contactAddressProperty.country = e1.text();
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactAddress: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseContactInformation(QDomElement const & e, QgsWmsContactInformationProperty& contactInformationProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactInformation: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "ContactPersonPrimary")
          {
            parseContactPersonPrimary(e1, contactInformationProperty.contactPersonPrimary);
          }
          else if (e1.tagName() == "ContactPosition")
          {
            contactInformationProperty.contactPosition = e1.text();
          }
          else if (e1.tagName() == "ContactAddress")
          {
            parseContactAddress(e1, contactInformationProperty.contactAddress);
          }
          else if (e1.tagName() == "ContactVoiceTelephone")
          {
            contactInformationProperty.contactVoiceTelephone = e1.text();
          }
          else if (e1.tagName() == "ContactFacsimileTelephone")
          {
            contactInformationProperty.contactFacsimileTelephone = e1.text();
          }
          else if (e1.tagName() == "ContactElectronicMailAddress")
          {
            contactInformationProperty.contactElectronicMailAddress = e1.text();
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseContactInformation: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseOnlineResource(QDomElement const & e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseOnlineResource: entering." << std::endl;
#endif

  onlineResourceAttribute.xlinkHref = e.attribute("xlink:href");

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseOnlineResource: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseKeywordList(QDomElement  const & e, QStringList& keywordListProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseKeywordList: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "Keyword")
          {
#ifdef QGISDEBUG
            std::cout << "      Keyword." << std::endl; 
#endif
            keywordListProperty += e1.text();
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseKeywordList: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseGet(QDomElement const & e, QgsWmsGetProperty& getProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseGet: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "OnlineResource")
          {
#ifdef QGISDEBUG
            std::cout << "      OnlineResource." << std::endl;
#endif
            parseOnlineResource(e1, getProperty.onlineResource);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseGet: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parsePost(QDomElement const & e, QgsWmsPostProperty& postProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parsePost: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "OnlineResource")
          {
#ifdef QGISDEBUG
            std::cout << "      OnlineResource." << std::endl;
#endif
            parseOnlineResource(e1, postProperty.onlineResource);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parsePost: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseHttp(QDomElement const & e, QgsWmsHttpProperty& httpProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseHttp: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "Get")
          {
#ifdef QGISDEBUG
            std::cout << "      Get." << std::endl;
#endif
            parseGet(e1, httpProperty.get);
          }
          else if (e1.tagName() == "Post")
          {
#ifdef QGISDEBUG
            std::cout << "      Post." << std::endl;
#endif
            parsePost(e1, httpProperty.post);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseHttp: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseDcpType(QDomElement const & e, QgsWmsDcpTypeProperty& dcpType)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseDcpType: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "HTTP")
          {
#ifdef QGISDEBUG
            std::cout << "      HTTP." << std::endl; 
#endif
            parseHttp(e1, dcpType.http);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseDcpType: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseOperationType(QDomElement const & e, QgsWmsOperationType& operationType)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseOperationType: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "Format")
          {
#ifdef QGISDEBUG
            std::cout << "      Format." << std::endl; 
#endif
            operationType.format += e1.text();
          }
          else if (e1.tagName() == "DCPType")
          {
#ifdef QGISDEBUG
            std::cout << "      DCPType." << std::endl;
#endif
            QgsWmsDcpTypeProperty dcp;
            parseDcpType(e1, dcp);
            operationType.dcpType.push_back(dcp);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseOperationType: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseRequest(QDomElement const & e, QgsWmsRequestProperty& requestProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseRequest: entering." << std::endl;
#endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "GetMap")
          {
#ifdef QGISDEBUG
            std::cout << "      GetMap." << std::endl; 
#endif
            parseOperationType(e1, requestProperty.getMap);
          }
          else if (e1.tagName() == "GetFeatureInfo")
          {
#ifdef QGISDEBUG
            std::cout << "      GetFeatureInfo." << std::endl;
#endif
            parseOperationType(e1, requestProperty.getFeatureInfo);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseRequest: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseLegendUrl(QDomElement const & e, QgsWmsLegendUrlProperty& legendUrlProperty)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseLegendUrl: entering." << std::endl;
#endif

  legendUrlProperty.width  = e.attribute("width").toUInt();
  legendUrlProperty.height = e.attribute("height").toUInt();

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "Format")
          {
            legendUrlProperty.format = e1.text();
          }
          else if (e1.tagName() == "OnlineResource")
          {
            parseOnlineResource(e1, legendUrlProperty.onlineResource);
          }
      }
      n1 = n1.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseLegendUrl: exiting." << std::endl;
#endif
}


void QgsWmsProvider::parseStyle(QDomElement const & e, QgsWmsStyleProperty& styleProperty)
{
// #ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseStyle: entering." << std::endl;
// #endif

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          if      (e1.tagName() == "Name")
          {
            styleProperty.name = e1.text();
          }
          else if (e1.tagName() == "Title")
          {
            styleProperty.title = e1.text();
          }
          else if (e1.tagName() == "Abstract")
          {
            styleProperty.abstract = e1.text();
          }
          else if (e1.tagName() == "LegendURL")
          {
            // TODO
          }
          else if (e1.tagName() == "StyleSheetURL")
          {
            // TODO
          }
          else if (e1.tagName() == "StyleURL")
          {
            // TODO
          }
      }
      n1 = n1.nextSibling();
  }

//#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseStyle: exiting." << std::endl;
//#endif
}


void QgsWmsProvider::parseLayer(QDomElement const & e, QgsWmsLayerProperty& layerProperty,
                                QgsWmsLayerProperty *parentProperty)
{
#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseLayer: entering." << std::endl;
#endif


// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
//  // enforce WMS non-inheritance rules
//  layerProperty.name =        QString::null;
//  layerProperty.title =       QString::null;
//  layerProperty.abstract =    QString::null;
//  layerProperty.keywordList.clear();

  // assume true until we find a child layer
  bool atleaf = TRUE;

  layerProperty.queryable   = e.attribute("queryable").toUInt();
  layerProperty.cascaded    = e.attribute("cascaded").toUInt();
  layerProperty.opaque      = e.attribute("opaque").toUInt();
  layerProperty.noSubsets   = e.attribute("noSubsets").toUInt();
  layerProperty.fixedWidth  = e.attribute("fixedWidth").toUInt();
  layerProperty.fixedHeight = e.attribute("fixedHeight").toUInt();

  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          //std::cout << "    " << e1.tagName() << std::endl; // the node really is an element.

          if      (e1.tagName() == "Layer")
          {
//            std::cout << "      Nested layer." << std::endl; 

            QgsWmsLayerProperty subLayerProperty;

            // Inherit things into the sublayer
            //   Ref: 7.2.4.8 Inheritance of layer properties
            subLayerProperty.style                    = layerProperty.style;
            subLayerProperty.crs                      = layerProperty.crs;
            subLayerProperty.boundingBox              = layerProperty.boundingBox;
            subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
            // TODO

            parseLayer(e1, subLayerProperty, &layerProperty );

            layerProperty.layer.push_back(subLayerProperty);

            atleaf = FALSE;
          }
          else if (e1.tagName() == "Name")
          {
            layerProperty.name = e1.text();
          }
          else if (e1.tagName() == "Title")
          {
            layerProperty.title = e1.text();
          }
          else if (e1.tagName() == "Abstract")
          {
            layerProperty.abstract = e1.text();
          }
          else if (e1.tagName() == "KeywordList")
          {
            parseKeywordList(e1, layerProperty.keywordList);
          }
          else if (e1.tagName() == "CRS")
          {
            layerProperty.crs.push_back(e1.text());
          }
          else if (e1.tagName() == "SRS")        // legacy from earlier versions of WMS
          {
            // SRS can contain several definitions separated by whitespace
            // though this was deprecated in WMS 1.1.1
            QStringList srsList = e1.text().split(QRegExp("\\s+"));

            QStringList::const_iterator i;
            for (i = srsList.constBegin(); i != srsList.constEnd(); ++i)
            {
              layerProperty.crs.push_back(*i);
            }
          }
          else if (e1.tagName() == "LatLonBoundingBox")        // legacy from earlier versions of WMS
          {

//            std::cout << "      LLBB is: '" << e1.attribute("minx") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("miny") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("maxx") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("maxy") << "'." << std::endl;

            layerProperty.ex_GeographicBoundingBox = QgsRect( 
                                                e1.attribute("minx").toDouble(),
                                                e1.attribute("miny").toDouble(),
                                                e1.attribute("maxx").toDouble(),
                                                e1.attribute("maxy").toDouble()
                                              );
          }
	  else if(e1.tagName() == "EX_GeographicBoundingBox") //for WMS 1.3
	    {
	      QDomElement wBoundLongitudeElem = n1.namedItem("westBoundLongitude").toElement();
	      QDomElement eBoundLongitudeElem = n1.namedItem("eastBoundLongitude").toElement();
	      QDomElement sBoundLatitudeElem = n1.namedItem("southBoundLatitude").toElement();
	      QDomElement nBoundLatitudeElem = n1.namedItem("northBoundLatitude").toElement();
	      double wBLong, eBLong, sBLat, nBLat;
	      bool wBOk, eBOk, sBOk, nBOk;
	      wBLong = wBoundLongitudeElem.text().toDouble(&wBOk);
	      eBLong = eBoundLongitudeElem.text().toDouble(&eBOk);
	      sBLat = sBoundLatitudeElem.text().toDouble(&sBOk);
	      nBLat = nBoundLatitudeElem.text().toDouble(&nBOk);
	      if(wBOk && eBOk && sBOk && nBOk)
		{
		  layerProperty.ex_GeographicBoundingBox = QgsRect(wBLong, sBLat, eBLong, nBLat);
		}
	    }
          else if (e1.tagName() == "BoundingBox")
          {
              // TODO: overwrite inherited
              QgsWmsBoundingBoxProperty bbox;
              bbox.box = QgsRect ( e1.attribute("minx").toDouble(),
                                   e1.attribute("miny").toDouble(),
                                   e1.attribute("maxx").toDouble(),
                                   e1.attribute("maxy").toDouble()
                                 );
              bbox.crs = e1.attribute("SRS");
              layerProperty.boundingBox.push_back ( bbox );
          }
          else if (e1.tagName() == "Dimension")
          {
            // TODO
          }
          else if (e1.tagName() == "Attribution")
          {
            // TODO
          }
          else if (e1.tagName() == "AuthorityURL")
          {
            // TODO
          }
          else if (e1.tagName() == "Identifier")
          {
            // TODO
          }
          else if (e1.tagName() == "MetadataURL")
          {
            // TODO
          }
          else if (e1.tagName() == "DataURL")
          {
            // TODO
          }
          else if (e1.tagName() == "FeatureListURL")
          {
            // TODO
          }
          else if (e1.tagName() == "Style")
          {
            QgsWmsStyleProperty styleProperty;

            parseStyle(e1, styleProperty);

            layerProperty.style.push_back(styleProperty);
          }
          else if (e1.tagName() == "MinScaleDenominator")
          {
            // TODO
          }
          else if (e1.tagName() == "MaxScaleDenominator")
          {
            // TODO
          }
          // If we got here then it's not in the WMS 1.3 standard

      }
      n1 = n1.nextSibling();
  }

  if (atleaf)
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
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseLayer: testing bounding box CRS which is " 
            << layerProperty.boundingBox[i].crs.toLocal8Bit().data() << "." << std::endl;
#endif
      if ( layerProperty.boundingBox[i].crs == DEFAULT_LATLON_CRS )
      {
        extentForLayer[ layerProperty.name ] = 
                layerProperty.boundingBox[i].box;
      }
    }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseLayer: extent for " 
            << layerProperty.name.toLocal8Bit().data() << " is "
            << extentForLayer[ layerProperty.name ].stringRep(3).toLocal8Bit().data() << "." << std::endl;
#endif

    // Insert into the local class' registry
    layersSupported.push_back(layerProperty);

    //if there are several <Layer> elements without a parent layer, the style list needs to be cleared
    if(atleaf)
      {
	layerProperty.style.clear();
      }
  }

#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseLayer: exiting." << std::endl;
#endif
}


bool QgsWmsProvider::parseServiceExceptionReportDOM(QByteArray const & xml)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseServiceExceptionReportDOM: entering." << std::endl;

  //test the content of the QByteArray
  QString responsestring(xml);
  QgsLogger::debug("QgsWmsProvider::parseServiceExceptionReportDOM, received the following data: "+responsestring);
#endif

  // Convert completed document into a DOM
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = serviceExceptionReportDOM.setContent(xml, false, &errorMsg, &errorLine, &errorColumn);

  if (!contentSuccess)
  {
    mErrorCaption = tr("DOM Exception");
    mError = QString(tr("Could not get WMS Service Exception at %1: %2 at line %3 column %4")
                .arg(baseUrl)
                .arg(errorMsg)
                .arg(errorLine)
                .arg(errorColumn) );

  QgsLogger::debug("DOM Exception: "+mError);

    return FALSE;
  }

  QDomElement docElem = serviceExceptionReportDOM.documentElement();

  // TODO: Assert the docElem.tagName() is "ServiceExceptionReport"

  // serviceExceptionProperty.version = docElem.attribute("version");

  // Start walking through XML.
  QDomNode n = docElem.firstChild();

  while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() ) {
          //std::cout << e.tagName() << std::endl; // the node really is an element.

          if      (e.tagName() == "ServiceException")
          {
#ifdef QGISDEBUG
            std::cout << "  ServiceException." << std::endl;
#endif
            parseServiceException(e);
          }

      }
      n = n.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseServiceExceptionReportDOM: exiting." << std::endl;
#endif

  return TRUE;
}


void QgsWmsProvider::parseServiceException(QDomElement const & e)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseServiceException: entering." << std::endl;
#endif

  QString seCode = e.attribute("code");
  QString seText = e.text();

  // set up friendly descriptions for the service exception
  if      (seCode == "InvalidFormat")
  {
    mError = tr("Request contains a Format not offered by the server.");
  }
  else if (seCode == "InvalidCRS")
  {
    mError = tr("Request contains a CRS not offered by the server for one or more of the Layers in the request.");
  }
  else if (seCode == "InvalidSRS")  // legacy WMS < 1.3.0
  {
    mError = tr("Request contains a SRS not offered by the server for one or more of the Layers in the request.");
  }
  else if (seCode == "LayerNotDefined")
  {
    mError = tr("GetMap request is for a Layer not offered by the server, "
                "or GetFeatureInfo request is for a Layer not shown on the map.");
  }
  else if (seCode == "StyleNotDefined")
  {
    mError = tr("Request is for a Layer in a Style not offered by the server.");
  }
  else if (seCode == "LayerNotQueryable")
  {
    mError = tr("GetFeatureInfo request is applied to a Layer which is not declared queryable.");
  }
  else if (seCode == "InvalidPoint")
  {
    mError = tr("GetFeatureInfo request contains invalid X or Y value.");
  }
  else if (seCode == "CurrentUpdateSequence")
  {
    mError = tr("Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to "
                "current value of service metadata update sequence number.");
  }
  else if (seCode == "InvalidUpdateSequence")
  {
    mError = tr("Value of (optional) UpdateSequence parameter in GetCapabilities request is greater "
                "than current value of service metadata update sequence number.");
  }
  else if (seCode == "MissingDimensionValue")
  {
    mError = tr("Request does not include a sample dimension value, and the server did not declare a "
                "default value for that dimension.");
  }
  else if (seCode == "InvalidDimensionValue")
  {
    mError = tr("Request contains an invalid sample dimension value.");
  }
  else if (seCode == "OperationNotSupported")
  {
    mError = tr("Request is for an optional operation that is not supported by the server.");
  }
  else
  {
    mError = tr("(Unknown error code from a post-1.3 WMS server)");
  }

  mError += "\n" + tr("The WMS vendor also reported: ");
  mError += seText;

  mError += "\n" + tr("This is probably due to a bug in the QGIS program.  Please report this error.");

  // TODO = e.attribute("locator");

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseServiceException: composed error message '" << mError.toLocal8Bit().data() << "'." << std::endl;
#endif

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseServiceException: exiting." << std::endl;
#endif
}


QgsDataSourceURI * QgsWmsProvider::getURI()
{

  QgsDataSourceURI* dsuri;
   
  dsuri = new QgsDataSourceURI;
  
  //TODO
  
  return dsuri;
}


void QgsWmsProvider::setURI(QgsDataSourceURI * uri)
{
  // TODO
} 



QgsRect *QgsWmsProvider::extent()
{
  if (extentDirty)
  {
    if (calculateExtent())
    {
      extentDirty = FALSE;
    }
  }

  return &layerExtent;
}

void QgsWmsProvider::reset()
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


void QgsWmsProvider::showStatusMessage(QString const & theMessage)
{
    // Pass-through
    // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
    emit setStatus(theMessage);
}


bool QgsWmsProvider::calculateExtent()
{
  //! \todo Make this handle non-geographic CRSs (e.g. floor plans) as per WMS spec

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: entered." << std::endl;
#endif

  // Make sure we know what extents are available
  if (!retrieveServerCapabilities())
  {
    return FALSE;
  }

  // Set up the coordinate transform from the WMS standard CRS:84 bounding
  // box to the user's selected CRS
  if (!mCoordinateTransform)
  {
    QgsSpatialRefSys qgisSrsSource;
    QgsSpatialRefSys qgisSrsDest;

    qgisSrsSource.createFromOgcWmsCrs(DEFAULT_LATLON_CRS);
    qgisSrsDest  .createFromOgcWmsCrs(imageCrs);

    mCoordinateTransform = new QgsCoordinateTransform(qgisSrsSource, qgisSrsDest);
  }

  bool firstLayer = true; //flag to know if a layer is the first to be successfully transformed
  for ( QStringList::Iterator it  = activeSubLayers.begin(); 
                              it != activeSubLayers.end(); 
                            ++it ) 
  {
    QgsDebugMsg("Sublayer Iterator: "+*it);
    // This is the extent for the layer name in *it
    QgsRect extent = extentForLayer.find( *it )->second;

    // Convert to the user's CRS as required
    try
      {
	extent = mCoordinateTransform->transformBoundingBox(extent, QgsCoordinateTransform::FORWARD);
      }
    catch(QgsCsException &cse)
      {
	continue; //ignore extents of layers which cannot be transformed info the required CRS
      }

    //make sure extent does not contain 'inf' or 'nan'
#ifdef HAVE_STDISFINITE
    if(!std::isfinite(extent.xMin()) || !std::isfinite((int)extent.yMin()) || !std::isfinite(extent.xMax()) || \
       !std::isfinite((int)extent.yMax()))
#else
    if(!isfinite(extent.xMin()) || !isfinite((int)extent.yMin()) || !isfinite(extent.xMax()) || \
       !isfinite((int)extent.yMax()))
#endif
      {
	continue;
      }

    // add to the combined extent of all the active sublayers
    if (firstLayer)
    {
      layerExtent = extent;
    }
    else
    {
      layerExtent.combineExtentWith( &extent );
    }

    firstLayer = false;
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: combined extent is '" << 
               layerExtent.stringRep().toLocal8Bit().data() << "' after '" << (*it).toLocal8Bit().data() << "'." << std::endl;
#endif

  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: exiting with '" << layerExtent.stringRep().toLocal8Bit().data() << "'." << std::endl;
#endif

  return TRUE;

}


int QgsWmsProvider::capabilities() const
{
  int capability = 0;
  bool canIdentify = FALSE;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::capabilities: entering." << std::endl;
#endif

  // Test for the ability to use the Identify map tool
  for ( QStringList::const_iterator it  = activeSubLayers.begin(); 
                                    it != activeSubLayers.end(); 
                                  ++it )
  {
    // Is sublayer visible?
    if (TRUE == activeSubLayerVisibility.find( *it )->second)
    {
      // Is sublayer queryable?
      if (TRUE == mQueryableForLayer.find( *it )->second)
      {
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::capabilities: '" << (*it).toLocal8Bit().data() << "' is queryable." << std::endl;
#endif
        canIdentify = TRUE;
      }
    }
  }

  // Collect all the test results into one bitmask
  if (canIdentify)
  {
    capability = (capability | QgsRasterDataProvider::Identify);
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::capabilities: exiting with '" << capability << "'." << std::endl;
#endif
  return capability;
}


QString QgsWmsProvider::getMetadata()
{

  QString myMetadataQString = "";

  // Server Properties section
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Server Properties:");
  myMetadataQString += "</td></tr>";

  // Use a nested table
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += "<table width=\"100%\">";

  // Table header
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Property") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Value") + "</font>";
  myMetadataQString += "</th><tr>";

  // WMS Version
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("WMS Version");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.version;
  myMetadataQString += "</td></tr>";

  // Service Title
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Title");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.title;
  myMetadataQString += "</td></tr>";

  // Service Abstract
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Abstract");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.abstract;
  myMetadataQString += "</td></tr>";

  // Service Keywords
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Keywords");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.keywordList.join("<br />");
  myMetadataQString += "</td></tr>";

  // Service Online Resource
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Online Resource");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += "-";
  myMetadataQString += "</td></tr>";

  // Service Contact Information
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Contact Person");
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
  myMetadataQString += tr("Fees");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.fees;
  myMetadataQString += "</td></tr>";

  // Service Access Constraints
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Access Constraints");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.service.accessConstraints;
  myMetadataQString += "</td></tr>";

  // GetMap Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Image Formats");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getMap.format.join("<br />");
  myMetadataQString += "</td></tr>";

  // GetFeatureInfo Request Formats
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Identify Formats");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += mCapabilities.capability.request.getFeatureInfo.format.join("<br />");
  myMetadataQString += "</td></tr>";

  // Layer Count (as managed by this provider)
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Layer Count");
  myMetadataQString += "</td>";
  myMetadataQString += "<td bgcolor=\"gray\">";
  myMetadataQString += QString::number( layersSupported.size() );
  myMetadataQString += "</td></tr>";

  // Close the nested table
  myMetadataQString += "</table>";
  myMetadataQString += "</td></tr>";

  // Iterate through layers

  for (int i = 0; i < layersSupported.size(); i++)
  {

    // TODO: Handle nested layers
    QString layerName = layersSupported[i].name;   // for aesthetic convenience

    // Layer Properties section
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Layer Properties: ");
    myMetadataQString += layerName;
    myMetadataQString += "</td></tr>";
  
    // Use a nested table
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += "<table width=\"100%\">";
  
    // Table header
    myMetadataQString += "<tr><th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr("Property") + "</font>";
    myMetadataQString += "</th>";
    myMetadataQString += "<th bgcolor=\"black\">";
    myMetadataQString += "<font color=\"white\">" + tr("Value") + "</font>";
    myMetadataQString += "</th><tr>";
  
    // Layer Selectivity (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Selected");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (activeSubLayers.findIndex(layerName) >= 0) ?
                           tr("Yes") : tr("No");
    myMetadataQString += "</td></tr>";
  
    // Layer Visibility (as managed by this provider)
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Visibility");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += (activeSubLayers.findIndex(layerName) >= 0) ?
                           (
                            (activeSubLayerVisibility.find(layerName)->second) ?
                            tr("Visible") : tr("Hidden")
                           ) :
                           tr("n/a");
    myMetadataQString += "</td></tr>";
  
    // Layer Title
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Title");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].title;
    myMetadataQString += "</td></tr>";
  
    // Layer Abstract
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Abstract");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].abstract;
    myMetadataQString += "</td></tr>";
  
    // Layer Queryability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Can Identify");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += ((layersSupported[i].queryable) ? tr("Yes") : tr("No"));
    myMetadataQString += "</td></tr>";

    // Layer Opacity
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Can be Transparent");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += ((layersSupported[i].opaque) ? tr("No") : tr("Yes"));
    myMetadataQString += "</td></tr>";

    // Layer Subsetability
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Can Zoom In");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += ((layersSupported[i].noSubsets) ? tr("No") : tr("Yes"));
    myMetadataQString += "</td></tr>";

    // Layer Server Cascade Count
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Cascade Count");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].cascaded;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Width
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Fixed Width");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].fixedWidth;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Fixed Height");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += layersSupported[i].fixedHeight;
    myMetadataQString += "</td></tr>";

    // Layer Fixed Height
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("WGS 84 Bounding Box");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += extentForLayer[ layerName ].stringRep().toLocal8Bit().data();
    myMetadataQString += "</td></tr>";

    // Layer Coordinate Reference Systems
    for ( int j = 0; j < layersSupported[i].crs.size(); j++ )
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr("Available in CRS");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].crs[j];
      myMetadataQString += "</td></tr>";
    }

    // Layer Styles
    for (int j = 0; j < layersSupported[i].style.size(); j++)
    {
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr("Available in style");
      myMetadataQString += "</td>";
      myMetadataQString += "<td>";

      // Nested table.
      myMetadataQString += "<table width=\"100%\">";

      // Layer Style Name
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr("Name");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].name;
      myMetadataQString += "</td></tr>";

      // Layer Style Title
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr("Title");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"gray\">";
      myMetadataQString += layersSupported[i].style[j].title;
      myMetadataQString += "</td></tr>";

      // Layer Style Abstract
      myMetadataQString += "<tr><td bgcolor=\"gray\">";
      myMetadataQString += tr("Abstract");
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

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::getMetadata: exiting with '" << myMetadataQString.toLocal8Bit().data() << "'." << std::endl;
#endif

  return myMetadataQString;
}


QString QgsWmsProvider::identifyAsText(const QgsPoint& point)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::identifyAsText: entering." << std::endl;
#endif

  // Collect which layers to query on

  QStringList queryableLayers = QStringList();

  // Test for which layers are suitable for querying with
  for ( QStringList::const_iterator it  = activeSubLayers.begin(); 
                                    it != activeSubLayers.end(); 
                                  ++it )
  {
    // Is sublayer visible?
    if (TRUE == activeSubLayerVisibility.find( *it )->second)
    {
      // Is sublayer queryable?
      if (TRUE == mQueryableForLayer.find( *it )->second)
      {
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::identifyAsText: '" << (*it).toLocal8Bit().data() << "' is queryable." << std::endl;
#endif
        queryableLayers += *it;
      }
    }
  }

  QString layers = queryableLayers.join(",");
  Q3Url::encode( layers );

  // Compose request to WMS server

  QString requestUrl = mGetFeatureInfoUrlBase;

  requestUrl += "&";
  requestUrl += "QUERY_LAYERS=" + layers;
  requestUrl += "&";
   //! \todo Need to tie this into the options provided by GetCapabilities
  requestUrl += "INFO_FORMAT=text/plain";

// X,Y in WMS 1.1.1; I,J in WMS 1.3.0

//   requestUrl += "&";
//   requestUrl += QString( "I=%1" )
//                    .arg( point.x() );
//   requestUrl += "&";
//   requestUrl += QString( "J=%1" )
//                    .arg( point.y() );

  requestUrl += "&";
  requestUrl += QString( "X=%1" )
                   .arg( point.x() );
  requestUrl += "&";
  requestUrl += QString( "Y=%1" )
                   .arg( point.y() );

  QString text = retrieveUrl(requestUrl);

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::identifyAsText: exiting with '"
            << text.toLocal8Bit().data() << "'." << std::endl;
#endif
  return text;
}


QString QgsWmsProvider::errorCaptionString()
{
  return mErrorCaption;
}


QString QgsWmsProvider::errorString()
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::errorString: returning '" 
            << mError.toLocal8Bit().data() << "'." << std::endl;
#endif
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
QGISEXTERN QgsWmsProvider * classFactory(const QString *uri)
{
  return new QgsWmsProvider(*uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey(){
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
QGISEXTERN bool isProvider(){
  return true;
}

