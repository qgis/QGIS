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
#define QGSSENSORTHINGSCONNECTION_H

#define SIP_NO_FILE

#include <QStringList>

#include "qgshttpheaders.h"
#include "qgis_core.h"
#include "qgssettingstree.h"
#include "qgsabstractproviderconnection.h"

class QgsSettingsEntryBool;
class QgsSettingsEntryDouble;
class QgsSettingsEntryInteger;
class QgsSettingsEntryString;
class QgsSettingsEntryVariantMap;
template<class T> class QgsSettingsEntryEnumFlag;

/**
 * \brief Represents connections to SensorThings data sources.
 *
 * \ingroup core
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSensorThingsProviderConnection : public QgsAbstractProviderConnection
{

  public:

    ///@cond PRIVATE
    static inline QgsSettingsTreeNamedListNode *sTreeSensorThingsConnections = QgsSettingsTree::sTreeConnections->createNamedListNode(
          QStringLiteral( "sensorthings" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsUrl;
    static const QgsSettingsEntryVariantMap *settingsHeaders;
    static const QgsSettingsEntryString *settingsUsername;
    static const QgsSettingsEntryString *settingsPassword;
    static const QgsSettingsEntryString *settingsAuthcfg;
    ///@endcond PRIVATE

    /**
     * Constructor for QgsSensorThingsProviderConnection, using the stored settings with the specified connection \a name.
     */
    QgsSensorThingsProviderConnection( const QString &name );

    /**
     * Constructor for QgsSensorThingsProviderConnection, using the a specific connection \a uri and \a configuration.
     */
    QgsSensorThingsProviderConnection( const QString &uri, const QVariantMap &configuration );

    void store( const QString &name ) const final;
    void remove( const QString &name ) const final;

    /**
    * \brief Represents decoded data of a SensorThings connection.
    *
    * \ingroup core
    * \note Not available in Python bindings.
    *
    * \since QGIS 3.36
    */
    struct Data
    {
      //! Source URI
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
     * Returns connection \a data encoded as a string containing a URI for a SensorThings vector data provider.
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
};


#endif // QGSSENSORTHINGSCONNECTION_H
