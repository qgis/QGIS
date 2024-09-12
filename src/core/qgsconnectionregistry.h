/***************************************************************************
                         qgsconnectionregistry.h
                         ------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSCONNECTIONREGISTRY_H
#define QGSCONNECTIONREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"
#include <QObject>

class QgsAbstractProviderConnection;


/**
 * \class QgsConnectionRegistry
 * \ingroup core
 * \brief A registry for saved data provider connections, allowing retrieval of
 * saved connections by name and provider type.
 *
 * QgsConnectionRegistry is not usually directly created, but rather accessed through
 * QgsApplication::connectionRegistry().
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsConnectionRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsConnectionRegistry.
     */
    QgsConnectionRegistry( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QgsConnectionRegistry( const QgsConnectionRegistry &other ) = delete;
    QgsConnectionRegistry &operator=( const QgsConnectionRegistry &other ) = delete;

    /**
     * Creates a new connection by loading the connection with the given \a id from the settings.
     *
     * The \a id string must be of the format "provider://connection_name", e.g. "postgres://my_connection" for
     * the PostgreSQL connection saved as "my_connection".
     *
     * Ownership is transferred to the caller.
     *
     * \throws QgsProviderConnectionException
     */
    QgsAbstractProviderConnection *createConnection( const QString &name ) SIP_THROW( QgsProviderConnectionException ) SIP_FACTORY;

  private:

#ifdef SIP_RUN
    QgsConnectionRegistry( const QgsConnectionRegistry &other );
#endif
};

#endif // QGSCONNECTIONREGISTRY_H


