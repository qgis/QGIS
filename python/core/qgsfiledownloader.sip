/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfiledownloader.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFileDownloader : QObject
{
%Docstring
QgsFileDownloader is a utility class for downloading files.

To use this class, it is necessary to pass the URL and an output file name as
arguments to the constructor, the download will start immediately.

The download is asynchronous.

The object will destroy itself when the request completes, errors or is canceled.
An optional authentication configuration can be specified.

.. note::

   This class was part of the GUI library from QGIS 2.18.1 until QGIS 3.0

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfiledownloader.h"
%End
  public:

    QgsFileDownloader( const QUrl &url, const QString &outputFileName, const QString &authcfg = QString(), bool delayStart = false );
%Docstring
QgsFileDownloader

:param url: the download url
:param outputFileName: file name where the downloaded content will be stored
:param authcfg: optionally apply this authentication configuration
:param delayStart: if true, the download will not be commenced immediately and must
be triggered by a later call to startDownload(). This can be useful if connections need
to be made to the downloader and there's a chance the download will emit
signals before these connections have been made.
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
Emitted when the download was canceled by the user.

.. seealso:: :py:func:`cancelDownload`
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

    void cancelDownload();
%Docstring
Call to abort the download and delete this object after the cancelation
has been processed.

.. seealso:: :py:func:`downloadCanceled`
%End

    void startDownload();
%Docstring
Called to start the download
%End

  protected:
    ~QgsFileDownloader();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfiledownloader.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
