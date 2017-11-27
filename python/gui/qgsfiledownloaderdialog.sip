/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledownloaderdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFileDownloaderDialog : QProgressDialog
{
%Docstring
 QgsFileDownloaderDialog is a QProgressDialog subclass which
 handles file downloads and user feedback.

 Internally, it uses QgsFileDownloader to handle the download,
 while showing progress via a progress dialog and supporting
 cancelation.

.. note::

   Until QGIS 3.0 this functionality was available via QgsFileDownloader.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfiledownloaderdialog.h"
%End
  public:

    QgsFileDownloaderDialog( const QUrl &url, const QString &outputFileName, const QString &authcfg = QString() );
%Docstring
 QgsFileDownloader
 \param url the download url
 \param outputFileName file name where the downloaded content will be stored
 \param authcfg optionally apply this authentication configuration
%End

  signals:
    void downloadCompleted();
%Docstring
Emitted when the download has completed successfully
%End
    void downloadExited();
%Docstring
Emitted always when the downloader exits
%End
    void downloadCanceled();
%Docstring
Emitted when the download was canceled by the user
%End
    void downloadError( QStringList errorMessages );
%Docstring
Emitted when an error makes the download fail
%End
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );
%Docstring
Emitted when data are ready to be processed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledownloaderdialog.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
