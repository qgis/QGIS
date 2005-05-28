/***************************************************************************
      qgswmsprovider.h  -  QGIS Data provider for 
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

#ifndef QGSWMSPROVIDER_H
#define QGSWMSPROVIDER_H

#include <map>
#include <vector>

#include "../../src/qgsrasterdataprovider.h"
#include "../../src/qgsrect.h"

#include "qgsdatasourceuri.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qhttp.h>
#include <qdom.h>


  /** Layer Property structure */  
  // TODO: Fill to WMS specifications
  struct QgsWmsLayerProperty
  {
    // WMS layer properties
    QString name;
    QString title;
    QString abstract;
    QString keywordlist;
    QString style;
    QString srs;
    QgsRect latlonbbox;
    
    // WMS layer attributes
    bool    queryable;
    int     cascaded;
    bool    opaque;
    bool    nosubsets;
    int     fixedwidth;
    int     fixedheight;
    
  };


/**

  \brief Data provider for OGC WMS layers.
 
  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a OGC Web Map Service.
  
  TODO: Make it work
  
*/
class QgsWmsProvider : public QgsRasterDataProvider
{
  
  Q_OBJECT

public:
  
 
  /**
  * Constructor for the provider. 
  *
  * \param   uri   Address of the WMS server.
  *
  */
  QgsWmsProvider(QString uri = 0);

  //! Destructor
  virtual ~QgsWmsProvider();

  // TODO: Document this better, make static
  /** \brief   Returns a list of the supported layers of this WMS
   */
  virtual std::vector<QgsWmsLayerProperty> supportedLayers();

  /**
   * Add the list of WMS layer names to be rendered by this server
   */
  void addLayers(QStringList layers);

  /**
   * Reorder the list of WMS layer names to be rendered by this server
   * (in order from bottom to top)
   * \note   layers must have been previously added.
   */
  void setLayerOrder(QStringList layers);

  /**
   * Set the visibility of the given sublayer name
   */
  void setSubLayerVisibility(QString name, bool vis);

        
  // TODO: Document this better.
  /** \brief   Renders the layer as an image
   * TODO: Add pixel depth parameter (intended to match the display or printer device)
   */
  QImage* draw(QgsRect viewExtent, int pixelWidth, int pixelHeight);

  
  /** Experimental function only **/
  void getServerCapabilities();
  
  /* Example URI: http://ims.cr.usgs.gov:80/servlet/com.esri.wms.Esrimap/USGS_EDC_Trans_BTS_Roads?SERVICE=WMS&REQUEST=GetCapabilities */

  /** Used to ask the layer for its projection as a WKT string. Implements
   * virtual method of same name in QgsDataProvider. */
  QString getProjectionWKT()  {return QString("Not implemented yet");} ;

  /**
   * Get the data source URI structure used by this layer
   */
  QgsDataSourceURI * getURI();

  /**
   * Set the data source URI used by this layer
   */
  void setURI(QgsDataSourceURI * uri);

  /** Return the extent for this data layer
  */
  virtual QgsRect *extent();

  /** Reset the layer - for a PostgreSQL layer, this means clearing the PQresult
   * pointer and setting it to 0
   */
  void reset();

  /**Returns true if layer is valid
  */
  bool isValid();

  //! get WMS Server version string
  QString wmsVersion();

  //! get raster formats supported
  std::list<QString> formatsSupported();
  
  /* Included here because qgsdataprovider.h insisted */
  void setDataSourceUri(QString)
  {}

  /* Included here because qgsdataprovider.h insisted */
  QString getDataSourceUri()
  {}

  /**
   * Sub-layers handled by this provider, in order from bottom to top
   *
   * Sub-layers are used to abstract the way the WMS server can combine
   * layers in some way at the server, before it serves them to this
   * WMS client.
   */
  QStringList subLayers();

  // TODO: Get the WMS connection
  
  // TODO: Get the table name associated with this provider instance

  /**Returns a bitmask containing the supported capabilities*/
  // int capabilities() const;

  
signals:

    /** \brief emit a signal to notify of a progress event */
    void setProgress(int theProgress, int theTotalSteps);

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void setStatus(QString theStatusQString);

    
public slots:
  
  void showStatusMessage(QString theMessage);
 
    
private:

  /**
   * Retrieve and parse the (cached) Capabilities document from the server
   *
   * When this returns, "layers" will make sense.
   *
   * TODO: Make network-timeout tolerant
   */ 
  void retrieveServerCapabilities();


  //! Test function: see if we can download a WMS' capabilites
  void downloadCapabilitiesURI(QString uri);

  //! Test function: see if we can download a map from a WMS
  void drawTest(QString uri);
  
  //! Test function: see if we can parse a WMS' capabilites
  void parseCapabilities(QByteArray xml);
  
  //! parse the WMS Capability XML element
  void parseCapability(QDomElement e);

  //! parse the WMS Layer XML element
  // TODO: Make recursable
  void parseLayer(QDomElement e, QgsWmsLayerProperty& layerproperty);

  //! calculates the combined extent of the layers selected by layersDrawn  
  void calculateExtent();

  
  //! Data source URI of the WMS for this layer
  QString httpuri;

  /**
   * Flag indicating if the layer data source is a valid WMS layer
   */
  bool valid;
  
  /**
   * Spatial reference id of the layer
   */
  QString srid;
  
  /**
   * Rectangle that contains the extent (bounding box) of the layer
   */
  QgsRect layerExtent;
    
  /**
   * Capabilities of the WMS Server (raw)
   */
  QByteArray httpcapabilitiesresponse;
  
  /**
   * Capabilities of the WMS Server
   */
  QDomDocument capabilitiesDOM;
  
  /**
   * layers hosted by the WMS Server
   */
  std::vector<QgsWmsLayerProperty> layersSupported;

  /**
   * extents per layer
   */
  std::map<QString, QgsRect> extentForLayer;
  
  /**
   * Active sublayers managed by this provider in a draw function, in order from bottom to top
   * (some may not be visible in a draw function, cf. activeSubLayerVisibility)
   */
  QStringList activeSubLayers;

  /**
   * Visibility status of the given active sublayer
   */
  std::map<QString, bool> activeSubLayerVisibility;

};

#endif
