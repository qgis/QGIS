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


/*
 * The following structs reflect the WMS XML schema,
 * as illustrated in Appendix E of the Web Map Service standard, version 1.3, 2004-08-02.
 */

  /** OnlineResource Attribute structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsOnlineResourceAttribute
  {
    QString xlinkHref;
  };

  /** Get Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsGetProperty
  {
    QgsWmsOnlineResourceAttribute onlineResource;
  };

  /** Post Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsPostProperty
  {
    QgsWmsOnlineResourceAttribute onlineResource;
  };

  /** HTTP Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsHttpProperty
  {
    QgsWmsGetProperty    get;
    QgsWmsPostProperty   post;  // can be null
  };

  /** DCP Type Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsDcpTypeProperty
  {
    QgsWmsHttpProperty http;
  };

  /** Operation Type structure (for GetMap and GetFeatureInfo) */
  // TODO: Fill to WMS specifications
  struct QgsWmsOperationType
  {
    std::vector<QString>                 format;
    std::vector<QgsWmsDcpTypeProperty>   dcpType;
  };

  /** Request Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsRequestProperty
  {
    // QgsWmsGetCapabilitiesProperty   ...
    // -- don't include since if we can get the capabilities,
    //    we already know what's in this part.
    QgsWmsOperationType     getMap;
    QgsWmsOperationType     getFeatureInfo;
  };

  /** Exception Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsExceptionProperty
  {
    std::vector<QString>                 format;   // text formats supported.
  };

  /** Primary Contact Person Property structure */
  struct QgsWmsContactPersonPrimaryProperty
  {
    QString            contactPerson;
    QString            contactOrganization;
  };

  /** Contact Address Property structure */
  struct QgsWmsContactAddressProperty
  {
    QString            addressType;
    QString            address;
    QString            city;
    QString            stateOrProvince;
    QString            postCode;
    QString            country;
  };

  /** Contact Information Property structure */
  struct QgsWmsContactInformationProperty
  {
    QgsWmsContactPersonPrimaryProperty contactPersonPrimary;
    QString                            contactPosition;
    QgsWmsContactAddressProperty       contactAddress;
    QString                            contactVoiceTelephone;
    QString                            contactFacsimileTelephone;
    QString                            contactElectronicMailAddress;
  };

  /** Service Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsServiceProperty
  {
    // QString                            name;  // Should always be "WMS"
    QString                            title;
    QString                            abstract;
    std::vector<QString>               keywordList;
    QgsWmsOnlineResourceAttribute      onlineResource;
    QgsWmsContactInformationProperty   contactInformation;
    QString                            fees;
    QString                            accessConstraints;
    uint                               layerLimit;
    uint                               maxWidth;
    uint                               maxHeight;
  };

  /** Bounding Box Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsBoundingBoxProperty
  {
    QString   crs;
    QgsRect   box;    // consumes minx, miny, maxx, maxy.
    double    resx;   // spatial resolution (in CRS units)
    double    resy;   // spatial resolution (in CRS units)
  };

  /** Dimension Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsDimensionProperty
  {
    QString   name;
    QString   units;
    QString   unitSymbol;
    QString   defaultValue;   // plain "default" is a reserved word
    bool      multipleValues;
    bool      nearestValue;
    bool      current;
  };

  /** Logo URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsLogoUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;

    int                             width;
    int                             height;
  };

  /** Attribution Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsAttributionProperty
  {
    QString                         title;
    QgsWmsOnlineResourceAttribute   onlineResource;
    QgsWmsLogoUrlProperty           logoUrl;
  };

  /** Legend URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsLegendUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;

    int                             width;
    int                             height;
  };

  /** StyleSheet URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsStyleSheetUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;
  };

  /** Style URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsStyleUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;
  };

  /** Style Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsStyleProperty
  {
    QString                               name;
    QString                               title;
    QString                               abstract;
    std::vector<QgsWmsLegendUrlProperty>  legendUrl;
    QgsWmsStyleSheetUrlProperty           styleSheetUrl;
    QgsWmsStyleUrlProperty                styleUrl;
  };

  /** Authority URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsAuthorityUrlProperty
  {
    QgsWmsOnlineResourceAttribute   onlineResource;
    QString                         name;             // XML "NMTOKEN" type
  };

  /** Identifier Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsIdentifierProperty
  {
    QString   authority;
  };

  /** Metadata URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsMetadataUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;
    QString                         type;             // XML "NMTOKEN" type
  };

  /** Data List URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsDataListUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;
  };

  /** Feature List URL Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsFeatureListUrlProperty
  {
    QString                         format;
    QgsWmsOnlineResourceAttribute   onlineResource;
  };

  /** Layer Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsLayerProperty
  {
    // WMS layer properties
    QString                                     name;
    QString                                     title;
    QString                                     abstract;
    std::vector<QString>                        keywordList;
    std::vector<QString>                        crs;        // coord ref sys
    QgsRect                                     ex_GeographicBoundingBox;
    std::vector<QgsWmsBoundingBoxProperty>      boundingBox;
    std::vector<QgsWmsDimensionProperty>        dimension;
    QgsWmsAttributionProperty                   attribution;
    std::vector<QgsWmsAuthorityUrlProperty>     authorityUrl;
    std::vector<QgsWmsIdentifierProperty>       identifier;
    std::vector<QgsWmsMetadataUrlProperty>      metadataUrl;
    std::vector<QgsWmsDataListUrlProperty>      dataListUrl;
    std::vector<QgsWmsFeatureListUrlProperty>   featureListUrl;
    std::vector<QgsWmsStyleProperty>            style;
    double                                      minScaleDenominator;
    double                                      maxScaleDenominator;
    std::vector<QgsWmsLayerProperty>            layer;      // nested layers

    // WMS layer attributes
    bool               queryable;
    int                cascaded;
    bool               opaque;
    bool               noSubsets;
    int                fixedWidth;
    int                fixedHeight;
  };

  /** Capability Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsCapabilityProperty
  {
    QgsWmsRequestProperty           request;
    QgsWmsExceptionProperty         exception;
    QgsWmsLayerProperty             layer;
  };

  /** Capabilities Property structure */
  // TODO: Fill to WMS specifications
  struct QgsWmsCapabilitiesProperty
  {
    QgsWmsServiceProperty           service;
    QgsWmsCapabilityProperty        capability;
    QString                         version;
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
  * \param   uri   HTTP URL of the Web Server, optionally followed by a space then the proxy host name,
  *                another space, and the proxy host port.  If no proxy is declared then we will
  *                contact the host directly.
  *
  */
  QgsWmsProvider(QString const & uri = 0);

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


  /** return the number of layers for the current data source

    @note 

    Should this be subLayerCount() instead?
  */
  size_t layerCount() const;

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
   * Ownership of the returned QImage remains with this provider and its lifetime
   * is guaranteed only until the next call to draw() or destruction of this provider.
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

  //! get raster formats supported by both the WMS Server and Qt
  std::vector<QString> supportedFormats();
  
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

  /**
   * Get metadata in a format suitable for feeding directly
   * into a subset of the GUI raster properties "Metadata" tab.
   */
  QString getMetadata();

    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString name() const;


    /** return description

    Return a terse string describing what the provider is.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString description() const;

  
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
  void parseCapabilities(QByteArray xml, QgsWmsCapabilitiesProperty& capabilitiesProperty);
  
  //! parse the WMS Service XML element
  void parseService(QDomElement e, QgsWmsServiceProperty& serviceProperty);

  //! parse the WMS Capability XML element
  void parseCapability(QDomElement e, QgsWmsCapabilityProperty& capabilityProperty);

  //! parse the WMS OnlineResource XML element
  void parseOnlineResource(QDomElement e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute);

  //! parse the WMS Get XML element
  void parseGet(QDomElement e, QgsWmsGetProperty& getProperty);

  //! parse the WMS Post XML element
  void parsePost(QDomElement e, QgsWmsPostProperty& postProperty);

  //! parse the WMS HTTP XML element
  void parseHttp(QDomElement e, QgsWmsHttpProperty& httpProperty);

  //! parse the WMS DCPType XML element
  void parseDcpType(QDomElement e, QgsWmsDcpTypeProperty& dcpType);

  //! parse the WMS GetCapabilities, GetMap, or GetFeatureInfo XML element, each of type "OperationType".
  void parseOperationType(QDomElement e, QgsWmsOperationType& operationType);

  //! parse the WMS Request XML element
  void parseRequest(QDomElement e, QgsWmsRequestProperty& requestProperty);

  //! parse the WMS Layer XML element
  // TODO: Make recursable
  void parseLayer(QDomElement e, QgsWmsLayerProperty& layerProperty);

  //! calculates the combined extent of the layers selected by layersDrawn  
  void calculateExtent();

  
  //! Data source URI of the WMS for this layer
  QString httpuri;

  //! HTTP proxy host name for the WMS for this layer
  QString httpproxyhost;

  //! HTTP proxy port number for the WMS for this layer
  Q_UINT16 httpproxyport;

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
   * Parsed capabilities of the WMS Server
   */
  QgsWmsCapabilitiesProperty capabilities;
  
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

  /**
   * The previously retrieved image from the WMS server.
   * This can be reused if draw() is called consecutively
   * with the same parameters.
   */
  QImage* cachedImage;

  /**
   * The previous parameter to draw().
   */
  QgsRect cachedViewExtent;

  /**
   * The previous parameter to draw().
   */
  int cachedPixelWidth;

  /**
   * The previous parameter to draw().
   */
  int cachedPixelHeight;

};

#endif
