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
#include "qgsdatasourceuri.h"
#include "qgsexception.h"

/**
 * The QgsAbstractProviderConnection provides an interface for data provider connections.
 *
 * Connections objects can be created by passing the connection name and in this case
 * they are automatically loaded from the settings, or by passing a data source URI
 * in the constructor.
 *
 * Concrete classes must implement methods to retrieve, save and remove connections from
 * the settings.
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
     * Creates a new connection with \a name and initializes the connection from the \a uri.
     * The connection is not automatically stored in the settings.
     * \see store()
     */
    QgsAbstractProviderConnection( const QString &name, const QString &uri );

    virtual ~QgsAbstractProviderConnection() = default;

    /**
     * Stores the connection in the settings.
     * \param configuration stores additional connection settings that are used by the
     * source select dialog and are not part of the data source URI
     */
    virtual void store( const QVariantMap &configuration = QVariantMap() ) const = 0;

    /**
     * Deletes the connection from the settings.
     */
    virtual void remove( ) const = 0;

    /**
     * Returns the connection name
     */
    QString name() const;

    /**
     * Returns the connection data source URI string representation
     */
    QString uri() const;

    /**
     * Sets the connection data source URI to \a uri
     */
    void setUri( const QString &uri );

  private:

    QString mConnectionName;
    QString mUri;

};

#endif // QGSABSTRACTPROVIDERCONNECTION_H
