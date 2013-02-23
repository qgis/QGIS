/***************************************************************************
  qgsosmdownload.h
  --------------------------------------
  Date                 : January 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSMDOWNLOAD_H
#define OSMDOWNLOAD_H


#include <QObject>
#include <QFile>
#include <QNetworkReply>

class QgsRectangle;

/**
 * @brief OSMDownload is a utility class for downloading OpenStreetMap via Overpass API.
 *
 * To use this class, it is necessary to set query, output file name and start the request.
 * The interface is asynchronous, the caller has to wait for finished() signal that is
 * emitted whe the request has finished (successfully or with an error).
 *
 * To check whether the the request has been successful, check hasError() and use errorString()
 * to retreive error message. An error may happen either directly in start() method
 * or during the network communication.
 *
 * By default OSMDownload uses remote service at location returned by defaultServiceUrl() method.
 */
class ANALYSIS_EXPORT QgsOSMDownload : public QObject
{
    Q_OBJECT
  public:

    //! Return URL of the service that is used by default
    static QString defaultServiceUrl();

    //! Create query (in Overpass Query Language) that fetches everything in given rectangle
    static QString queryFromRect( const QgsRectangle& rect );

    QgsOSMDownload();
    ~QgsOSMDownload();

    void setServiceUrl( const QString& serviceUrl ) { mServiceUrl = serviceUrl; }
    QString serviceUrl() const { return mServiceUrl; }

    void setQuery( const QString& query ) { mQuery = query; }
    QString query() const { return mQuery; }

    void setOutputFileName( const QString& outputFileName ) { mFile.setFileName( outputFileName ); }
    QString outputFileName() const { return mFile.fileName(); }

    bool hasError() const { return !mError.isNull(); }
    QString errorString() const { return mError; }

    /**
     * @brief Starts network request for data. The prerequisite is that the query string and output
     * file name have been set.
     *
     * Only one request may be pending at one point - if you need more requests at once, use several instances.
     *
     * @return true if the network request has been issued, false otherwise (and sets error string)
     */
    bool start();

    /**
     * @brief Aborts current pending request
     * @return true if there is a pending request and has been aborted, false otherwise
     */
    bool abort();

    //! Returns true if the request has already finished
    bool isFinished() const;

  signals:
    void finished(); //!< emitted when the network reply has finished (with success or with an error)
    void downloadProgress( qint64, qint64 ); //!< normally the total length is not known (until we reach end)

  private slots:
    void onReadyRead();
    void onFinished();
    void onError( QNetworkReply::NetworkError err );

  private:
    QString mServiceUrl;
    QString mQuery;
    QString mError;

    QNetworkReply* mReply;
    QFile mFile;
};

#endif // OSMDOWNLOAD_H
