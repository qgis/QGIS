/***************************************************************************
  qgsfileuploader.h
  --------------------------------------
  Date                 : August 2025
  Copyright            : (C) 2025 by Valentin Buira
  Email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEUPLOADER_H
#define QGSFILEUPLOADER_H

#include <QObject>
#include <QFile>
#include <QNetworkReply>
#include <QUrl>

#include "qgis.h"
#include "qgis_core.h"

#ifndef QT_NO_SSL
#include <QSslError>
#endif

/**
 * \ingroup core
 * \brief A utility class for uploading files.
 *
 * To use this class, it is necessary to pass the URL and a the file name of the file to upload as
 * arguments to the constructor, the upload will start immediately.
 *
 * The upload is asynchronous.
 *
 * The object will destroy itself when the request completes, errors or is canceled.
 * An optional authentication configuration can be specified.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsFileUploader : public QObject
{
    Q_OBJECT
  public:

    /**
     * QgsFileUploader
     * \param uploadFileName file name of the file to upload to the server
     * \param url the upload URL
     * \param formName the upload URL
     * \param authcfg optionally apply this authentication configuration
     * \param delayStart if TRUE, the download will not be commenced immediately and must
     * be triggered by a later call to startUpload(). This can be useful to setup connections
     */
    QgsFileUploader( const QString &uploadFileName, const QUrl &url, const QString &formName = QString(), const QString &authcfg = QString(), bool delayStart = true );

  signals:
    //! Emitted when the upload has completed successfully
    void uploadCompleted( const QUrl &url );
    //! Emitted always when the uploader exits
    void uploadExited();

    /**
     * Emitted when the upload was canceled by the user.
     * \see cancelUpload()
     */
    void uploadCanceled();

    //! Emitted when an error makes the upload fail
    void uploadError( QStringList errorMessages );
    //! Emitted when data are ready to be processed
    void uploadProgress( qint64 bytesSent, qint64 bytesTotal );

  public slots:

    /**
     * Call to abort the upload and delete this object after the cancellation
     * has been processed.
     * \see uploadCanceled()
     */
    void cancelUpload();

    //! Called to start the upload
    void startUpload();

  private slots:
    //! Called when the network reply has finished
    void onFinished();
    //! Called on data ready to be processed
    void onUploadProgress( qint64 bytesSent, qint64 bytesTotal );
    //! Called when a network request times out
    void onRequestTimedOut( QNetworkReply *reply );

#ifndef QT_NO_SSL

    /**
     * Called on SSL network Errors
     * \param reply
     * \param errors
     */
    void onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

  protected:
    ~QgsFileUploader() override;

  private:

    /**
     * Abort current request and show an error if the instance has GUI
     * notifications enabled.
     */
    void error( const QStringList &errorMessages );
    void error( const QString &errorMessage );
    QUrl mUrl;
    QString mFormName;
    QNetworkReply *mReply = nullptr;
    QFile mFile;
    bool mUploadCanceled;
    QStringList mErrors;
    QString mAuthCfg;
};

#endif // QGSFILEUPLOADER_H
