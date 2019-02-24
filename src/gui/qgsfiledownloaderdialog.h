/***************************************************************************
  qgsfiledownloaderdialog.h
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

#ifndef QGSFILEDOWNLOADERDIALOG_H
#define QGSFILEDOWNLOADERDIALOG_H

#include <QProgressDialog>
#include "qgis_gui.h"

class QgsFileDownloader;

/**
 * \ingroup gui
 * QgsFileDownloaderDialog is a QProgressDialog subclass which
 * handles file downloads and user feedback.
 *
 * Internally, it uses QgsFileDownloader to handle the download,
 * while showing progress via a progress dialog and supporting
 * cancellation.
 *
 * \note Until QGIS 3.0 this functionality was available via QgsFileDownloader.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsFileDownloaderDialog : public QProgressDialog
{
    Q_OBJECT
  public:

    /**
     * QgsFileDownloader
     * \param url the download url
     * \param outputFileName file name where the downloaded content will be stored
     * \param authcfg optionally apply this authentication configuration
     */
    QgsFileDownloaderDialog( const QUrl &url, const QString &outputFileName, const QString &authcfg = QString() );

  signals:
    //! Emitted when the download has completed successfully
    void downloadCompleted();
    //! Emitted always when the downloader exits
    void downloadExited();
    //! Emitted when the download was canceled by the user
    void downloadCanceled();
    //! Emitted when an error makes the download fail
    void downloadError( QStringList errorMessages );
    //! Emitted when data are ready to be processed
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

  private slots:

    void onError( const QStringList &errors );
    void onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );

  private:

    QString mOutputFileName;
    QgsFileDownloader *mDownloader = nullptr;

};

#endif // QGSFILEDOWNLOADERDIALOG_H
