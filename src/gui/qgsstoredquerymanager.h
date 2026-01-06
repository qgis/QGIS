/***************************************************************************
                             qgsstoredquerymanager.h
                             ----------------------------------
    Date                 : February 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTOREDQUERYMANAGER_H
#define QGSSTOREDQUERYMANAGER_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgssettingstree.h"

/**
 * \ingroup gui
 * \brief A manager for stored SQL queries.
 *
 * QgsStoredQueryManager is not usually directly created, instead
 * use the instance accessible through QgsGui::storedQueryManager().
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsStoredQueryManager : public QObject
{
    Q_OBJECT

  public:
#ifndef SIP_RUN
    ///@cond PRIVATE
    static inline QgsSettingsTreeNamedListNode *sTreeStoredQueries = QgsSettingsTree::sTreeDatabase->createNamedListNode( u"stored-queries"_s );
    static const QgsSettingsEntryString *settingQueryName;
    static const QgsSettingsEntryString *settingQueryDefinition;
    ///@endcond
#endif

    /**
     * Constructor for QgsStoredQueryManager, with the specified
     * \a parent object.
     */
    QgsStoredQueryManager( QObject *parent = nullptr );

    /**
     * Saves a query to the manager.
     *
     * If a query with the same \a name already exists it will be overwritten with the new definition.
     *
     * \param name user-set, unique name for the query.
     * \param query query definition to store
     * \param backend storage backend for query
     *
     * \see queryAdded()
     * \see queryChanged()
     */
    void storeQuery( const QString &name, const QString &query, Qgis::QueryStorageBackend backend = Qgis::QueryStorageBackend::LocalProfile );

    /**
     * Removes the stored query with matching \a name.
     *
     * \param name name of query to remove
     * \param backend storage backend for query
     *
     * \see queryRemoved()
     */
    void removeQuery( const QString &name, Qgis::QueryStorageBackend backend = Qgis::QueryStorageBackend::LocalProfile );

    /**
     * Returns a list of the names of all stored queries for the specified \a backend.
     */
    QStringList allQueryNames( Qgis::QueryStorageBackend backend = Qgis::QueryStorageBackend::LocalProfile ) const;

    /**
     * Returns the query definition with matching \a name, from the specified \a backend.
     */
    QString query( const QString &name, Qgis::QueryStorageBackend backend = Qgis::QueryStorageBackend::LocalProfile ) const;

    /**
     * \ingroup gui
     * \brief Contains details about a stored query.
     *
     * \since QGIS 3.44
     */
    class QueryDetails
    {
      public:
        /**
         * Name of the query.
         */
        QString name;

        /**
         * Query definition.
         */
        QString definition;

        /**
         * Storage backend.
         */
        Qgis::QueryStorageBackend backend = Qgis::QueryStorageBackend::LocalProfile;
    };

    /**
     * Returns details of all queries stored in the manager.
     *
     * Queries will be sorted by name.
     */
    QList< QgsStoredQueryManager::QueryDetails > allQueries() const;

  signals:

    /**
     * Emitted when a query is added to the manager.
     */
    void queryAdded( const QString &name, Qgis::QueryStorageBackend backend );

    /**
     * Emitted when an existing query is changed in the manager.
     */
    void queryChanged( const QString &name, Qgis::QueryStorageBackend backend );

    /**
     * Emitted when a query is removed from the manager.
     */
    void queryRemoved( const QString &name, Qgis::QueryStorageBackend backend );

  private:
    static QString getQueryHash( const QString &name );
};


#endif // QGSSTOREDQUERYMANAGER_H
