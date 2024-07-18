/***************************************************************************
  qgsrunnableprovidercreator.h - QgsRunnableProviderCreator

 ---------------------
 begin                : 20.3.2023
 copyright            : (C) 2023 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRUNNABLEPROVIDERCREATOR_H
#define QGSRUNNABLEPROVIDERCREATOR_H

#include <QRunnable>
#include <QObject>

#include "qgsdataprovider.h"

#define SIP_NO_FILE

/**
 * \ingroup core
 * \brief The QgsRunnableProviderCreator class is used when reading a project
 * to create asynchronously provider that support asynchronous creation
 *
 * \since QGIS 3.32
 */
class QgsRunnableProviderCreator : public QObject, public QRunnable
{
    Q_OBJECT
  public:

    //! Constructor
    QgsRunnableProviderCreator( const QString &layerId,
                                QString const &providerKey,
                                QString const &dataSource,
                                const QgsDataProvider::ProviderOptions &options,
                                Qgis::DataProviderReadFlags flags );

    void run() override;

    /**
     * Returns the created data provider.
     * Caller takes ownership and as the dataprovider object was moved to the thread
     * where the QgsRunnableProviderCreator was created, the caller has to be in this thread.
     */
    QgsDataProvider *dataProvider();

  signals:
    //! Emitted when a provider is created with \a isValid set to True when the provider is valid
    void providerCreated( bool isValid, const QString &layerId );

  private:
    std::unique_ptr<QgsDataProvider> mDataProvider;
    QString mLayerId;
    QString mProviderKey;
    QString mDataSource;
    QgsDataProvider::ProviderOptions mOptions;
    Qgis::DataProviderReadFlags mFlags;
};

#endif // QGSRUNNABLEPROVIDERCREATOR_H
