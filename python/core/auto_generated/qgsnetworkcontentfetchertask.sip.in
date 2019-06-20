/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetchertask.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsNetworkContentFetcherTask : QgsTask
{
%Docstring
Handles HTTP network content fetching in a background task.

Provides a simple method for fetching remote HTTP content in a QgsTask.
Url redirects are automatically handled.

After constructing a QgsNetworkContentFetcherTask, callers should
connect to the QgsNetworkContentFetcherTask.fetched signal. They can
then safely access the network reply() from the connected slot
without danger of the task being first removed by the QgsTaskManager.

.. seealso:: :py:class:`QgsNetworkContentFetcher`

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsnetworkcontentfetchertask.h"
%End
  public:

    QgsNetworkContentFetcherTask( const QUrl &url );
%Docstring
Constructor for a QgsNetworkContentFetcherTask which fetches
the specified ``url``.
%End

    QgsNetworkContentFetcherTask( const QNetworkRequest &request );
%Docstring
Constructor for a QgsNetworkContentFetcherTask which fetches
the specified network ``request``.
%End

    ~QgsNetworkContentFetcherTask();

    virtual bool run();

    virtual void cancel();


    QNetworkReply *reply();
%Docstring
Returns the network reply. Ownership is not transferred.

May return ``None`` if the request has not yet completed.
%End

  signals:

    void fetched();
%Docstring
Emitted when the network content has been fetched, regardless
of whether the fetch was successful or not.

Users of QgsNetworkContentFetcherTask should connect to this signal,
and from the associated slot they can then safely access the network reply()
without danger of the task being first removed by the :py:class:`QgsTaskManager`.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkcontentfetchertask.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
