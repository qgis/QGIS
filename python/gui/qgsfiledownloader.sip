/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledownloader.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsFileDownloader : QObject
{
%Docstring
 QgsFileDownloader is a utility class for downloading files.

 To use this class, it is necessary to pass the URL and an output file name as
 arguments to the constructor, the download will start immediately.
 The download is asynchronous and depending on the guiNotificationsEnabled
 parameter accepted by the constructor (default = true) the class will
 show a progress dialog and report all errors in a QMessageBox.warning dialog.
 If the guiNotificationsEnabled parameter is set to false, the class can still
 be used through the signals and slots mechanism.
 The object will destroy itself when the request completes, errors or is canceled.
 An optional authentication configuration can be specified.

.. versionadded:: 2.18.1
%End

%TypeHeaderCode
#include "qgsfiledownloader.h"
%End
  public:

    QgsFileDownloader( const QUrl &url, const QString &outputFileName, bool guiNotificationsEnabled = true, QString authcfg = QString() );
%Docstring
 QgsFileDownloader
 \param url the download url
 \param outputFileName file name where the downloaded content will be stored
 \param guiNotificationsEnabled if false, the downloader will not display any progress bar or error message
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

  public slots:

    void onDownloadCanceled();
%Docstring
 Called when a download is canceled by the user
 this slot aborts the download and deletes
 the object.
 Never call this slot directly: this is meant to
 be managed by the signal-slot system.
%End

  protected:
    ~QgsFileDownloader();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledownloader.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
