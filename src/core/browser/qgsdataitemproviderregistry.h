/***************************************************************************
  qgsdataitemproviderregistry.h
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMPROVIDERREGISTRY_H
#define QGSDATAITEMPROVIDERREGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>

#include "qgis_sip.h"

#include "qgis_core.h"

class QgsDataItemProvider;

/**
 * \ingroup core
 * \brief This class keeps a list of data item providers that may add items to the browser tree.
 *
 * When created, it automatically adds providers from provider plugins (e.g. PostGIS, WMS, ...)
 *
 * QgsDataItemProviderRegistry is not usually directly created, but rather accessed through
 * QgsApplication::dataItemProviderRegistry().
 *
 */
class CORE_EXPORT QgsDataItemProviderRegistry : public QObject
{
    Q_OBJECT
  public:

    QgsDataItemProviderRegistry();

    ~QgsDataItemProviderRegistry();

    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry &rh ) = delete;
    QgsDataItemProviderRegistry &operator=( const QgsDataItemProviderRegistry &rh ) = delete;

    /**
     * Returns the list of available providers.
     */
    QList<QgsDataItemProvider *> providers() const;

    /**
     * Returns the (possibly NULLPTR) data item provider named \a providerName
     * \since QGIS 3.14
     */
    QgsDataItemProvider *provider( const QString &providerName ) const;

    /**
     * Adds a \a provider implementation to the registry. Ownership of the provider
     * is transferred to the registry.
     */
    void addProvider( QgsDataItemProvider *provider SIP_TRANSFER );

    /**
     * Removes a \a provider implementation from the registry.
     * The provider object is automatically deleted.
     */
    void removeProvider( QgsDataItemProvider *provider );

    /**
     * Returns the (possibly blank) data provider key for a given data item provider name.
     *
     * \param dataItemProviderName name of the data item provider
     * \since QGIS 3.14
     */
    QString dataProviderKey( const QString &dataItemProviderName );

  signals:

    /**
     * Emitted when a new data item provider has been added.
     * \since QGIS 3.14
     */
    void providerAdded( QgsDataItemProvider *provider );

    /**
     * Emitted when a data item provider is about to be removed
     * \since QGIS 3.14
     */
    void providerWillBeRemoved( QgsDataItemProvider *provider );

  private:
#ifdef SIP_RUN
    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry &rh );
#endif

    //! Available providers, owned by this class
    QList<QgsDataItemProvider *> mProviders;

    //! Keeps track of data item provider <-> data provider association
    QMap<QString, QString> mDataItemProviderOrigin;

};

#endif // QGSDATAITEMPROVIDERREGISTRY_H
