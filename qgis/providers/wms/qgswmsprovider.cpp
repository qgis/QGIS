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

#include <fstream>
#include <iostream>

#include "../../src/qgsrect.h"
#include "qgshttptransaction.h"

#include "qgswmsprovider.h"

#include <qnetwork.h>
#include <qhttp.h>
#include <qurl.h>
#include <qmessagebox.h>

#ifdef QGISDEBUG
#include <qfile.h>
#endif

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

QgsWmsProvider::QgsWmsProvider(QString uri)
  : httpuri(uri),
    httpproxyhost(0),
    httpproxyport(80),
    httpcapabilitiesresponse(0)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider: constructing with uri '" << uri << "'." << std::endl;
#endif
  
  // assume this is a valid layer until we determine otherwise
  valid = true;
  
  /* GetCapabilities routine.
     Once proven, we'll move to its own function
  */   
  //httpuri = "http://ims.cr.usgs.gov:80/servlet/com.esri.wms.Esrimap/USGS_EDC_Trans_BTS_Roads?SERVICE=WMS&REQUEST=GetCapabilities";

  //httpuri = "http://www.ga.gov.au/bin/getmap.pl?dataset=national&";
  

  getServerCapabilities();
  

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
  

}


std::vector<QgsWmsLayerProperty> QgsWmsProvider::supportedLayers()
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::supportedLayers: Entering." << std::endl;
#endif

  // Allow the provider to collect the capabilities first.
  retrieveServerCapabilities();

  return layersSupported;

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::supportedLayers: Exiting." << std::endl;
#endif

}





size_t QgsWmsProvider::layerCount() const
{
    return 1;                   // XXX properly return actual number of layers
} // QgsWmsProvider::layerCount()




void QgsWmsProvider::addLayers(QStringList layers)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: Entering" <<
                  " with layer list of " << layers.join(", ") << std::endl;
#endif

  // TODO: Make activeSubLayers a std::map in order to avoid duplicates

  activeSubLayers += layers;
  
  // Set the visibility of these new layers on by default
  for ( QStringList::Iterator it  = layers.begin(); 
                              it != layers.end(); 
                            ++it ) 
  {

    activeSubLayerVisibility[*it] = TRUE;
 
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: set visibility of layer '" << *it << "' to TRUE." << std::endl;
#endif
  }

  // now that the layers have changed, the extent will as well.
  calculateExtent();

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::addLayers: Exiting." << std::endl;
#endif
}
      

void QgsWmsProvider::setLayerOrder(QStringList layers)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setLayerOrder: Entering." << std::endl;
#endif
  
  activeSubLayers = layers;
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::setLayerOrder: Exiting." << std::endl;
#endif
}


void QgsWmsProvider::setSubLayerVisibility(QString name, bool vis)
{
  activeSubLayerVisibility[name] = vis;
}


