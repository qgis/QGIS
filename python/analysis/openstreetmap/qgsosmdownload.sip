/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmdownload.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsOSMDownload : QObject
{
%Docstring
 OSMDownload is a utility class for downloading OpenStreetMap via Overpass API.

 To use this class, it is necessary to set query, output file name and start the request.
 The interface is asynchronous, the caller has to wait for finished() signal that is
 emitted whe the request has finished (successfully or with an error).

 To check whether the the request has been successful, check hasError() and use errorString()
 to retrieve error message. An error may happen either directly in start() method
 or during the network communication.

 By default OSMDownload uses remote service at location returned by defaultServiceUrl() method.
%End

%TypeHeaderCode
#include "qgsosmdownload.h"
%End
  public:

    static QString defaultServiceUrl();
%Docstring
Return URL of the service that is used by default
 :rtype: str
%End

    static QString queryFromRect( const QgsRectangle &rect );
%Docstring
Create query (in Overpass Query Language) that fetches everything in given rectangle
 :rtype: str
%End

    QgsOSMDownload();

    QgsOSMDownload( const QString &query );
%Docstring
 Constructor for QgsOSMDownload
 \param query The query to execute in the Overpass API.

.. versionadded:: 3.0
%End

    ~QgsOSMDownload();

    void setServiceUrl( const QString &serviceUrl );
    QString serviceUrl() const;
%Docstring
 :rtype: str
%End

    void setQuery( const QString &query );
    QString query() const;
%Docstring
 :rtype: str
%End

    void setOutputFileName( const QString &outputFileName );
    QString outputFileName() const;
%Docstring
 :rtype: str
%End

    bool hasError() const;
%Docstring
 :rtype: bool
%End
    QString errorString() const;
%Docstring
 :rtype: str
%End

    bool start();
%Docstring
 Starts network request for data. The prerequisite is that the query string and output
 file name have been set.

 Only one request may be pending at one point - if you need more requests at once, use several instances.

 :return: true if the network request has been issued, false otherwise (and sets error string)
 :rtype: bool
%End

    bool abort();
%Docstring
 Aborts current pending request
 :return: true if there is a pending request and has been aborted, false otherwise
 :rtype: bool
%End

    bool isFinished() const;
%Docstring
Returns true if the request has already finished
 :rtype: bool
%End

  signals:
    void finished();
%Docstring
Emitted when the network reply has finished (with success or with an error)
%End

    void downloadProgress( qint64, qint64 );
%Docstring
Normally the total length is not known (until we reach end)
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmdownload.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
