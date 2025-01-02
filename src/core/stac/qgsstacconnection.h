/***************************************************************************
    qgsstacconnection.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACCONNECTION_H
#define QGSSTACCONNECTION_H


#include "qgis_core.h"
#include "qgssettingstree.h"
#include "qgssettingstreenode.h"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QStringList>

#include "qgsabstractproviderconnection.h"
#include "qgshttpheaders.h"

class QgsSettingsEntryString;
class QgsSettingsEntryVariantMap;

/**
 * \brief Represents connections to STAC catalogs.
 *
 * \ingroup core
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacConnection : public QgsAbstractProviderConnection
{

  public:

#ifndef SIP_RUN

    ///@cond PRIVATE
    static inline QgsSettingsTreeNamedListNode *sTreeConnectionStac = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "stac" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryString *settingsAuthcfg;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryVariantMap *settingsHeaders;

    ///@endcond PRIVATE
#endif

    /**
     * Constructor for QgsStacConnection, using the stored settings with the specified connection \a name.
     */
    QgsStacConnection( const QString &name );

    /**
     * Constructor for QgsStacConnection, using the a specific connection details.
     */
    QgsStacConnection( const QString &uri, const QVariantMap &configuration );

    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;

    //! Represents decoded data of a connection
    struct Data
    {
      //! Catalog URL
      QString url;
      //! Authentication configuration id
      QString authCfg;
      //! HTTP Basic username
      QString username;
      //! HTTP Basic password
      QString password;
      //! HTTP headers
      QgsHttpHeaders httpHeaders;
    };

    //! Returns connection data encoded as a string
    static QString encodedUri( const Data &conn );
    //! Decodes connection string to a data structure
    static Data decodedUri( const QString &uri );

    //! Returns list of existing connections, unless the hidden ones
    static QStringList connectionList();
    //! Returns connection details
    static Data connection( const QString &name );
    //! Removes a connection from the list
    static void deleteConnection( const QString &name );
    //! Adds a new connection to the list
    static void addConnection( const QString &name, const Data &conn );
    //! Returns last used connection
    static QString selectedConnection();
    //! Saves name of the last used connection
    static void setSelectedConnection( const QString &connName );
};

///@endcond

#endif // QGSSTACCONNECTION_H
