/***************************************************************************
                       qgsnetworkcontentfetchertask.h
                             -------------------
    begin                : March, 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSNETWORKCONTENTFETCHERTASK_H
#define QGSNETWORKCONTENTFETCHERTASK_H

#include "qgstaskmanager.h"
#include "qgis_core.h"
#include <QNetworkRequest>

class QgsNetworkContentFetcher;
class QNetworkReply;

/**
 * \class QgsNetworkContentFetcherTask
 * \ingroup core
 * \brief Handles HTTP network content fetching in a background task.
 *
 * Provides a simple method for fetching remote HTTP content in a QgsTask.
 * Url redirects are automatically handled.
 *
 * After constructing a QgsNetworkContentFetcherTask, callers should
 * connect to the QgsNetworkContentFetcherTask::fetched signal. They can
 * then safely access the network reply() from the connected slot
 * without danger of the task being first removed by the QgsTaskManager.
 *
 * \see QgsNetworkContentFetcher
 *
 * \since QGIS 3.2
*/
class CORE_EXPORT QgsNetworkContentFetcherTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for a QgsNetworkContentFetcherTask which fetches
     * the specified \a url.
     */
    QgsNetworkContentFetcherTask( const QUrl &url );

    /**
     * Constructor for a QgsNetworkContentFetcherTask which fetches
     * the specified network \a request.
     */
    QgsNetworkContentFetcherTask( const QNetworkRequest &request );

    ~QgsNetworkContentFetcherTask() override;

    bool run() override;
    void cancel() override;

    /**
     * Returns the network reply. Ownership is not transferred.
     *
     * May return NULLPTR if the request has not yet completed.
     */
    QNetworkReply *reply();

  signals:

    /**
     * Emitted when the network content has been fetched, regardless
     * of whether the fetch was successful or not.
     *
     * Users of QgsNetworkContentFetcherTask should connect to this signal,
     * and from the associated slot they can then safely access the network reply()
     * without danger of the task being first removed by the QgsTaskManager.
     */
    void fetched();

  private:

    QNetworkRequest mRequest;
    QgsNetworkContentFetcher *mFetcher = nullptr;

};

#endif //QGSNETWORKCONTENTFETCHERTASK_H
