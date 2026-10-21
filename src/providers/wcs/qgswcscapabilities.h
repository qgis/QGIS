/***************************************************************************
      qgswcscapabilities.h  -  WCS capabilities
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au

    wcs                  : 4/2012 Radim Blazek, based on qgswmsprovider.h

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWCSCAPABILITIES_H
#define QGSWCSCAPABILITIES_H

#include "qgsdatasourceuri.h"
#include "qgsrectangle.h"

#include <QDomElement>
#include <QMap>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * Encapsulates metadata link properties.
 */
struct QgsWcsMetadataLinkProperty
{
    //! Metadata type, the standard to which the metadata complies
    QString metadataType;

    //! Optional metadata link URL
    QString xlinkHref;
};

/**
 * Encapsulates coverage details from CoverageSummary.
 */
struct QgsWcsCoverageSummary
{
    QgsWcsCoverageSummary() = default;

    int orderId = 0;

    //! Coverage unique identifier
    QString identifier;

    //! Coverage title
    QString title;

    //! Coverage description
    QString abstract;

    //! Coverage CRS which GetCoverage response may be expressed
    QStringList supportedCrs;

    //! Format identifiers, which GetCoverage response may be encoded
    QStringList supportedFormat;
    QList<double> nullValues;

    //! Minimum bounding rectangle surrounding this coverage
    QgsRectangle wgs84BoundingBox; // almost useless, we need the native
    QString nativeCrs;

    //! Optional metadataLink
    QgsWcsMetadataLinkProperty metadataLink;

    //! Map of bounding boxes, key is CRS name (srsName), e.g. EPSG:4326
    QMap<QString, QgsRectangle> boundingBoxes;
    QgsRectangle nativeBoundingBox;

    //! timePosition or timePeriod (beginPosition/endPosition[/timeResolution] - used in KVP request)
    QStringList times;
    QVector<QgsWcsCoverageSummary> coverageSummary;
    // non reflecting Capabilities structure:
    bool valid = false;
    bool described = false;
    // native size
    int width = 0;
    int height = 0;
    bool hasSize = false;
};

//! Encapsulates capability properties
struct QgsWcsCapabilitiesProperty
{
    QString version;
    QString title;
    QString abstract;
    QString getCoverageGetUrl;
    // using QgsWcsCoverageSummary for contents for simplification
    QgsWcsCoverageSummary contents;
};

