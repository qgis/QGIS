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
#include <memory>

class QgsFeedback;
class QgsProviderSublayerDetails;

/**
 * \ingroup core
 *
 * \brief A QgsTask which retrieves sublayer details for a URI.
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
    QgsProviderSublayerTask( const QString &uri );

    ~QgsProviderSublayerTask() override;

    /**
     * Return the sublayer details as calculated by the task.
     */
    QList<QgsProviderSublayerDetails> results() const;

    bool run() override;
    void cancel() override;

  private:

    QString mUri;

    std::unique_ptr< QgsFeedback > mFeedback;

    QList<QgsProviderSublayerDetails> mResults;

};

#endif // QGSPROVIDERSUBLAYERTASKTASK_H
