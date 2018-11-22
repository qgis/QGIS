/***************************************************************************
  qgsfiledownloader.h
  --------------------------------------
  Date                 : November 2016
  Copyright            : (C) 2016 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEDOWNLOADER_H
#define QGSFILEDOWNLOADER_H

#include <QObject>
#include <QFile>
#include <QNetworkReply>
#include "qgis_core.h"

#ifndef QT_NO_SSL
#include <QSslError>
#endif

/**
 * \ingroup core
 * QgsFileDownloader is a utility class for downloading files.
 *
 * To use this class, it is necessary to pass the URL and an output file name as
 * arguments to the constructor, the download will start immediately.
 *
 * The download is asynchronous.
 *
 * The object will destroy itself when the request completes, errors or is canceled.
 * An optional authentication configuration can be specified.
 *
 * \note This class was part of the GUI library from QGIS 2.18.1 until QGIS 3.0
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFileDownloader : public QObject
{
    Q_OBJECT
  public:

    /**
     * QgsFileDownloader
     * \param url the download url
     * \param outputFileName file name where the downloaded content will be stored
     * \param authcfg optionally apply this authentication configuration
     * \param delayStart if true, the download will not be commenced immediately and must
     * be triggered by a later call to startDownload(). This can be useful if connections need
     * to be made to the downloader and there's a chance the download will emit
     * signals before these connections have been made.
     */
    QgsFileDownloader( const QUrl &url, const QString &outputFileName, const QString &authcfg = QString(), bool delayStart = false );

  signals:
    //! Emitted when the download has completed successfully
    void downloadCompleted();
    //! Emitted always when the downloader exits
    void downloadExited();

    /**
     * Emitted when the download was canceled by the user.
     * \see cancelDownload()
     */
    void downloadCanceled();

    //! Emitted when an error makes the download fail
    void downloadError( QStringList errorMessages );
    //! Emitted when data are ready to be processed
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

  public slots:

    /**
     * Call to abort the download and delete this object after the cancelation
     * has been processed.
     * \see downloadCanceled()
     */
    void cancelDownload();

    //! Called to start the download
    void startDownload();

  private slots:
    //! Called when the network reply data are ready
    void onReadyRead();
    //! Called when the network reply has finished
    void onFinished();
    //! Called on data ready to be processed
    void onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
    //! Called when a network request times out
    void onRequestTimedOut();

#ifndef QT_NO_SSL

    /**
     * Called on SSL network Errors
     * \param reply
     * \param errors
     */
    void onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

  protected:
    ~QgsFileDownloader() override;

  private:

    /**
     * Abort current request and show an error if the instance has GUI
     * notifications enabled.
     */
    void error( const QStringList &errorMessages );
    void error( const QString &errorMessage );
    QUrl mUrl;
    QNetworkReply *mReply = nullptr;
    QFile mFile;
    bool mDownloadCanceled;
    QStringList mErrors;
    QString mAuthCfg;
};

#endif // QGSFILEDOWNLOADER_H
