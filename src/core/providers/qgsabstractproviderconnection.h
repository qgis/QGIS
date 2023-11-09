/***************************************************************************
  qgsabstractproviderconnection.h - QgsAbstractProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSABSTRACTPROVIDERCONNECTION_H
#define QGSABSTRACTPROVIDERCONNECTION_H

#include <QString>
#include <QVariantMap>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \brief The QgsAbstractProviderConnection provides an interface for data provider connections.
 *
 * Connections objects can be constructed loading them from the connections stored
 * in the settings by passing the connection name.
 * A new connection object can also be created by passing a data source URI in the constructor.
 *
 * Provider metadata keep a cache of the existing connections, to manage stored
 * connections it is recommendend to call metadata methods instead of loading and
 * storing the connections directly.
 *
 * Concrete classes must implement methods to retrieve, save and remove connections from
 * the settings.
 *
 * \ingroup core
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsAbstractProviderConnection
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsAbstractDatabaseProviderConnection *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsAbstractDatabaseProviderConnection;
    }
    else if ( dynamic_cast<QgsAbstractProviderConnection *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsAbstractProviderConnection;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Creates a new connection with \a name by reading its configuration from the settings.
     * If a connection with this name cannot be found, an empty connection will be returned.
     */
    QgsAbstractProviderConnection( const QString &name );

    /**
     * Creates a new connection from the given \a uri and \a configuration.
     * The connection is not automatically stored in the settings.
     * \see store()
     */
    QgsAbstractProviderConnection( const QString &uri, const QVariantMap &configuration );

    virtual ~QgsAbstractProviderConnection() = default;

    /**
     * Stores the connection in the settings.
     * \param name the name under which the connection will be stored
     */
    virtual void store( const QString &name ) const = 0;

    /**
     * Deletes the connection from the settings.
     */
    virtual void remove( const QString &name ) const = 0;

    /**
     * Returns an icon representing the connection.
     */
    virtual QIcon icon() const;

    /**
     * Returns the connection data source URI string representation
     */
    QString uri() const;

    /**
     * Sets the connection data source URI to \a uri
     */
    void setUri( const QString &uri );

    /**
     * Returns the connection configuration parameters
     */
    QVariantMap configuration() const;

    /**
     * Sets the connection \a configuration
     */
    void setConfiguration( const QVariantMap &configuration );

  private:

    QString mUri;
    QVariantMap mConfiguration;

};

#endif // QGSABSTRACTPROVIDERCONNECTION_H
