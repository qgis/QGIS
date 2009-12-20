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

#include "qgsrasterdataprovider.h"
#include "qgsrectangle.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>

class QgsCoordinateTransform;

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
  QStringList                      format;
  QVector<QgsWmsDcpTypeProperty>   dcpType;
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
  QStringList                 format;   // text formats supported.
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
  QStringList                        keywordList;
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
  QgsRectangle   box;    // consumes minx, miny, maxx, maxy.
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
  QString                           name;
  QString                           title;
  QString                           abstract;
  QVector<QgsWmsLegendUrlProperty>  legendUrl;
  QgsWmsStyleSheetUrlProperty       styleSheetUrl;
  QgsWmsStyleUrlProperty            styleUrl;
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
  int                                     orderId;
  QString                                 name;
  QString                                 title;
  QString                                 abstract;
  QStringList                             keywordList;
  QVector<QString>                        crs;        // coord ref sys
  QgsRectangle                            ex_GeographicBoundingBox;
  QVector<QgsWmsBoundingBoxProperty>      boundingBox;
  QVector<QgsWmsDimensionProperty>        dimension;
  QgsWmsAttributionProperty               attribution;
  QVector<QgsWmsAuthorityUrlProperty>     authorityUrl;
  QVector<QgsWmsIdentifierProperty>       identifier;
  QVector<QgsWmsMetadataUrlProperty>      metadataUrl;
  QVector<QgsWmsDataListUrlProperty>      dataListUrl;
  QVector<QgsWmsFeatureListUrlProperty>   featureListUrl;
  QVector<QgsWmsStyleProperty>            style;
  double                                  minimumScaleDenominator;
  double                                  maximumScaleDenominator;
  QVector<QgsWmsLayerProperty>            layer;      // nested layers

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

*/
class QgsWmsProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    /**
    * Constructor for the provider.
    *
    * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
    *                otherwise we contact the host directly.
    *
    */
    QgsWmsProvider( QString const & uri = 0 );

    //! Destructor
    virtual ~QgsWmsProvider();

    /**
     * \brief   Returns a list of the supported layers of the WMS server
     *
     * \param[out] layers   The list of layers will be placed here.
     *
     * \retval FALSE if the layers could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * \todo Document this better, make static
     */
    virtual bool supportedLayers( QVector<QgsWmsLayerProperty> & layers );

    /**
     * \brief   Returns a map for the hierarchy of layers
     */
    virtual void layerParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const;

    // TODO: Document this better
    /** \brief   Returns a list of the supported CRSs of the given layers
     *  \note    Since WMS can specify CRSs per layer, this may change depending
                 on which layers are specified!  The "lowest common denominator" between
                 all specified layers is returned.
     */
    virtual QSet<QString> supportedCrsForLayers( QStringList const & layers );

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Add the list of WMS layer names to be rendered by this server
     */
    void addLayers( QStringList const &  layers,
                    QStringList const &  styles = QStringList() );


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
    void setLayerOrder( QStringList const & layers );

    /**
     * Set the visibility of the given sublayer name
     */
    void setSubLayerVisibility( QString const & name, bool vis );

    /**
     * Get the image encoding (as a MIME type) used in the transfer from the WMS server
     */
    QString imageEncoding() const;

    /**
     * Set the image encoding (as a MIME type) used in the transfer from the WMS server
     */
    void setImageEncoding( QString const & mimeType );

    /**
     * Set the image projection (in WMS CRS format) used in the transfer from the WMS server
     *
     * \note an empty crs value will result in the previous CRS being retained.
     */
    void setImageCrs( QString const & crs );

    /**
     * Set the name of the connection for use in authentication where required
     * \note added in 1.1
     */
    void setConnectionName( QString const & connName );

    // TODO: Document this better.
    /** \brief   Renders the layer as an image
     *
     *  \return  A QImage - if the attempt to retrieve data for the draw was unsuccessful, returns 0
     *           and more information can be found in lastError() and lastErrorTitle()
     *
     *  \todo    Add pixel depth parameter (intended to match the display or printer device)
     *
     *  \note    Ownership of the returned QImage remains with this provider and its lifetime
     *           is guaranteed only until the next call to draw() or destruction of this provider.
     *
     *  \warning A pointer to an QImage is used, as a plain QImage seems to have difficulty being
     *           shared across library boundaries
     */
    QImage * draw( QgsRectangle const &  viewExtent, int pixelWidth, int pixelHeight );