QImage* QgsWmsProvider::draw(QgsRect viewExtent, int pixelWidth, int pixelHeight)
{

  QImage* i;
  

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


  // Bounding box in WMS format
  QString bbox;
  bbox = QString("%1,%2,%3,%4").
          arg(viewExtent.xMin()).
          arg(viewExtent.yMin()).
          arg(viewExtent.xMax()).
          arg(viewExtent.yMax());
          
  // Width in WMS format
  QString width;
  width = width.setNum(pixelWidth);

  // Height in WMS format
  QString height;
  height = height.setNum(pixelHeight);

  // Split proxy from the provider-encoded uri  
  //   ( url ( + " " + proxyhost + " " + proxyport) )
  
  QStringList drawuriparts = QStringList::split(" ", httpuri, TRUE);
  
  QString drawuri = drawuriparts.front();
  drawuriparts.pop_front();
  
  if (drawuriparts.count())
  {
    httpproxyhost = drawuriparts.front();
    drawuriparts.pop_front();
    
    if (drawuriparts.count())
    {
      bool * ushortConversionOK;
      httpproxyport = drawuriparts.front().toUShort(ushortConversionOK);
      drawuriparts.pop_front();
      
      if (!(*ushortConversionOK))
      {
        httpproxyport = 80;  // standard HTTP port
      }
    }
    else
    {
      httpproxyport = 80;  // standard HTTP port
    }
  }    
  
  // Calculate active layers that are also visible.

  QStringList visibleLayers = QStringList();
    
  for ( QStringList::Iterator it  = activeSubLayers.begin(); 
                              it != activeSubLayers.end(); 
                            ++it ) 
  {
    if (TRUE == activeSubLayerVisibility.find( *it )->second)
    {
      visibleLayers += *it;
    }
  }
    
  QString layers = visibleLayers.join(",");
  QUrl::encode( layers );
  
  
  // compose the URL query string for the WMS server.
  
  drawuri += "Service=WMS";
  drawuri += "&";
  drawuri += "Version=1.1.0";
  drawuri += "&";
  drawuri += "Request=GetMap";
  drawuri += "&";
  drawuri += "BBox=" + bbox;
  drawuri += "&";
  drawuri += "SRS=EPSG:4326";
  drawuri += "&";
  drawuri += "Width=" + width;
  drawuri += "&";
  drawuri += "Height=" + height;
  drawuri += "&";
  drawuri += "Layers=" + layers;
  drawuri += "&";
  drawuri += "Styles=";
  drawuri += "&";
  drawuri += "Format=image/png";

  emit setStatus( QString("Test from QgsWmsProvider") );

  
  QgsHttpTransaction http(drawuri, httpproxyhost, httpproxyport);
  
  
  // Do a passthrough for the status bar text
  connect(
          &http, SIGNAL(setStatus        (QString)),
           this,   SLOT(showStatusMessage(QString))
         );
  


  // TODO: Check Content-Type is correct
  QByteArray imagesource = http.getSynchronously();
  
  // TODO: Bail gracefully if we get a "application/vnd.ogc.se_xml" error
  
  if (http.responseContentType() == "application/vnd.ogc.se_xml")
  {
    // We had a Service Exception from the WMS
    
    QMessageBox::critical(0,
      "WMS Service Exception",
      imagesource);

    
  }
  
  
#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::draw: Response received." << std::endl;
#endif

#ifdef QGISDEBUG
  QFile file( "/tmp/qgis-wmsprovider-draw-raw.png" );
  if ( file.open( IO_WriteOnly ) ) 
  {
    file.writeBlock(imagesource);
    file.close();
  }
#endif

  //bool success = i.loadFromData(imagesource);
  i = new QImage(imagesource);

#ifdef QGISDEBUG
  // Get what we can support
  
  QStringList list = i->inputFormatList();
  QStringList::Iterator it = list.begin();
  while( it != list.end() )
  {
    std::cout << "QgsWmsProvider::addLayers: can support input of '" << *it << "'." << std::endl;
    ++it;
  }
#endif

  
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::draw: Exiting." << std::endl;
#endif


  // TODO: bit depth conversion to the client's expectation
//  return *(i.convertDepth(32));
  return i;

}


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


void QgsWmsProvider::retrieveServerCapabilities()
{

  // TODO: Smarter caching - need to refresh if demanded.

  if ( httpcapabilitiesresponse.isNull() )
  {
  
    QString uri = httpuri + "SERVICE=WMS&REQUEST=GetCapabilities";

    QgsHttpTransaction http(uri, httpproxyhost, httpproxyport);

    httpcapabilitiesresponse = http.getSynchronously();

  
#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::getServerCapabilities: Converting to DOM." << std::endl;
#endif
  
    parseCapabilities(httpcapabilitiesresponse);

  }
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::getServerCapabilities: exiting." << std::endl;
#endif

}

// private
void QgsWmsProvider::downloadCapabilitiesURI(QString uri)
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: Entered with '" << uri << "'" << std::endl;
#endif

  QgsHttpTransaction http(uri, httpproxyhost, httpproxyport);

  httpcapabilitiesresponse = http.getSynchronously();

  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: Converting to DOM." << std::endl;
