/***************************************************************************
    qgsgdalcloudconnection.h
    ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALCLOUDCONNECTION_H
#define QGSGDALCLOUDCONNECTION_H

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
 * \brief Represents connections to cloud providers via GDAL's VSI handlers.
 *
 * \ingroup core
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsGdalCloudProviderConnection : public QgsAbstractProviderConnection
{

  public:

#ifndef SIP_RUN

    ///@cond PRIVATE
    static inline QgsSettingsTreeNamedListNode *sTreeConnectionCloud = QgsSettingsTree::sTreeConnections->createNamedListNode( QStringLiteral( "cloud" ), Qgis::SettingsTreeNodeOption::NamedListSelectedItemSetting );

    static const QgsSettingsEntryString *settingsVsiHandler;
    static const QgsSettingsEntryString *settingsContainer;
    static const QgsSettingsEntryString *settingsPath;
    static const QgsSettingsEntryVariantMap *settingsCredentialOptions;

    ///@endcond PRIVATE
#endif

    /**
     * Constructor for QgsGdalCloudProviderConnection, using the stored settings with the specified connection \a name.
     */
    QgsGdalCloudProviderConnection( const QString &name );

    /**
     * Constructor for QgsGdalCloudProviderConnection, using the a specific connection details.
     */
    QgsGdalCloudProviderConnection( const QString &uri, const QVariantMap &configuration );

    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;

    struct DirectoryObject
    {
      //! Object name
      QString name;
      //! TRUE if the object represents a file
      bool isFile = false;
      //! TRUE if the object represents a directory
      bool isDir = false;
    };

    /**
     * Returns the contents of the bucket at the specified \a path.
     */
    QList< DirectoryObject > contents( const QString &path ) const;

    /**
    * \brief Represents decoded data of a GDAL cloud provider connection.
    *
    * \ingroup core
    * \note Not available in Python bindings.
    *
    * \since QGIS 3.40
    */
    struct Data
    {
      //! VSI handler
      QString vsiHandler;

      //! Container or bucket
      QString container;

      //! Path
      QString rootPath;

      //! Credential options
      QVariantMap credentialOptions;

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

#endif // QGSGDALCLOUDCONNECTION_H
