/***************************************************************************
  qgspostgresconnectionconfigurator.h  -  connection configurator class for
  PostgeSQL based connectors.
                             -------------------
    begin                : 2023/06/06
    copyright            : (C) 2023 by Alexey Karandashev
    email                : reflectored at pm dot me
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLCONNECTIONCONFIGURATOR_H
#define QGSSQLCONNECTIONCONFIGURATOR_H
#define SIP_NO_FILE

#include <QString>
#include <QtCore/qstring.h>
#include <QtCore/qstringliteral.h>

#include "qgsdatasourceuri.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgssettings.h"

/**
 * \ingroup core
 * \brief Global PostgreSQL provider connection settings.
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsPostgreSqlConnectionSettings SIP_SKIP
{
  public:
    static inline QgsSettingsTreeNamedListNode *sTreeConnections = QgsSettingsTree::sTreeConnections->createNamedListNode( "PostgreSQL", Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );
    static const QgsSettingsEntryString *sService;
    static const QgsSettingsEntryString *sHost;
    static const QgsSettingsEntryString *sPort;
    static const QgsSettingsEntryString *sDatabase;
    static const QgsSettingsEntryString *sSessionRole;
    static const QgsSettingsEntryString *sUsername;
    static const QgsSettingsEntryString *sPassword;
    static const QgsSettingsEntryEnumFlag<QgsDataSourceUri::SslMode> *sSslMode;
    static const QgsSettingsEntryBool *sPublicOnly;
    static const QgsSettingsEntryBool *sGeometryColumnsOnly;
    static const QgsSettingsEntryBool *sDontResolveType;
    static const QgsSettingsEntryBool *sAllowGeometrylessTables;
    static const QgsSettingsEntryBool *sEstimatedMetadata;
    static const QgsSettingsEntryBool *sSaveUsername;
    static const QgsSettingsEntryBool *sSavePassword;
    static const QgsSettingsEntryBool *sOldSave;
    static const QgsSettingsEntryBool *sProjectsInDatabase;
    static const QgsSettingsEntryString *sAuthCfg;
    static const QgsSettingsEntryStringList *sKeys;
    static const QgsSettingsEntryBool *sMetadataInDatabase;
    static const QgsSettingsEntryInteger *sDefaultTimeout;
    static const QString mConnectionTypeName;
    static const QString mConnectionTypePort;
};

/**
 * \ingroup core
 * \brief A template implementation of A SQL based provider connection configurator.
 * \since QGIS 3.32
 */
template <typename T>
class CORE_EXPORT QgsSqlConnectionConfigurator SIP_SKIP
{
  public:

    /**
     * \returns QStringList of the provider's connections.
     */
    static QStringList connectionList()
    {
      return T::sTreeConnections->items();
    }

    /**
     * \returns QString of the provider's selected connection.
     */
    static QString selectedConnection()
    {
      return T::sTreeConnections->selectedItem();
    }

    /**
     * Selects a connection for the provider.
     *
     * \param name the name of the connection to be selected.
     */
    static void setSelectedConnection( const QString &name )
    {
      T::sTreeConnections->setSelectedItem( name );
    }

    /**
     * Generate a URI for the given connection.
     *
     * \param connectionName of the connection.
     *
     * \returns QgsDataSourceUri a URI of the connection.
     */
    static QgsDataSourceUri connUri( const QString &connectionName )
    {
      QgsDebugMsgLevel( "theConnName = " + connectionName, 2 );

      const QString service = T::sService->value( connectionName );
      const QString host = T::sHost->value( connectionName );
      QString port = T::sPort->value( connectionName );
      if ( port.length() == 0 )
      {
        port = T::mConnectionTypePort;
      }
      const QString database = T::sDatabase->value( connectionName );

      const bool estimatedMetadata = useEstimatedMetadata( connectionName );
      QgsDataSourceUri::SslMode sslmode = T::sSslMode->valueWithDefaultOverride( QgsDataSourceUri::SslPrefer, connectionName );

      QString username;
      QString password;
      if ( T::sSaveUsername->value( connectionName ) )
      {
        username = T::sUsername->value( connectionName );
      }

      if ( T::sSavePassword->value( connectionName ) )
      {
        password = T::sPassword->value( connectionName );
      }

      // Old save setting
      if ( T::sOldSave->exists( connectionName ) )
      {
        username = T::sUsername->value( connectionName );

        if ( T::sOldSave->value( connectionName ) )
        {
          password = T::sPassword->value( connectionName );
        }
      }

      const QString authcfg = T::sAuthCfg->value( connectionName );

      QgsDataSourceUri uri;
      if ( !service.isEmpty() )
      {
        uri.setConnection( service, database, username, password, sslmode, authcfg );
      }
      else
      {
        uri.setConnection( host, port, database, username, password, sslmode, authcfg );
      }
      uri.setUseEstimatedMetadata( estimatedMetadata );

      return uri;
    }

    /**
     * Returns the publicOnly setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff public schema only setting is TRUE.
     */
    static bool publicSchemaOnly( const QString &connectionName )
    {
      return T::sPublicOnly->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Returns the geometryColumnsOnly setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff geometry columns only setting is TRUE.
     */
    static bool geometryColumnsOnly( const QString &connectionName )
    {

      return T::sGeometryColumnsOnly->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Returns the dontResolveType setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff don't resolve type setting is TRUE.
     */
    static bool dontResolveType( const QString &connectionName )
    {

      return T::sDontResolveType->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Returns the useEstimatedMetadata setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff use estimated metadata setting is TRUE.
     */
    static bool useEstimatedMetadata( const QString &connectionName )
    {

      return T::sEstimatedMetadata->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Returns the allowGeometrylessTables setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff allow geometryless tables setting is TRUE.
     */
    static bool allowGeometrylessTables( const QString &connectionName )
    {
      return T::sAllowGeometrylessTables->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Returns the allowProjectsInDatabase setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff allow projects in database setting is TRUE.
     */
    static bool allowProjectsInDatabase( const QString &connectionName )
    {
      return T::sProjectsInDatabase->valueWithDefaultOverride( false, connectionName );
    }

    /**
     * Delete a connection for the provider.
     *
     * \param connectionName of the connection.
     */
    static void deleteConnection( const QString &connectionName )
    {
      T::sTreeConnections->deleteItem( connectionName );
    }

    /**
     * Returns the metadataInDatabase setting for the connection.
     *
     * \param connectionName of the connection.
     *
     * \returns TRUE iff metadata in database setting is TRUE.
     */
    static bool allowMetadataInDatabase( const QString &connectionName )
    {
      return T::sMetadataInDatabase->valueWithDefaultOverride( false, connectionName );
    }
};

#endif
