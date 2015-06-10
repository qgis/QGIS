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

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QNetworkRequest>
#include <QVector>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

/** CoverageSummary structure */
struct QgsWcsCoverageSummary
{
  QgsWcsCoverageSummary() : orderId( 0 ), valid( false ), described( false ), width( 0 ), height( 0 ), hasSize( false ) { }

  int           orderId;
  QString       identifier;
  QString       title;
  QString       abstract;
  QStringList   supportedCrs;
  QStringList   supportedFormat;
  QList<double> nullValues;
  QgsRectangle  wgs84BoundingBox; // almost useless, we need the native
  QString       nativeCrs;
  // Map of bounding boxes, key is CRS name (srsName), e.g. EPSG:4326
  QMap<QString, QgsRectangle> boundingBoxes;
  QgsRectangle  nativeBoundingBox;
  // timePosition or timePeriod (beginPosition/endPosition[/timeResolution] - used in KVP request)
  QStringList times;
  QVector<QgsWcsCoverageSummary> coverageSummary;
  // non reflecting Capabilities structure:
  bool valid;
  bool described;
  // native size
  int width;
  int height;
  bool hasSize;
};

/** Capability Property structure */
struct QgsWcsCapabilitiesProperty
{
  QString                   version;
  QString                   title;
  QString                   abstract;
  QString                   getCoverageGetUrl;
  // using QgsWcsCoverageSummary for contents for simplification
  QgsWcsCoverageSummary     contents;
};