#endif
  
  parseCapabilities(httpcapabilitiesresponse);


  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadCapabilitiesURI: exiting." << std::endl;
#endif

}


// private
void QgsWmsProvider::drawTest(QString uri)
{

/*

Example URL (?):
"http://www.ga.gov.au/bin/getmap.pl?dataset=national&VERSION=1.3.0&REQUEST=GetMap&LAYERS=d02&STYLES=&CRS=EPSG:3112&BBOX=151,-28,153,-26&WIDTH=400&HEIGHT=400&FORMAT=image/png"

Example URL (works!)
"http://www.ga.gov.au/bin/getmap.pl?dataset=national&Service=WMS&Version=1.1.0&Request=GetMap&BBox=130,-40,160,-10&SRS=EPSG:4326&Width=400&Height=400&Layers=railways&Format=image/png"

*/

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::drawTest: Entered with '" << uri << "'" << std::endl;
#endif
  
  QgsHttpTransaction http(uri, httpproxyhost, httpproxyport);

  http.getSynchronously();

  
#ifdef QGISDEBUG
    std::cout << "QgsWmsProvider::drawTest: Response received." << std::endl;
#endif
//   


  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::downloadMapURI: exiting." << std::endl;
#endif

}


void QgsWmsProvider::parseCapabilities(QByteArray xml)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapabilities: entering." << std::endl;
  
  QFile file( "/tmp/qgis-wmsprovider-capabilities.xml" );
  if ( file.open( IO_WriteOnly ) ) 
  {
    file.writeBlock(xml);
    file.close();
  }
#endif
  
  // Convert completed document into a DOM
  capabilitiesDOM.setContent(xml);

  QDomElement docElem = capabilitiesDOM.documentElement();
  
  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() ) {
          //std::cout << e.tagName() << std::endl; // the node really is an element.
          
          if (e.tagName() == "Capability")
          {
            parseCapability(e);
          }  
      
      }
      n = n.nextSibling();
  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapabilities: exiting." << std::endl;
#endif


}


void QgsWmsProvider::parseCapability(QDomElement e)
{
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapability: entering." << std::endl;
#endif
  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          //std::cout << "  " << e1.tagName() << std::endl; // the node really is an element.
      
          if (e1.tagName() == "Layer")
          {
            // TODO: Initialise this variable
            QgsWmsLayerProperty layerproperty = {0};
            
            parseLayer(e1, layerproperty);
          }  
      
      }
      n1 = n1.nextSibling();
  }    

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::parseCapability: exiting." << std::endl;
#endif

}


