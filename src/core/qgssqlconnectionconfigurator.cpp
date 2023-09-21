/***************************************************************************
  qgspostgresconnectionconfigurator.cpp  -  connection configurator class for
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

#include "qgssqlconnectionconfigurator.h"

const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sService = new QgsSettingsEntryString( QStringLiteral( "service" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sHost = new QgsSettingsEntryString( QStringLiteral( "host" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sPort = new QgsSettingsEntryString( QStringLiteral( "port" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sDatabase = new QgsSettingsEntryString( QStringLiteral( "database" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sSessionRole = new QgsSettingsEntryString( QStringLiteral( "session_role" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeConnections );
const QgsSettingsEntryEnumFlag<QgsDataSourceUri::SslMode> *QgsPostgreSqlConnectionSettings::sSslMode = new QgsSettingsEntryEnumFlag<QgsDataSourceUri::SslMode>( QStringLiteral( "sslmode" ), sTreeConnections, QgsDataSourceUri::SslPrefer );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sPublicOnly = new QgsSettingsEntryBool( QStringLiteral( "publicOnly" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sGeometryColumnsOnly = new QgsSettingsEntryBool( QStringLiteral( "geometryColumnsOnly" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sDontResolveType = new QgsSettingsEntryBool( QStringLiteral( "dontResolveType" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sAllowGeometrylessTables = new QgsSettingsEntryBool( QStringLiteral( "allowGeometrylessTables" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sEstimatedMetadata = new QgsSettingsEntryBool( QStringLiteral( "estimatedMetadata" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sSaveUsername = new QgsSettingsEntryBool( QStringLiteral( "saveUsername" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sSavePassword = new QgsSettingsEntryBool( QStringLiteral( "savePassword" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sOldSave = new QgsSettingsEntryBool( QStringLiteral( "save" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sProjectsInDatabase = new QgsSettingsEntryBool( QStringLiteral( "projectsInDatabase" ), sTreeConnections );
const QgsSettingsEntryString *QgsPostgreSqlConnectionSettings::sAuthCfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeConnections );
const QgsSettingsEntryStringList *QgsPostgreSqlConnectionSettings::sKeys = new QgsSettingsEntryStringList( QStringLiteral( "keys" ), sTreeConnections );
const QgsSettingsEntryBool *QgsPostgreSqlConnectionSettings::sMetadataInDatabase = new QgsSettingsEntryBool( QStringLiteral( "metadataInDatabase" ), sTreeConnections );
const QgsSettingsEntryInteger *QgsPostgreSqlConnectionSettings::sDefaultTimeout = new QgsSettingsEntryInteger( QStringLiteral( "default_timeout" ), sTreeConnections, 30 );
const QString QgsPostgreSqlConnectionSettings::mConnectionTypeName = "PostgreSQL";
const QString QgsPostgreSqlConnectionSettings::mConnectionTypePort = "5432";
