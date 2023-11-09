/***************************************************************************
                             qgsprovidersublayertask.h
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSPROVIDERSUBLAYERTASKTASK_H
#define QGSPROVIDERSUBLAYERTASKTASK_H

#include "qgstaskmanager.h"
#include <QReadWriteLock>
#include <memory>

class QgsFeedback;
class QgsProviderSublayerDetails;

/**
 * \ingroup core
 *
 * \brief A QgsTask which retrieves sublayer details for a URI.
 *
 * This task executes a call to QgsProviderRegistry::querySublayers() in a background
 * thread. Depending on the URI queried it can be expensive to calculate the sublayers
 * (e.g. in the case where a full table scan is required to resolve mixed geometry
 * type layers), so it is beneficial to perform these queries in the background wherever
 * possible.
 *
 * While QgsProviderRegistry::querySublayers() offers various flags to control
 * how in-depth the querying will be, these flags are not exposed through QgsProviderSublayerTask.
 * Rather QgsProviderSublayerTask will always execute the most thorough query
 * possible, regardless of how expensive this may be.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProviderSublayerTask, which retrieves sublayer details for the
     * specified \a uri.
     */
    QgsProviderSublayerTask( const QString &uri, bool includeSystemTables = false );

    /**
     * Constructor for QgsProviderSublayerTask, which retrieves sublayer details for the
     * specified \a uri, restricted to a particular provider.
     *
     * \since QGIS 3.30
     */
    QgsProviderSublayerTask( const QString &uri, const QString &providerKey, bool includeSystemTables = false );

    ~QgsProviderSublayerTask() override;

    /**
     * Returns the sublayer details as calculated by the task.
     */
    QList<QgsProviderSublayerDetails> results() const;

    void cancel() override;

  protected:

    bool run() override;

  private:

    QString mUri;

    QString mProviderKey;

    bool mIncludeSystemTables = false;

    std::unique_ptr< QgsFeedback > mFeedback;

    QList<QgsProviderSublayerDetails> mResults;

    mutable QReadWriteLock mLock;

};

#endif // QGSPROVIDERSUBLAYERTASKTASK_H
