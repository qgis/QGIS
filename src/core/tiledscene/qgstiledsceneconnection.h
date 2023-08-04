/***************************************************************************
    qgstiledsceneconnection.h
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENECONNECTION_H
#define QGSTILEDSCENECONNECTION_H

#include "qgis_core.h"
#include "qgssettingstree.h"
#include "qgssettingstreenode.h"

#define SIP_NO_FILE

#include <QStringList>

#include "qgsabstractproviderconnection.h"
#include "qgshttpheaders.h"

class QgsSettingsEntryString;
class QgsSettingsEntryInteger;
class QgsSettingsEntryVariantMap;

/**
 * \brief Represents connections to tiled scene data sources.
 *
 * \ingroup core
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneProviderConnection : public QgsAbstractProviderConnection
{

  public:

#ifndef SIP_RUN

    ///@cond PRIVATE
    static inline QgsSettingsTreeNamedListNode *sTreeConnectionTiledScene = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "tiled-scene" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsProvider;
    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryString *settingsAuthcfg;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryVariantMap *settingsHeaders;

    ///@endcond PRIVATE
#endif

    /**
     * Constructor for QgsTiledSceneProviderConnection, using the stored settings with the specified connection \a name.
     */
    QgsTiledSceneProviderConnection( const QString &name );

    /**
     * Constructor for QgsTiledSceneProviderConnection, using the a specific connection \a uri, \a provider name and \a configuration.
     */
    QgsTiledSceneProviderConnection( const QString &uri, const QString &provider, const QVariantMap &configuration );

    virtual void store( const QString &name ) const override;
    virtual void remove( const QString &name ) const override;

    /**
     * Returns the data provider associated with the connection.
     */
    QString providerKey() const { return mProvider; }

    /**
    * \brief Represents decoded data of a tiled scene connection.
    *
    * \ingroup core
    * \note Not available in Python bindings.
    *
    * \since QGIS 3.34
    */
    struct Data
    {
      //! Provider key
      QString provider;

      //! Source URI
      QString url;

      //! Authentication configuration ID
      QString authCfg;

      //! HTTP Basic username
      QString username;

      //! HTTP Basic password
      QString password;

      //! HTTP headers
      QgsHttpHeaders httpHeaders;

    };

    /**
     * Returns connection \a data encoded as a string.
     *
     * \see encodedLayerUri()
     * \see decodedUri()
     */
    static QString encodedUri( const Data &data );

    /**
     * Returns a connection \a uri decoded to a data structure.
     *
     * \see encodedUri()
     * \see encodedLayerUri()
     */
    static Data decodedUri( const QString &uri );

    /**
     * Returns connection \a data encoded as a string containing a URI for a QgsTiledSceneLayer.
     *
     * \see encodedUri()
     * \see decodedUri()
     */
    static QString encodedLayerUri( const Data &data );

    /**
     * Returns a list of the stored connection names.
     */
    static QStringList connectionList();

    /**
     * Returns connection details for the stored connection with the specified \a name.
     */
    static Data connection( const QString &name );

    /**
     * Stores a new \a connection, under the specified connection \a name.
     */
    static void addConnection( const QString &name, const Data &connection );

    /**
     * Returns the name of the last used connection.
     *
     * \see setSelectedConnection()
     */
    static QString selectedConnection();

    /**
     * Stores the \a name of the last used connection.
     *
     * \see selectedConnection()
     */
    static void setSelectedConnection( const QString &name );

  private:

    QString mProvider;
};

#endif // QGSTILEDSCENECONNECTION_H
