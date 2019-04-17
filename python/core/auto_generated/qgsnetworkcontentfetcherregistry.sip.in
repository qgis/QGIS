/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetcherregistry.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsFetchedContent : QObject
{
%Docstring
FetchedContent holds useful information about a network content being fetched

.. seealso:: :py:class:`QgsNetworkContentFetcherRegistry`

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsnetworkcontentfetcherregistry.h"
%End
  public:
    enum ContentStatus
    {
      NotStarted,
      Downloading,
      Finished,
      Failed
    };

    explicit QgsFetchedContent( const QString &url, QTemporaryFile *file = 0, ContentStatus status = NotStarted );
%Docstring
Constructs a FetchedContent with pointer to the downloaded file and status of the download
%End

    ~QgsFetchedContent();



    const QString filePath() const;
%Docstring
Returns the path to the local file, an empty string if the file is not accessible yet.
%End

    ContentStatus status() const;
%Docstring
Returns the status of the download
%End

    QNetworkReply::NetworkError error() const;
%Docstring
Returns the potential error of the download
%End

  public slots:

    void download( bool redownload = false );
%Docstring
Start the download

:param redownload: if set to ``True``, it will restart any achieved or pending download.
%End

    void cancel();
%Docstring
Cancel the download operation
%End

  signals:
    void fetched();
%Docstring
Emitted when the file is fetched and accessible
%End

};

class QgsNetworkContentFetcherRegistry : QObject
{
%Docstring
Registry for temporary fetched files

This provides a simple way of downloading and accessing
remote files during QGIS application running.

.. seealso:: :py:class:`QgsFetchedContent`

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsnetworkcontentfetcherregistry.h"
%End
  public:
    enum FetchingMode
    {
      DownloadLater,
      DownloadImmediately,
    };

    explicit QgsNetworkContentFetcherRegistry();
%Docstring
Create the registry for temporary downloaded files
%End

    ~QgsNetworkContentFetcherRegistry();

    const QgsFetchedContent *fetch( const QString &url, FetchingMode fetchingMode = DownloadLater );
%Docstring
Initialize a download for the given URL

:param url: the URL to be fetched
:param fetchingMode: defines if the download will start immediately or shall be manually triggered

.. note::

   If the download starts immediately, it will not redownload any already fetched or currently fetching file.
%End


    QString localPath( const QString &filePathOrUrl );
%Docstring
Returns the path to a local file or to a temporary file previously fetched by the registry

:param filePathOrUrl: can either be a local file path or a remote content which has previously been fetched
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetcherregistry.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