/**
  \brief WCS Capabilities.
*/
class QgsWcsCapabilities : public QObject
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
    QgsWcsCapabilities( QgsDataSourceURI const & theUri );
    QgsWcsCapabilities();

    //! Destructor
    ~QgsWcsCapabilities();

    void setUri( QgsDataSourceURI const &theUri );

    QgsWcsCapabilitiesProperty capabilities();

    /**
     * \brief   Returns a list of the supported layers of the WCS server
     *
     * \param[out] layers   The list of layers will be placed here.
     *
     * \retval false if the layers could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool supportedCoverages( QVector<QgsWcsCoverageSummary> &coverageSummary );

    /**
     * \brief   Returns a map for the hierarchy of layers
     */
    void coverageParents( QMap<int, int> &parents, QMap<int, QStringList> &parentNames ) const;

    //! Get coverage summary for identifier
    QgsWcsCoverageSummary coverage( QString const & theIdentifier );

    //! Get list of all coverage summaries
    QList<QgsWcsCoverageSummary> coverages();

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    static QString prepareUri( QString uri );

    /** \brief Returns the GetCoverage full url
     *  \param version optional version, e.g. 1.0.0 or 1.1.0 */
    QString getCapabilitiesUrl( const QString version ) const;

    /** \brief Returns the GetCoverage full url using current version  */
    QString getCapabilitiesUrl() const;

    /** \brief Returns the GetCoverage full full url using current version  */
    QString getDescribeCoverageUrl( QString const &identifier ) const;

    /** Returns the GetCoverage base url */
    QString getCoverageUrl() const;

    //! Send request to server
    bool sendRequest( QString const & url );

    /** Get additional coverage info from server. Version 1.0 GetCapabilities
     *  response does not contain all info (CRS, formats).
     */
    bool describeCoverage( QString const &identifier, bool forceRefresh = false );

    bool convertToDom( QByteArray const &xml );
    bool parseDescribeCoverageDom10( QByteArray const &xml, QgsWcsCoverageSummary *coverage );
    bool parseDescribeCoverageDom11( QByteArray const &xml, QgsWcsCoverageSummary *coverage );

    //! set authorization header
    void setAuthorization( QNetworkRequest &request ) const;

    QString version() const { return mCapabilities.version; }

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

    /**
     * \brief   Returns the format of the error message (text or html)
     */
    QString lastErrorFormat();

    //! Get tag name without namespace
    static QString stripNS( const QString &name );

    //! Get text of first child of specified name, NS is ignored
    static QString firstChildText( const QDomElement &element, const QString &name );

    //! Get first child of specified name, NS is ignored
    static QDomElement firstChild( const QDomElement &element, const QString &name );

    /** Find sub elements by path which is string of dot separated tag names.
     *  NS is ignored. Example path: domainSet.spatialDomain.RectifiedGrid */
    static QList<QDomElement> domElements( const QDomElement &element, const QString &path );

    /** Find first sub element by path which is string of dot separated tag names.
     *  NS is ignored. Example path: domainSet.spatialDomain.RectifiedGrid */
    static QDomElement domElement( const QDomElement &element, const QString &path );

    /** Get text of element specified by path */
    static QString domElementText( const QDomElement &element, const QString &path );

    /** Get sub elements texts by path */
    static QStringList domElementsTexts( const QDomElement &element, const QString &path );

  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

  private slots:
    void capabilitiesReplyFinished();
    void capabilitiesReplyProgress( qint64, qint64 );

  private:
    void parseUri();

    //! Get coverage summary for identifier
    QgsWcsCoverageSummary * coverageSummary( QString const & theIdentifier, QgsWcsCoverageSummary* parent = 0 );

    // ! Get list of all sub coverages
    QList<QgsWcsCoverageSummary> coverageSummaries( QgsWcsCoverageSummary* parent = 0 );

    void initCoverageSummary( QgsWcsCoverageSummary &coverageSummary );

    void clear();

    void showMessageBox( const QString &title, const QString &text );

    QList<int> parseInts( const QString &text );
    QList<double> parseDoubles( const QString &text );
    QString crsUrnToAuthId( const QString &text );
    /**
     * \brief Retrieve and parse the (cached) Capabilities document from the server
     *
     * \param preferredVersion - optional version, e.g. 1.0.0, 1.1.0
     *
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( QString preferredVersion );

    /** Retrieve the best WCS version supported by server and QGIS */
    bool retrieveServerCapabilities();

    //! \return false if the capabilities document could not be parsed - see lastError() for more info
    bool parseCapabilitiesDom( QByteArray const &xml, QgsWcsCapabilitiesProperty &capabilities );

    // ------------- 1.0 --------------------
    //! parse the WCS Layer XML element
    void parseContentMetadata( QDomElement const &e, QgsWcsCoverageSummary &coverageSummary );

    //! parse the WCS Layer XML element
    void parseCoverageOfferingBrief( QDomElement const &e, QgsWcsCoverageSummary &coverageSummary,
                                     QgsWcsCoverageSummary *parent = 0 );

    // ------------- 1.1 --------------------
    //! parse the WCS Layer XML element
    void parseCoverageSummary( QDomElement const &e, QgsWcsCoverageSummary &coverageSummary,
                               QgsWcsCoverageSummary *parent = 0 );

    //! Data source uri
    QgsDataSourceURI mUri;

    //! Response capabilities version
    QString mVersion;

    /**
     * Capabilities of the WCS Server (raw)
     */
    QByteArray mCapabilitiesResponse;

    /**
     * Capabilities of the WCS Server
     */
    QDomDocument mCapabilitiesDom;

    /**
     * Last Service Exception Report from the WCS Server
     */
    QDomDocument mServiceExceptionReportDom;

    /**
     * Parsed capabilities of the WCS Server
     */
    QgsWcsCapabilitiesProperty mCapabilities;

    /**
     * layers hosted by the WCS Server
     * This vector contain initial copies which are not updated by coverageSummary()!!!
     */
    QVector<QgsWcsCoverageSummary> mCoveragesSupported;

    /**
     * The reply to the capabilities request
     */
    QNetworkReply *mCapabilitiesReply;

    /**
     * The error caption associated with the last WCS error.
     */
    QString mErrorTitle;

    /**
     * The error message associated with the last WCS error.
     */
    QString mError;

    /** The mime type of the message
     */
    QString mErrorFormat;

    int mCoverageCount;

    //! number of layers and parents
    QMap<int, int> mCoverageParents;
    QMap<int, QStringList> mCoverageParentIdentifiers;

    //! Username for basic http authentication
    QString mUserName;

    //! Password for basic http authentication
    QString mPassword;

    //! Cache load control
    QNetworkRequest::CacheLoadControl mCacheLoadControl;
};


#endif

// ENDS
