/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetcher.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsNetworkContentFetcher : QObject
{
%Docstring
 HTTP network content fetcher. A simple method for fetching remote HTTP content
and converting the content to standard formats. Url redirects are automatically
handled.
.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgsnetworkcontentfetcher.h"
%End
  public:

    QgsNetworkContentFetcher();
%Docstring
 Constructor for QgsNetworkContentFetcher.
%End

    virtual ~QgsNetworkContentFetcher();

    void fetchContent( const QUrl &url );
%Docstring
 Fetches content from a remote URL and handles redirects. The finished()
 signal will be emitted when content has been fetched.
 \param url URL to fetch
%End

    QNetworkReply *reply();
%Docstring
 Returns a reference to the network reply
 :return: QNetworkReply for fetched URL content
 :rtype: QNetworkReply
%End

    QString contentAsString() const;
%Docstring
 Returns the fetched content as a string
 :return: string containing network content
 :rtype: str
%End

  signals:

    void finished();
%Docstring
 Emitted when content has loaded
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetcher.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