/**
 * Encapsulates WCS capabilities.
*/
class QgsWcsCapabilities : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for the QgsWcsCapabilities.
     *
     * \param uri HTTP URL of the WCS server. If needed a proxy will be used
     * otherwise we contact the host directly.
     */
    explicit QgsWcsCapabilities( QgsDataSourceUri const &uri );
    //! copy constructor
    explicit QgsWcsCapabilities( const QgsWcsCapabilities &other );
    QgsWcsCapabilities() = default;

    void setUri( QgsDataSourceUri const &uri );

    const QgsWcsCapabilitiesProperty &capabilities() const;

    /**
     * Returns a list of the supported layers of the WCS server
     *
     * \param[out] layers The list of coverages will be placed here.
     * \returns FALSE if the layers could not be retrieved or parsed.
     */
    bool supportedCoverages( QVector<QgsWcsCoverageSummary> &coverageSummary );

    /**
     * Returns a map for the hierarchy of layers
     */
    void coverageParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const;

    //! Gets coverage summary for identifier
    QgsWcsCoverageSummary coverage( QString const &identifier );

    //! Gets list of all coverage summaries
    QList<QgsWcsCoverageSummary> coverages() const;

    /**
     * Returns the request URL with query parameters from the base URI preserved.
     */
    QUrl requestUrl( const QString &serverUrl ) const;

    /**
     * Returns the GetCoverage full URL.
     * \param version optional version, e.g. 1.0.0 or 1.1.0
    */
    QString getCapabilitiesUrl( const QString &version ) const;

    //! Returns the GetCoverage full URL using current version.
    QString getCapabilitiesUrl() const;

    /**
     * Returns the DescribeCoverage full URL for given coverage \a identifier.
     * \param identifier coverage identifier.
     */
    QString getDescribeCoverageUrl( QString const &identifier ) const;

    //! Returns the GetCoverage base URL
    QString getCoverageUrl() const;

    //! Sends request to server
    bool sendRequest( QString const &url );

    /**
     * Gets additional coverage info from server.
     *
     * Version 1.0 GetCapabilities response does not contain all info (CRS, formats).
     */
    bool describeCoverage( QString const &identifier, bool forceRefresh = false );

    bool convertToDom( QByteArray const &xml );
    bool parseDescribeCoverageDom10( QByteArray const &xml, QgsWcsCoverageSummary *coverage );
    bool parseDescribeCoverageDom11( QByteArray const &xml, QgsWcsCoverageSummary *coverage );

    //! Sets authorization header
    bool setAuthorization( QNetworkRequest &request ) const;

    //! Sets authorization reply
    bool setAuthorizationReply( QNetworkReply *reply ) const;

    QString version() const { return mCapabilities.version; }

    //! Returns the caption error text for the last error in this provider.
    QString lastErrorTitle();

    //! Returns the verbose error text for the last error in this provider.
    QString lastError();

    //! Returns the format of the error message (text or html).
    QString lastErrorFormat();

    //! Gets tag name without namespace
    static QString stripNS( const QString &name );

    //! Gets text of first child of specified name, NS is ignored
    static QString firstChildText( const QDomElement &element, const QString &name );

    //! Gets first child of specified name, NS is ignored
    static QDomElement firstChild( const QDomElement &element, const QString &name );

    /**
     * Finds sub elements by path which is string of dot separated tag names.
     * NS is ignored. Example path: domainSet.spatialDomain.RectifiedGrid
     */
    static QList<QDomElement> domElements( const QDomElement &element, const QString &path );

    /**
     * Finds first sub element by path which is string of dot separated tag names.
     * NS is ignored. Example path: domainSet.spatialDomain.RectifiedGrid
     */
    static QDomElement domElement( const QDomElement &element, const QString &path );

    //! Gets text of element specified by path
    static QString domElementText( const QDomElement &element, const QString &path );

    //! Gets sub elements texts by path
    static QStringList domElementsTexts( const QDomElement &element, const QString &path );

    //! Gets given element link tag value
    static QString elementLink( const QDomElement &element );

  signals:
    //! Emitted when progress changes.
    void progressChanged( int progress, int totalSteps );

    //! Emitted when status changes.
    void statusChanged( QString const &statusQString );

    void downloadFinished();

  private slots:
    void capabilitiesReplyFinished();
    void capabilitiesReplyProgress( qint64, qint64 );

  private:
    void parseUri();

    //! Gets coverage summary for identifier
    QgsWcsCoverageSummary *coverageSummary( QString const &identifier, QgsWcsCoverageSummary *parent = nullptr );

    //! Gets list of all sub coverages
    QList<QgsWcsCoverageSummary> coverageSummaries( const QgsWcsCoverageSummary *parent = nullptr ) const;

    void initCoverageSummary( QgsWcsCoverageSummary &coverageSummary );

    void clear();

    void showMessageBox( const QString &title, const QString &text );

    QList<int> parseInts( const QString &text );
    QList<double> parseDoubles( const QString &text );
    QString crsUrnToAuthId( const QString &text );

    /**
     * Retrieves and parses the (cached) Capabilities document from the server.
     *
     * \param preferredVersion optional version, e.g. 1.0.0, 1.1.0
     * \returns FALSE if the capabilities document could not be retrieved or parsed.
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( const QString &preferredVersion );

    //! Retrieves the best WCS version supported by server and QGIS
    bool retrieveServerCapabilities();

    //! Parses the capabilities document.
    bool parseCapabilitiesDom( const QByteArray &xml, QgsWcsCapabilitiesProperty &capabilities );

    // ------------- 1.0 --------------------
    //! Parses the WCS Layer XML element
    void parseContentMetadata( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary );

    //! Parses the WCS Layer XML element
    void parseCoverageOfferingBrief( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent = nullptr );

    //! Parses metadata element from the document
    void parseMetadataLink( const QDomElement &element, QgsWcsMetadataLinkProperty &metadataLink );

    // ------------- 1.1 --------------------
    //! Parses the WCS Layer XML element
    void parseCoverageSummary( const QDomElement &element, QgsWcsCoverageSummary &coverageSummary, QgsWcsCoverageSummary *parent = nullptr );

    //! Data source URI
    QgsDataSourceUri mUri;

    //! Response capabilities version
    QString mVersion;

    //! Capabilities of the WCS server (raw)
    QByteArray mCapabilitiesResponse;

    //! Capabilities of the WCS server
    QDomDocument mCapabilitiesDom;

    //! Last service exception report from the WCS server
    QDomDocument mServiceExceptionReportDom;

    //! Parsed capabilities of the WCS server
    QgsWcsCapabilitiesProperty mCapabilities;

    /**
     * Layers hosted by the WCS server.
     *
     * This vector contain initial copies which are not updated by coverageSummary()!!!
     */
    QVector<QgsWcsCoverageSummary> mCoveragesSupported;

    //! The reply to the capabilities request
    QNetworkReply *mCapabilitiesReply = nullptr;

    //! The error caption associated with the last WCS error.
    QString mErrorTitle;

    //! The error message associated with the last WCS error.
    QString mError;

    //! The mime type of the message
    QString mErrorFormat;

    int mCoverageCount = 0;

    //! Number of layers and parents
    QMap<int, int> mCoverageParents;
    QMap<int, QStringList> mCoverageParentIdentifiers;

    //! Username for basic http authentication
    QString mUserName;

    //! Password for basic http authentication
    QString mPassword;

    //! Cache load control
    QNetworkRequest::CacheLoadControl mCacheLoadControl = QNetworkRequest::PreferNetwork;

    QgsWcsCapabilities &operator=( const QgsWcsCapabilities & ) = delete;
};

#endif // QGSWCSCAPABILITIES_H
