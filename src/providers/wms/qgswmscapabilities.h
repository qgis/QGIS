/***************************************************************************
    qgswmscapabilities.h
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
#ifndef QGSWMSCAPABILITIES_H
#define QGSWMSCAPABILITIES_H

#include <QHash>
#include <QMap>
#include <QNetworkRequest>
#include <QStringList>
#include <QVector>

#include "qgsauthmanager.h"
#include "qgsraster.h"
#include "qgsrectangle.h"
#include "qgsrasteriterator.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"


class QNetworkReply;

/*
 * The following structs reflect the WMS XML schema,
 * as illustrated in Appendix E of the Web Map Service standard, version 1.3, 2004-08-02.
 */

//! OnlineResource Attribute structure
// TODO: Fill to WMS specifications
struct QgsWmsOnlineResourceAttribute
{
  QString xlinkHref;
};

//! Gets Property structure
// TODO: Fill to WMS specifications
struct QgsWmsGetProperty
{
  QgsWmsOnlineResourceAttribute onlineResource;
};

//! Post Property structure
// TODO: Fill to WMS specifications
struct QgsWmsPostProperty
{
  QgsWmsOnlineResourceAttribute onlineResource;
};

//! HTTP Property structure
// TODO: Fill to WMS specifications
struct QgsWmsHttpProperty
{
  QgsWmsGetProperty    get;
  QgsWmsPostProperty   post;  // can be null
};

//! DCP Type Property structure
// TODO: Fill to WMS specifications
struct QgsWmsDcpTypeProperty
{
  QgsWmsHttpProperty http;
};

//! Operation Type structure (for GetMap and GetFeatureInfo)
// TODO: Fill to WMS specifications
struct QgsWmsOperationType
{
  QStringList                      format;
  QVector<QgsWmsDcpTypeProperty>   dcpType;
  QStringList                      allowedEncodings;
};

//! Request Property structure
// TODO: Fill to WMS specifications
struct QgsWmsRequestProperty
{
  // QgsWmsGetCapabilitiesProperty   ...
  // -- don't include since if we can get the capabilities,
  //    we already know what's in this part.
  QgsWmsOperationType     getMap;
  QgsWmsOperationType     getFeatureInfo;
  QgsWmsOperationType     getTile;
  QgsWmsOperationType     getLegendGraphic;
};

//! Exception Property structure
// TODO: Fill to WMS specifications
struct QgsWmsExceptionProperty
{
  QStringList        format;   // text formats supported.
};

//! Primary Contact Person Property structure
struct QgsWmsContactPersonPrimaryProperty
{
  QString            contactPerson;
  QString            contactOrganization;
};

//! Contact Address Property structure
struct QgsWmsContactAddressProperty
{
  QString            addressType;
  QString            address;
  QString            city;
  QString            stateOrProvince;
  QString            postCode;
  QString            country;
};

//! Contact Information Property structure
struct QgsWmsContactInformationProperty
{
  QgsWmsContactPersonPrimaryProperty contactPersonPrimary;
  QString                            contactPosition;
  QgsWmsContactAddressProperty       contactAddress;
  QString                            contactVoiceTelephone;
  QString                            contactFacsimileTelephone;
  QString                            contactElectronicMailAddress;
};

//! Service Property structure
// TODO: Fill to WMS specifications
struct QgsWmsServiceProperty
{
  QString                            title;
  QString                            abstract;
  QStringList                        keywordList;
  QgsWmsOnlineResourceAttribute      onlineResource;
  QgsWmsContactInformationProperty   contactInformation;
  QString                            fees;
  QString                            accessConstraints;
  uint                               layerLimit = 0;
  uint                               maxWidth = 0;
  uint                               maxHeight = 0;
};

//! Bounding Box Property structure
// TODO: Fill to WMS specifications
struct QgsWmsBoundingBoxProperty
{
  QString   crs;
  QgsRectangle   box;    // consumes minx, miny, maxx, maxy.
};

//! Dimension Property structure
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

//! Logo URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsLogoUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;

  int                             width;
  int                             height;
};

//! Attribution Property structure
// TODO: Fill to WMS specifications
struct QgsWmsAttributionProperty
{
  QString                         title;
  QgsWmsOnlineResourceAttribute   onlineResource;
  QgsWmsLogoUrlProperty           logoUrl;
};

//! Legend URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsLegendUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;

  int                             width;
  int                             height;
};

//! StyleSheet URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsStyleSheetUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;
};

//! Style URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsStyleUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;
};

//! Style Property structure
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

