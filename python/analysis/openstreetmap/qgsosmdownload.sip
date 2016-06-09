/**
 * @brief OSMDownload is a utility class for downloading OpenStreetMap via Overpass API.
 *
 * To use this class, it is necessary to set query, output file name and start the request.
 * The interface is asynchronous, the caller has to wait for finished() signal that is
 * emitted whe the request has finished (successfully or with an error).
 *
 * To check whether the the request has been successful, check hasError() and use errorString()
 * to retrieve error message. An error may happen either directly in start() method
 * or during the network communication.
 *
 * By default OSMDownload uses remote service at location returned by defaultServiceUrl() method.
 */
class QgsOSMDownload : QObject
{
%TypeHeaderCode
#include <qgsosmdownload.h>
%End
  public:

    //! Return URL of the service that is used by default
    static QString defaultServiceUrl();

    //! Create query (in Overpass Query Language) that fetches everything in given rectangle
    static QString queryFromRect( const QgsRectangle& rect );

    QgsOSMDownload();
    ~QgsOSMDownload();

    void setServiceUrl( const QString& serviceUrl );
    QString serviceUrl() const;

    void setQuery( const QString& query );
    QString query() const;

    void setOutputFileName( const QString& outputFileName );
    QString outputFileName() const;

    bool hasError() const;
    QString errorString() const;

    /**
     * @brief Starts network request for data. The prerequisite is that the query string and output
     * file name have been set.
     *
     * Only one request may be pending at one point - if you need more requests at once, use several instances.
     *
     * @return true if the network request has been issued, false otherwise (and sets error string)
     */
    bool start();

    /**
     * @brief Aborts current pending request
     * @return true if there is a pending request and has been aborted, false otherwise
     */
    bool abort();

    //! Returns true if the request has already finished
    bool isFinished() const;

  signals:
    void finished(); //!< emitted when the network reply has finished (with success or with an error)
    void downloadProgress( qint64, qint64 ); //!< normally the total length is not known (until we reach end)

};