void QgsWmsProvider::parseLayer(QDomElement e, QgsWmsLayerProperty& layerproperty)
{
#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseLayer: entering." << std::endl;
#endif

  // enforce WMS non-inheritance rules
  layerproperty.name =        QString::null;
  layerproperty.title =       QString::null;
  layerproperty.abstract =    QString::null;
  layerproperty.keywordlist = QString::null;

  // assume true until we find a child layer
  bool atleaf = TRUE;
  
  QDomNode n1 = e.firstChild();
  while( !n1.isNull() ) {
      QDomElement e1 = n1.toElement(); // try to convert the node to an element.
      if( !e1.isNull() ) {
          //std::cout << "    " << e1.tagName() << std::endl; // the node really is an element.
      
          if      (e1.tagName() == "Layer")
          {
//            std::cout << "      Nested layer." << std::endl; 

            parseLayer(e1, layerproperty);
            atleaf = FALSE;
          }
          else if (e1.tagName() == "Name")
          {
//            std::cout << "      Name is: '" << e1.text() << "'." << std::endl;
            layerproperty.name = e1.text();
          }
          else if (e1.tagName() == "Title")
          {
//            std::cout << "      Title is: '" << e1.text() << "'." << std::endl;
            layerproperty.title = e1.text();
          }  
          else if (e1.tagName() == "SRS")
          {
//            std::cout << "      SRS is: '" << e1.text() << "'." << std::endl;
            layerproperty.srs = e1.text();
          }  
          else if (e1.tagName() == "LatLonBoundingBox")
          {
            
//            std::cout << "      LLBB is: '" << e1.attribute("minx") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("miny") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("maxx") << "'." << std::endl;
//            std::cout << "      LLBB is: '" << e1.attribute("maxy") << "'." << std::endl;
            
            layerproperty.latlonbbox = QgsRect( 
                                                e1.attribute("minx").toDouble(),
                                                e1.attribute("miny").toDouble(),
                                                e1.attribute("maxx").toDouble(),
                                                e1.attribute("maxy").toDouble()
                                              );  
          }  
      
      }
      n1 = n1.nextSibling();
  }    
  
  if (atleaf)
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere
    
#ifdef QGISDEBUG
//    std::cout << "QgsWmsProvider::parseLayer: A layer definition is complete." << std::endl;

    std::cout << "QgsWmsProvider::parseLayer:   name is: '" << layerproperty.name << "'." << std::endl;
    std::cout << " QgsWmsProvider::parseLayer:  title is: '" << layerproperty.title << "'." << std::endl;
//    std::cout << "QgsWmsProvider::parseLayer:   srs is: '" << layerproperty.srs << "'." << std::endl;
//    std::cout << "QgsWmsProvider::parseLayer:   bbox is: '" << layerproperty.latlonbbox.stringRep() << "'." << std::endl;
    
    // Store the extent so that it can be combined with others later
    // in calculateExtent()
    extentForLayer[ layerproperty.name ] = layerproperty.latlonbbox;
    
    // Insert into the local class' registry
    layersSupported.push_back(layerproperty);
       

#endif
    
  }
  
#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::parseLayer: exiting." << std::endl;
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

std::list<QString> QgsWmsProvider::formatsSupported()
{
  // TODO
} 

  
QStringList QgsWmsProvider::subLayers()
{
  return activeSubLayers;
}


/*
int QgsWmsProvider::capabilities() const
{
    return ( QgsVectorDataProvider::AddFeatures | 
	     QgsVectorDataProvider::DeleteFeatures |
	     QgsVectorDataProvider::ChangeAttributeValues |
	     QgsVectorDataProvider::AddAttributes |
	     QgsVectorDataProvider::DeleteAttributes );
}
*/


void QgsWmsProvider::showStatusMessage(QString theMessage)
{

#ifdef QGISDEBUG
//  std::cout << "QgsWmsProvider::showStatusMessage: entered with '" << theMessage << "'." << std::endl;
#endif

    // Pass-through
    // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
    emit setStatus(theMessage);
}


void QgsWmsProvider::calculateExtent()
{

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: entered." << std::endl;
#endif

  for ( QStringList::Iterator it  = activeSubLayers.begin(); 
                              it != activeSubLayers.end(); 
                            ++it ) 
  {
  
    // This is the extent for the layer name in *it
    QgsRect extent = extentForLayer.find( *it )->second;
    
    if ( it == activeSubLayers.begin() )
    {
      layerExtent = extent;
    }
    else
    {
      layerExtent.combineExtentWith( &extent );
    }
  
#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: combined extent is '" << 
               layerExtent.stringRep() << "' after '" << *it << "'." << std::endl;
#endif

  }

#ifdef QGISDEBUG
  std::cout << "QgsWmsProvider::calculateExtent: exiting with '" << layerExtent.stringRep() << "'." << std::endl;
#endif

}

   
/**
 * Class factory to return a pointer to a newly created 
 * QgsWmsProvider object
 */
QGISEXTERN QgsWmsProvider * classFactory(const char *uri)
{
  return new QgsWmsProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey(){
  return QString("wms");
}
/**
 * Required description function 
 */
QGISEXTERN QString description(){
  return QString("OGC Web Map Service data provider");
} 
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider(){
  return true;
}