//! Authority URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsAuthorityUrlProperty
{
  QgsWmsOnlineResourceAttribute   onlineResource;
  QString                         name;             // XML "NMTOKEN" type
};

//! Identifier Property structure
// TODO: Fill to WMS specifications
struct QgsWmsIdentifierProperty
{
  QString   authority;
};

//! Metadata URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsMetadataUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;
  QString                         type;             // XML "NMTOKEN" type
};

//! Data List URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsDataListUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;
};

//! Feature List URL Property structure
// TODO: Fill to WMS specifications
struct QgsWmsFeatureListUrlProperty
{
  QString                         format;
  QgsWmsOnlineResourceAttribute   onlineResource;
};

//! Layer Property structure
// TODO: Fill to WMS specifications
struct QgsWmsLayerProperty
{
  // WMS layer properties
  int                                     orderId;
  QString                                 name;
  QString                                 title;
  QString                                 abstract;
  QStringList                             keywordList;
  QStringList                             crs;        // coord ref sys
  QgsRectangle                            ex_GeographicBoundingBox;
  QVector<QgsWmsBoundingBoxProperty>      boundingBoxes;
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

struct QgsWmtsTheme
{
  QString identifier;
  QString title, abstract;
  QStringList keywords;
  QgsWmtsTheme *subTheme = nullptr;
  QStringList layerRefs;

  QgsWmtsTheme() = default;
  ~QgsWmtsTheme() { delete subTheme; }
};

struct QgsWmtsTileMatrixLimits;

struct QgsWmtsTileMatrix
{
  QString identifier;
  QString title, abstract;
  QStringList keywords;
  double scaleDenom;
  QgsPointXY topLeft;  //!< Top-left corner of the tile matrix in map units
  int tileWidth;     //!< Width of a tile in pixels
  int tileHeight;    //!< Height of a tile in pixels
  int matrixWidth;   //!< Number of tiles horizontally
  int matrixHeight;  //!< Number of tiles vertically
  double tres;       //!< Pixel span in map units

  /**
   * Returns extent of a tile in map coordinates.
   * (same function as tileBBox() but returns QRectF instead of QgsRectangle)
   */
  QRectF tileRect( int col, int row ) const;

  /**
   * Returns extent of a tile in map coordinates
   * (same function as tileRect() but returns QgsRectangle instead of QRectF)
   */
  QgsRectangle tileBBox( int col, int row ) const;

  /**
   * Returns range of tiles that intersects with the view extent
   * (\a tml may be NULLPTR)
   */
  void viewExtentIntersection( const QgsRectangle &viewExtent, const QgsWmtsTileMatrixLimits *tml, int &col0, int &row0, int &col1, int &row1 ) const;

};

struct QgsWmtsTileMatrixSet
{
  QString identifier;   //!< Tile matrix set identifier
  QString title;        //!< Human readable tile matrix set name
  QString abstract;     //!< Brief description of the tile matrix set
  QStringList keywords; //!< List of words/phrases to describe the dataset
  QString crs;          //!< CRS of the tile matrix set
  QString wkScaleSet;   //!< Optional reference to a well-known scale set
  //! available tile matrixes (key = pixel span in map units)
  QMap<double, QgsWmtsTileMatrix> tileMatrices;

  //! Returns closest tile resolution to the requested one. (resolution = width [map units] / with [pixels])
  const QgsWmtsTileMatrix *findNearestResolution( double vres ) const;

