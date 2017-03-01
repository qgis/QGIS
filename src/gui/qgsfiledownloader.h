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
#include <QProgressDialog>
#ifndef QT_NO_OPENSSL
#include <QSslError>
#endif

/** \ingroup gui
 * QgsFileDownloader is a utility class for downloading files.
 *
 * To use this class, it is necessary to pass the URL and an output file name as
 * arguments to the constructor, the download will start immediately.
 * The download is asynchronous and depending on the guiNotificationsEnabled
 * parameter accepted by the constructor (default = true) the class will
 * show a progress dialog and report all errors in a QMessageBox::warning dialog.
 * If the guiNotificationsEnabled parameter is set to false, the class can still
 * be used through the signals and slots mechanism.
 * The object will destroy itself when the request completes, errors or is canceled.
 *
 * @note added in QGIS 2.18.1
 */
class GUI_EXPORT QgsFileDownloader : public QObject
{
    Q_OBJECT
  public:
    /**
     * QgsFileDownloader
     * @param url the download url
     * @param outputFileName file name where the downloaded content will be stored
     * @param guiNotificationsEnabled if false, the downloader will not display any progress bar or error message
     */
    QgsFileDownloader( QUrl url, QString outputFileName, bool guiNotificationsEnabled = true );

  signals:
    /** Emitted when the download has completed successfully */
    void downloadCompleted();
    /** Emitted always when the downloader exits  */
    void downloadExited();
    /** Emitted when the download was canceled by the user */
    void downloadCanceled();
    /** Emitted when an error makes the download fail  */
    void downloadError( QStringList errorMessages );
    /** Emitted when data ready to be processed */
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

  public slots:
    /**
     * Called when a download is canceled by the user
     * this slot aborts the download and deletes
     * the object
     */
    void onDownloadCanceled();

  private slots:
    /** Called when the network reply data are ready */
    void onReadyRead();
    /** Called when the network reply has finished */
    void onFinished();
    /** Called on data ready to be processed */
    void onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
    /** Called when a network request times out  */
    void onRequestTimedOut();
    /** Called to start the download */
    void startDownload();
#ifndef QT_NO_OPENSSL
    /**
     * Called on SSL network Errors
     * @param reply
     * @param errors
     */
    void onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

  protected:
    ~QgsFileDownloader();

  private:
    /**
     * Abort current request and show an error if the instance has GUI
     * notifications enabled.
     */
    void error( QStringList errorMessages );
    void error( QString errorMessage );
    QUrl mUrl;
    QNetworkReply* mReply;
    QFile mFile;
    QProgressDialog* mProgressDialog;
    bool mDownloadCanceled;
    QStringList mErrors;
    bool mGuiNotificationsEnabled;
};

#endif // QGSFILEDOWNLOADER_H
