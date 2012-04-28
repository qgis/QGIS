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

//#include "qgsrasterdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsrectangle.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>
#include <QUrl>

class QgsCoordinateTransform;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

/*
 * The following structs reflect the WCS XML schema,
 * as illustrated in ... the Web Coverage Service standard, version x.x xxxx-xx-xx.
 */

/** Get Property structure */
struct QgsWcsGet
{
  QString xlinkHref;
};

/** HTTP Property structure */
struct QgsWcsHTTP
{
  QgsWcsGet get;
};

/** DCP Type Property structure */
struct QgsWcsDCP
{
  QgsWcsHTTP http;
};

/** Version parameter */
struct QgsWcsVersion
{
  QStringList allowedValues;
};

/** Operation type structure */
struct QgsWcsOperation
{
  QgsWcsVersion version;
  QgsWcsDCP     dcp;
};

/** OperationsMetadata */
struct QgsWcsOperationsMetadata
{
  QgsWcsOperation  getCoverage;
};

/** ServiceerviceIdentification structure */
struct QgsWcsServiceIdentification
{
  QString                            title;
  QString                            abstract;
};

/** CoverageSummary structure */
struct QgsWcsCoverageSummary
{
  int           orderId;
  QString       identifier;
  QString       title;
  QString       abstract;
  QStringList   supportedCrs;
  QStringList   supportedFormat;
  QgsRectangle  wgs84BoundingBox;
  QVector<QgsWcsCoverageSummary> coverageSummary;
};

/** Contents structure */
/*
struct QgsWcsContents
{
  QStringList   supportedCrs;
  QStringList   supportedFormat;
  QVector<QgsWcsCoverageSummary> coverageSummary;
};
*/

/** Capability Property structure */
struct QgsWcsCapabilitiesProperty
{
  QString                       version;
  QgsWcsServiceIdentification   serviceIdentification;
  QgsWcsOperationsMetadata      operationsMetadata;
//  QgsWcsContents                contents;
  // using QgsWcsCoverageSummary for contents for simplification
  QgsWcsCoverageSummary         contents;
};

/**

  \brief Data provider for OGC WCS layers.

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
    //QgsWcsCapabilities( QString const & theUri = 0 );

    QgsWcsCapabilities( QgsDataSourceURI const & theUri );
    QgsWcsCapabilities( );

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

    //! Get coverage summare for identifier
    QgsWcsCoverageSummary coverageSummary( QString const & theIdentifier );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    static QString prepareUri( QString uri );

    /**Returns the GetCoverage url
     * @added in 1.5
     */
    QString getCoverageUrl() const;

    //! set authorization header
    void setAuthorization( QNetworkRequest &request ) const;

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

  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

  private slots:
    void capabilitiesReplyFinished();
    void capabilitiesReplyProgress( qint64, qint64 );

  private:
    void showMessageBox( const QString& title, const QString& text );

    //! Get tag name without namespace
    QString stripNS( const QString & name );

    /**
     * \brief Retrieve and parse the (cached) Capabilities document from the server
     *
     * \param forceRefresh  if true, ignores any previous response cached in memory
     *                      and always contact the server for a new copy.
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( bool forceRefresh = false );

    //! \return false if the capabilities document could not be parsed - see lastError() for more info
    bool parseCapabilitiesDom( QByteArray const &xml, QgsWcsCapabilitiesProperty &capabilities );

    //! parse the WCS Service XML element
    void parseServiceIdentification( QDomElement const &e, QgsWcsServiceIdentification &serviceIdentification );

    //! parse the WCS Capability XML element
    void parseOperationsMetadata( QDomElement const &e, QgsWcsOperationsMetadata &operationsMetadata );

    //! parse the WCS GetCoverage
    void parseOperation( QDomElement const & e, QgsWcsOperation& operation );

    //! parse the WCS HTTP XML element
    void parseHttp( QDomElement const &e, QgsWcsHTTP &http );

    //! parse the WCS DCPType XML element
    void parseDcp( QDomElement const &e, QgsWcsDCP &dcp );

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
};


#endif

// ENDS