  //! Returns the tile matrix for other near resolution from given tres (positive offset = lower resolution tiles)
  const QgsWmtsTileMatrix *findOtherResolution( double tres, int offset ) const;
};

enum QgsTileMode { WMTS, WMSC, XYZ };

struct QgsWmtsTileMatrixLimits
{
  QString tileMatrix;
  int minTileRow, maxTileRow;
  int minTileCol, maxTileCol;
};

struct QgsWmtsTileMatrixSetLink
{
  QString tileMatrixSet;
  QHash<QString, QgsWmtsTileMatrixLimits> limits;
};

struct QgsWmtsLegendURL
{
  QString format;
  double minScale, maxScale;
  QString href;
  int width, height;
};

struct QgsWmtsStyle
{
  QString identifier;
  QString title, abstract;
  QStringList keywords;
  bool isDefault;
  QList<QgsWmtsLegendURL> legendURLs;
};

/**
 * In case of multi-dimensional data, the service metadata can describe their multi-
 * dimensionality and tiles can be requested at specific values in these dimensions.
 * Examples of dimensions are Time, Elevation and Band.
 */
struct QgsWmtsDimension
{
  QString identifier;   //!< Name of the dimensional axis
  QString title;        //!< Human readable name
  QString abstract;     //!< Brief description of the dimension
  QStringList keywords; //!< List of words/phrases to describe the dataset
  QString UOM;          //!< Units of measure of dimensional axis
  QString unitSymbol;   //!< Symbol of the units
  QString defaultValue; //!< Default value to be used if value is not specified in request
  bool current;         //!< Indicates whether temporal data are normally kept current
  QStringList values;   //!< Available values for this dimension
};

struct QgsWmtsTileLayer
{
  enum QgsTileMode tileMode;
  QString identifier;
  QString title, abstract;
  QStringList keywords;
  QVector<QgsWmsBoundingBoxProperty> boundingBoxes;
  QStringList formats;
  QStringList infoFormats;
  QString defaultStyle;
  int dpi = -1;   //!< DPI of the tile layer (-1 for unknown DPI)
  //! available dimensions (optional, for multi-dimensional data)
  QHash<QString, QgsWmtsDimension> dimensions;
  QHash<QString, QgsWmtsStyle> styles;
  QHash<QString, QgsWmtsTileMatrixSetLink> setLinks;

  QHash<QString, QString> getTileURLs;
  QHash<QString, QString> getFeatureInfoURLs;
};

//! Capability Property structure
// TODO: Fill to WMS specifications
struct QgsWmsCapabilityProperty
{
  QgsWmsRequestProperty                request;
  QgsWmsExceptionProperty              exception;

  // Top level layer should normally be present max once
  // <element name="Capability">
  //    <element ref="wms:Layer" minOccurs="0"/>  - default maxOccurs=1
  // but there are a few non conformant capabilities around (#13762)
  QList<QgsWmsLayerProperty>           layers;

  QList<QgsWmtsTileLayer>              tileLayers;
  QHash<QString, QgsWmtsTileMatrixSet> tileMatrixSets;
};

//! Capabilities Property structure
// TODO: Fill to WMS specifications
struct QgsWmsCapabilitiesProperty
{
  QgsWmsServiceProperty         service;
  QgsWmsCapabilityProperty      capability;
  QString                       version;
};

//! Formats supported by QImageReader
struct QgsWmsSupportedFormat
{
  QString format;
  QString label;
};

enum QgsWmsTileAttribute
{
  TileReqNo = QNetworkRequest::User + 0,
  TileIndex = QNetworkRequest::User + 1,
  TileRect  = QNetworkRequest::User + 2,
  TileRetry = QNetworkRequest::User + 3,
};

enum QgsWmsDpiMode
{
  DpiNone = 0,
  DpiQGIS = 1,
  DpiUMN = 2,
  DpiGeoServer = 4,
  DpiAll = DpiQGIS | DpiUMN | DpiGeoServer,
};



struct QgsWmsParserSettings
{
  QgsWmsParserSettings( bool ignAxis = false, bool invAxis = false )
    : ignoreAxisOrientation( ignAxis )
    , invertAxisOrientation( invAxis )
  {}
  bool ignoreAxisOrientation;
  bool invertAxisOrientation;
};

struct QgsWmsAuthorization
{
  QgsWmsAuthorization( const QString &userName = QString(), const QString &password = QString(), const QString &referer = QString(), const QString &authcfg = QString() )
    : mUserName( userName )
    , mPassword( password )
    , mReferer( referer )
    , mAuthCfg( authcfg )
  {}

  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isEmpty() || !mPassword.isEmpty() )
    {
      request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toLatin1().toBase64() );
    }

    if ( !mReferer.isEmpty() )
    {
      request.setRawHeader( "Referer", QStringLiteral( "%1" ).arg( mReferer ).toLatin1() );
    }
    return true;
  }
  //! Sets authorization reply
  bool setAuthorizationReply( QNetworkReply *reply ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
    }
    return true;
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;

  //! Referer for http requests
  QString mReferer;

  //! Authentication configuration ID
  QString mAuthCfg;
};


//! URI that gets passed to provider
class QgsWmsSettings
{
  public:

    bool parseUri( const QString &uriString );

    QString baseUrl() const { return mBaseUrl; }
    QgsWmsAuthorization authorization() const { return mAuth; }

    QgsWmsParserSettings parserSettings() const { return mParserSettings; }

  protected:
    QgsWmsParserSettings    mParserSettings;

