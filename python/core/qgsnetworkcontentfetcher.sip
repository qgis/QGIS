/**
  \class QgsNetworkContentFetcher
  \ingroup core
  \brief HTTP network content fetcher. A simple method for fetching remote HTTP content
  and converting the content to standard formats. Url redirects are automatically
  handled.
  \since 2.5
*/

class QgsNetworkContentFetcher : QObject
{
%TypeHeaderCode
#include <qgsnetworkcontentfetcher.h>
%End

  public:
    QgsNetworkContentFetcher();

    virtual ~QgsNetworkContentFetcher();

    /** Fetches content from a remote URL and handles redirects. The finished()
     * signal will be emitted when content has been fetched.
     * @param url URL to fetch
    */
    void fetchContent( const QUrl url );

    /** Returns a reference to the network reply
     * @returns QNetworkReply for fetched URL content
    */
    QNetworkReply* reply();

    /** Returns the fetched content as a string
     * @returns string containing network content
    */
    QString contentAsString() const;

  signals:

    /** Emitted when content has loaded
    */
    void finished();

};