//  /** Experimental function only **/
//  void getServerCapabilities();

    /* Example URI: http://ims.cr.usgs.gov:80/servlet/com.esri.wms.Esrimap/USGS_EDC_Trans_BTS_Roads?SERVICE=WMS&REQUEST=GetCapabilities */

    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent();

    /** Reset the layer - for a PostgreSQL layer, this means clearing the PQresult
     * pointer and setting it to 0
     */
    void begin();

    /**Returns true if layer is valid
    */
    bool isValid();

    //! get WMS Server version string
    QString wmsVersion();

    //! get raster image encodings supported by the WMS Server, expressed as MIME types
    QStringList supportedImageEncodings();

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used to abstract the way the WMS server can combine
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayers() const;

    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the WMS server can symbolise
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayerStyles() const;


    // TODO: Get the WMS connection

    // TODO: Get the table name associated with this provider instance

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on which
        sublayers are visible on this provider, so it may
        be prudent to check this value per intended operation.
      */
    int capabilities() const;

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    QString metadata();


    /**
     * \brief Identify details from a WMS Server from the last screen update
     *
     * \param point[in]  The pixel coordinate (as it was displayed locally on screen)
     *
     * \return  A text document containing the return from the WMS server
     *
     * \note WMS Servers prefer to receive coordinates in image space, therefore
     *       this function expects coordinates in that format.
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     */
    QString identifyAsText( const QgsPoint& point );

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString lastErrorTitle();

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */

    QString lastError();

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
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );


  public slots:

    void showStatusMessage( QString const &  theMessage );


  private:

    /**
     * \brief Retrieve and parse the (cached) Capabilities document from the server
     *
     * \param forceRefresh  if true, ignores any previous response cached in memory
     *                      and always contact the server for a new copy.
     * \retval FALSE if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( bool forceRefresh = FALSE );

    /**
     * \brief Common URL retreival code for the differing WMS request types
     *
     * \retval 0 if an error occured - use lastError() and lastErrorTitle() for details
     *
     */
    QByteArray retrieveUrl( QString url );

    /*
      //! Test function: see if we can download a WMS' capabilities
      //! \retval FALSE if the download failed in some way
      bool downloadCapabilitiesURI(QString const &  uri);
    */

    //! \return FALSE if the capabilities document could not be parsed - see lastError() for more info
    bool parseCapabilitiesDom( QByteArray const & xml, QgsWmsCapabilitiesProperty& capabilitiesProperty );

    //! parse the WMS Service XML element
    void parseService( QDomElement const & e, QgsWmsServiceProperty& serviceProperty );

    //! parse the WMS Capability XML element
    void parseCapability( QDomElement const & e, QgsWmsCapabilityProperty& capabilityProperty );

    //! parse the WMS ContactPersonPrimary XML element
    void parseContactPersonPrimary( QDomElement const & e, QgsWmsContactPersonPrimaryProperty&
                                    contactPersonPrimaryProperty );

    //! parse the WMS ContactAddress XML element
    void parseContactAddress( QDomElement const & e, QgsWmsContactAddressProperty& contactAddressProperty );

    //! parse the WMS ContactInformation XML element
    void parseContactInformation( QDomElement const & e, QgsWmsContactInformationProperty&
                                  contactInformationProperty );

    //! parse the WMS OnlineResource XML element
    void parseOnlineResource( QDomElement const & e, QgsWmsOnlineResourceAttribute& onlineResourceAttribute );

    //! parse the WMS KeywordList XML element
    void parseKeywordList( QDomElement const & e, QStringList& keywordListProperty );

    //! parse the WMS Get XML element
    void parseGet( QDomElement const & e, QgsWmsGetProperty& getProperty );

    //! parse the WMS Post XML element
    void parsePost( QDomElement const & e, QgsWmsPostProperty& postProperty );

    //! parse the WMS HTTP XML element
    void parseHttp( QDomElement const & e, QgsWmsHttpProperty& httpProperty );

    //! parse the WMS DCPType XML element
    void parseDcpType( QDomElement const & e, QgsWmsDcpTypeProperty& dcpType );

    //! parse the WMS GetCapabilities, GetMap, or GetFeatureInfo XML element, each of type "OperationType".
    void parseOperationType( QDomElement const & e, QgsWmsOperationType& operationType );

    //! parse the WMS Request XML element
    void parseRequest( QDomElement const & e, QgsWmsRequestProperty& requestProperty );

    //! parse the WMS Legend URL XML element
    void parseLegendUrl( QDomElement const & e, QgsWmsLegendUrlProperty& legendUrlProperty );

    //! parse the WMS Style XML element
    void parseStyle( QDomElement const & e, QgsWmsStyleProperty& styleProperty );

    //! parse the WMS Layer XML element
    // TODO: Make recursable
    void parseLayer( QDomElement const & e, QgsWmsLayerProperty& layerProperty,
                     QgsWmsLayerProperty *parentProperty = 0 );

    /**
     * \brief parse the full WMS ServiceExceptionReport XML document
     *
     * \note mErrorCaption and mError are updated to suit the results of this function.
     */
    bool parseServiceExceptionReportDom( QByteArray const & xml );

    //! parse the WMS ServiceException XML element
    void parseServiceException( QDomElement const & e );


    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \retval FALSE if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent();

    /**
     * \brief Check for authentication information contained in the uri,
     * stripping and saving the username and password if present.
     *
     * \param uri uri to check
     *
     * \note added in 1.1
     */

    void setAuthentication( QString uri );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    QString prepareUri( QString uri );

    //! Data source URI of the WMS for this layer
    QString httpuri;

    //! Name of the stored connection
    QString connectionName;

    //! URL part of URI (httpuri)
    QString baseUrl;

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
    QgsRectangle layerExtent;

    /**
     * Capabilities of the WMS Server (raw)
     */
    QByteArray httpcapabilitiesresponse;

    /**
     * Capabilities of the WMS Server
     */
    QDomDocument capabilitiesDom;

    /**
     * Last Service Exception Report from the WMS Server
     */
    QDomDocument serviceExceptionReportDom;

    /**
     * Parsed capabilities of the WMS Server
     */
    QgsWmsCapabilitiesProperty mCapabilities;

    /**
     * layers hosted by the WMS Server
     */
    QVector<QgsWmsLayerProperty> layersSupported;

    /**
     * extents per layer (in WMS CRS:84 datum)
     */
    QMap<QString, QgsRectangle> extentForLayer;

    /**
     * available CRSs per layer
     */
    QMap<QString, QVector<QString> > crsForLayer;

    /**
     * WMS "queryable" per layer
     * Used in determining if the Identify map tool can be useful on the rendered WMS map layer.
     */
    QMap<QString, bool> mQueryableForLayer;

    /**
     * Active sublayers managed by this provider in a draw function, in order from bottom to top
     * (some may not be visible in a draw function, cf. activeSubLayerVisibility)
     */
    QStringList activeSubLayers;

    QStringList activeSubStyles;

    /**
     * Visibility status of the given active sublayer
     */
    QMap<QString, bool> activeSubLayerVisibility;

    /**
     * MIME type of the image encoding used from the WMS server
     */
    QString imageMimeType;

    /**
     * WMS CRS type of the image CRS used from the WMS server
     */
    QString imageCrs;

    /**
     * The previously retrieved image from the WMS server.
     * This can be reused if draw() is called consecutively
     * with the same parameters.
     */
    QImage* cachedImage;

    /**
     * The previous parameter to draw().
     */
    QgsRectangle cachedViewExtent;

    /**
     * The previous parameter to draw().
     */
    int cachedPixelWidth;

    /**
     * The previous parameter to draw().
     */
    int cachedPixelHeight;

    /**
     * The error caption associated with the last WMS error.
     */
    QString mErrorCaption;

    /**
     * The error message associated with the last WMS error.
     */
    QString mError;

    //! A QgsCoordinateTransform is used for transformation of WMS layer extents
    QgsCoordinateTransform * mCoordinateTransform;

    //! See if calculateExtents() needs to be called before extent() returns useful data
    bool extentDirty;

    //! Base URL for WMS GetFeatureInfo requests
    QString mGetFeatureInfoUrlBase;

    int mLayerCount;
    QMap<int, int> mLayerParents;
    QMap<int, QStringList> mLayerParentNames;

    //! Username for basic http authentication
    QString mUserName;

    //! Password for basic http authentication
    QString mPassword;

};

#endif

// ENDS