    //! layer is tiled, tile layer and active matrix set
    bool                    mTiled;
    //! whether we actually work with XYZ tiles instead of WMS / WMTS
    bool mXyz;
    //! chosen values for dimensions in case of multi-dimensional data (key=dim id, value=dim value)
    QHash<QString, QString>  mTileDimensionValues;
    //! name of the chosen tile matrix set
    QString                 mTileMatrixSetId;

    /**
     * Maximum width and height of getmap requests
     */
    int mMaxWidth;
    int mMaxHeight;

    /**
     * Step size when iterating the layer
     */
    int mStepWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
    int mStepHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;

    //! Data source URI of the WMS for this layer
    QString mHttpUri;

    //! URL part of URI (httpuri)
    QString mBaseUrl;

    QgsWmsAuthorization mAuth;

    bool mIgnoreGetMapUrl;
    bool mIgnoreGetFeatureInfoUrl;
    bool mIgnoreReportedLayerExtents = false;
    bool mSmoothPixmapTransform;
    enum QgsWmsDpiMode mDpiMode;

    /**
     * Active sublayers managed by this provider in a draw function, in order from bottom to top
     * (some may not be visible in a draw function, cf. activeSubLayerVisibility)
     */
    QStringList mActiveSubLayers;
    QStringList mActiveSubStyles;

    /**
     * Visibility status of the given active sublayer
     */
    QMap<QString, bool> mActiveSubLayerVisibility;

    //! FEATURE_COUNT for GetFeatureInfo
    int mFeatureCount;

    /**
     * MIME type of the image encoding used from the WMS server
     */
    QString mImageMimeType;

    QString mCrsId;

    bool mEnableContextualLegend;

    friend class QgsWmsProvider;
};


//! Keeps information about capabilities of particular URI
class QgsWmsCapabilities
{
  public:

    /**
     * Constructs a QgsWmsCapabilities object with the given \a coordinateTransformContext
     */
    QgsWmsCapabilities( const QgsCoordinateTransformContext &coordinateTransformContext = QgsCoordinateTransformContext() );

    bool isValid() const { return mValid; }

    bool parseResponse( const QByteArray &response, QgsWmsParserSettings settings );

    QString lastError() const { return mError; }
    QString lastErrorFormat() const { return mErrorFormat; }

    QgsWmsCapabilitiesProperty capabilitiesProperty() { return mCapabilities; }

    /**
     * \brief   Returns a list of the supported layers of the WMS server
     *
     * \returns The list of layers will be placed here.
     *
     * \todo Document this better
     */
    QVector<QgsWmsLayerProperty> supportedLayers() const { return mLayersSupported; }

    //! Gets raster image encodings supported by the WMS, expressed as MIME types
    QStringList supportedImageEncodings() const { return mCapabilities.capability.request.getMap.format; }

    /**
     * \brief   Returns a map for the hierarchy of layers
     */
    void layerParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const { parents = mLayerParents; parentNames = mLayerParentNames; }

    /**
     * \brief   Returns a list of the supported tile layers of the WMS server
     *
     * \returns The list of tile sets will be placed here.
     */
    QList<QgsWmtsTileLayer> supportedTileLayers() const { return mTileLayersSupported; }

    /**
     * \brief   Returns a list of the available tile matrix sets
     */
    QHash<QString, QgsWmtsTileMatrixSet> supportedTileMatrixSets() const { return mTileMatrixSets; }

    //! Find out whether to invert axis orientation when parsing/writing coordinates
    bool shouldInvertAxisOrientation( const QString &ogcCrs );

    //! Find out identify capabilities
    int identifyCapabilities() const;

  protected:
    bool parseCapabilitiesDom( QByteArray const &xml, QgsWmsCapabilitiesProperty &capabilitiesProperty );

    void parseService( QDomElement const &e, QgsWmsServiceProperty &serviceProperty );
    void parseOnlineResource( QDomElement const &e, QgsWmsOnlineResourceAttribute &onlineResourceAttribute );
    void parseKeywordList( QDomElement  const &e, QStringList &keywordListProperty );
    void parseContactInformation( QDomElement const &e, QgsWmsContactInformationProperty &contactInformationProperty );
    void parseContactPersonPrimary( QDomElement const &e, QgsWmsContactPersonPrimaryProperty &contactPersonPrimaryProperty );
    void parseContactAddress( QDomElement const &e, QgsWmsContactAddressProperty &contactAddressProperty );

