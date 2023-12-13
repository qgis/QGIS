/***************************************************************************
    qgssensorthingsconnection.h
    ---------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSCONNECTION_H

#define SIP_NO_FILE

#include <QStringList>

#include "qgshttpheaders.h"
#include "qgis_core.h"
#include "qgssettingstree.h"

class QgsSettingsEntryBool;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
class QgsSettingsEntryVariantMap;
template<class T> class QgsSettingsEntryEnumFlag;

///@cond PRIVATE

class CORE_EXPORT QgsSensorThingsConnection
{

  public:

    static inline QgsSettingsTreeNamedListNode *sTreeSensorThingsConnections = QgsSettingsTree::sTreeConnections->createNamedListNode(
          QStringLiteral( "sensorthings" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryVariantMap *settingsHeaders;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryString *settingsAuthcfg;

    QString name;
    QString url;
    // Authentication configuration id
    QString authCfg;
    // HTTP Basic username
    QString username;
    // HTTP Basic password
    QString password;
    // http headers
    QgsHttpHeaders httpHeaders;

    QString encodedUri() const;
};

//! Utility class for handling list of connections to sensor things layers
class CORE_EXPORT QgsSensorThingsConnectionUtils
{

  public:
    //! Returns list of existing connections, unless the hidden ones
    static QStringList connectionList();

    //! Returns connection details
    static QgsSensorThingsConnection connection( const QString &name );

    //! Removes a connection from the list
    static void deleteConnection( const QString &name );

    //! Adds a new connection to the list
    static void addConnection( const QgsSensorThingsConnection &conn );
};

///@endcond PRIVATE

#endif // QGSSENSORTHINGSCONNECTION_H