    void parseCapability( QDomElement const &e, QgsWmsCapabilityProperty &capabilityProperty );
    void parseRequest( QDomElement const &e, QgsWmsRequestProperty &requestProperty );
    void parseLegendUrl( QDomElement const &e, QgsWmsLegendUrlProperty &legendUrlProperty );
    void parseLayer( QDomElement const &e, QgsWmsLayerProperty &layerProperty, QgsWmsLayerProperty *parentProperty = nullptr );
    void parseStyle( QDomElement const &e, QgsWmsStyleProperty &styleProperty );

    void parseOperationType( QDomElement const &e, QgsWmsOperationType &operationType );
    void parseDcpType( QDomElement const &e, QgsWmsDcpTypeProperty &dcpType );
    void parseHttp( QDomElement const &e, QgsWmsHttpProperty &httpProperty );
    void parseGet( QDomElement const &e, QgsWmsGetProperty &getProperty );
    void parsePost( QDomElement const &e, QgsWmsPostProperty &postProperty );

    void parseTileSetProfile( QDomElement const &e );
    void parseWMTSContents( QDomElement const &e );
    void parseKeywords( const QDomNode &e, QStringList &keywords );
    void parseTheme( const QDomElement &e, QgsWmtsTheme &t );

    QString nodeAttribute( const QDomElement &e, const QString &name, const QString &defValue = QString() );

    /**
     * In case no bounding box is present in WMTS capabilities, try to estimate it from tile matrix sets.
     * Returns true if the detection went fine.
     */
    bool detectTileLayerBoundingBox( QgsWmtsTileLayer &l );

  protected:
    bool mValid = false;

    QString mError;
    QString mErrorCaption;
    QString mErrorFormat;

    QgsWmsParserSettings mParserSettings;

    //! number of layers and parents
    int mLayerCount = -1;
    QMap<int, int> mLayerParents;
    QMap<int, QStringList> mLayerParentNames;

    /**
     * WMS "queryable" per layer
     * Used in determining if the Identify map tool can be useful on the rendered WMS map layer.
     */
    QMap<QString, bool> mQueryableForLayer;

    /**
     * layers hosted by the WMS
     */
    QVector<QgsWmsLayerProperty> mLayersSupported;

    /**
     * tilesets hosted by the WMTS
     */
    QList<QgsWmtsTileLayer> mTileLayersSupported;

    /**
     * themes hosted by the WMTS
     */
    QList<QgsWmtsTheme> mTileThemes;

    /**
     * Parsed capabilities of the WMS
     */
    QgsWmsCapabilitiesProperty mCapabilities;

    //! Formats supported by server and provider
    QMap<QgsRaster::IdentifyFormat, QString> mIdentifyFormats;


    /**
     * tile matrix sets hosted by the WMS
     */
    QHash<QString, QgsWmtsTileMatrixSet> mTileMatrixSets;

    //temporarily caches invert axis setting for each crs
    QHash<QString, bool> mCrsInvertAxis;

  private:

    QgsCoordinateTransformContext mCoordinateTransformContext;

    friend class QgsWmsProvider;
};



/**
 * Class that handles download of capabilities.
 */
class QgsWmsCapabilitiesDownload : public QObject
{
    Q_OBJECT

  public:
    explicit QgsWmsCapabilitiesDownload( bool forceRefresh, QObject *parent = nullptr );

    QgsWmsCapabilitiesDownload( const QString &baseUrl, const QgsWmsAuthorization &auth, bool forceRefresh, QObject *parent = nullptr );

    ~QgsWmsCapabilitiesDownload() override;

    bool downloadCapabilities();

    bool downloadCapabilities( const QString &baseUrl, const QgsWmsAuthorization &auth );

    QString lastError() const { return mError; }

    QByteArray response() const { return mHttpCapabilitiesResponse; }

    //! Abort network request immediately
    void abort();

  signals:
    //! \brief emit a signal to be caught by qgisapp and display a msg on status bar
    void statusChanged( QString const   &statusQString );

    //! \brief emit a signal once the download is finished
    void downloadFinished();

  protected slots:
    void capabilitiesReplyFinished();
    void capabilitiesReplyProgress( qint64, qint64 );

  protected:
    //! URL part of URI (httpuri)
    QString mBaseUrl;

    QgsWmsAuthorization mAuth;

    //! The reply to the capabilities request
    QNetworkReply *mCapabilitiesReply = nullptr;

    //! The error message associated with the last WMS error.
    QString mError;

    //! The mime type of the message
    QString mErrorFormat;

    //! Capabilities of the WMS (raw)
    QByteArray mHttpCapabilitiesResponse;

    bool mIsAborted;
    bool mForceRefresh;
};



#endif // QGSWMSCAPABILITIES_H
