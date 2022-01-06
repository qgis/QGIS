/***************************************************************************
                            qgshistoryproviderregistry.h
                            --------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHISTORYPROVIDERREGISTRY_H
#define QGSHISTORYPROVIDERREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QObject>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QVector>

#include "qgssqliteutils.h"

class QgsAbstractHistoryProvider;

/**
 * Encapsulates a history entry.
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsHistoryEntry
{
  public:

    /**
     * Constructor for QgsHistoryEntry \a entry, with the specified \a providerId and \a timestamp.
     */
    QgsHistoryEntry( const QString &providerId, const QDateTime &timestamp, const QVariantMap &entry );

    /**
     * Constructor for QgsHistoryEntry \a entry.
     *
     * The entry timestamp will be automatically set to the current date/time.
     */
    QgsHistoryEntry( const QVariantMap &entry );

    //! Entry timestamp
    QDateTime timestamp;

    //! Associated history provider ID
    QString providerId;

    /**
     * Entry details.
     *
     * Entries details are stored as a free-form map. Interpretation of this map is the responsibility of the
     * associated history provider.
     */
    QVariantMap entry;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsHistoryEntry: %1 %2>" ).arg( sipCpp->providerId, sipCpp->timestamp.toString( Qt::ISODate ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

/**
 * The QgsHistoryProviderRegistry is a registry for objects which track user history (i.e. operations performed through the GUI).
 *
 * QgsHistoryProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::historyProviderRegistry().
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsHistoryProviderRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty history provider registry.
     *
     * QgsHistoryProviderRegistry is not usually directly created, but rather accessed through
     * QgsGui::historyProviderRegistry().
    */
    QgsHistoryProviderRegistry( QObject *parent = nullptr, bool useMemoryDatabase = false );

    ~QgsHistoryProviderRegistry() override;

    /**
     * Adds the default history providers to the registry.
     * \note Not available through Python bindings.
     */
    void addDefaultProviders() SIP_SKIP;

    /**
     * Adds a \a provider to the registry. Ownership of the provider is
     * transferred to the registry.
     *
     * Returns TRUE if the provider was successfully added.
     */
    bool addProvider( QgsAbstractHistoryProvider *provider SIP_TRANSFER );

    /**
     * Returns the provider with matching \a id, or NULLPTR if no matching
     * provider is registered.
     */
    QgsAbstractHistoryProvider *providerById( const QString &id );

    /**
     * Removes the provider with matching \a id.
     *
     * The provider will be deleted.
     *
     * Returns TRUE if the provider was successfully removed.
     */
    bool removeProvider( const QString &id );

    /**
     * Returns a list of the registered provider IDs.
     */
    QStringList providerIds() const;

    /**
     * Contains options for storing history entries.
     *
     * \ingroup gui
     * \since QGIS 3.24
     */
    class HistoryEntryOptions
    {
      public:

        /**
         * Constructor for HistoryEntryOptions.
         */
        HistoryEntryOptions() {}

        //! Target storage backends
        Qgis::HistoryProviderBackends storageBackends = Qgis::HistoryProviderBackend::LocalProfile;
    };

    /**
     * Adds an \a entry to the history logs.
     *
     * The entry will be tagged with the current date/time as the timestamp.
     *
     * The \a providerId specifies the history provider responsible for this entry.
     * Entry options are specified via the \a options argument.
     *
     * \param providerId associated QgsAbstractHistoryProvider::id()
     * \param entry entry to add
     * \param ok will be set to TRUE if entry was successfully added
     * \param options options
     *
     * \returns ID of newly added entry.
     */
    long long addEntry( const QString &providerId, const QVariantMap &entry, bool &ok SIP_OUT, QgsHistoryProviderRegistry::HistoryEntryOptions options = QgsHistoryProviderRegistry::HistoryEntryOptions() );

    /**
     * Adds an \a entry to the history logs.
     *
     * \param entry entry to add
     * \param ok will be set to TRUE if entry was successfully added
     * \param options options
     *
     * \returns ID of newly added entry.
     */
    long long addEntry( const QgsHistoryEntry &entry, bool &ok SIP_OUT, QgsHistoryProviderRegistry::HistoryEntryOptions options = QgsHistoryProviderRegistry::HistoryEntryOptions() );

    /**
     * Adds a list of \a entries to the history logs.
     */
    bool addEntries( const QList< QgsHistoryEntry > &entries, QgsHistoryProviderRegistry::HistoryEntryOptions options = QgsHistoryProviderRegistry::HistoryEntryOptions() );

    /**
     * Returns the entry with matching ID, from the specified \a backend.
     *
     * \param id ID of entry to find
     * \param ok will be set to TRUE if entry was found
     * \param backend associated backend
     *
     * \returns matching entry if found
     */
    QgsHistoryEntry entry( long long id, bool &ok, Qgis::HistoryProviderBackend backend = Qgis::HistoryProviderBackend::LocalProfile ) const;

    /**
     * Updates the existing entry with matching \a id.
     *
     * This method allows the content of an entry to be updated, e.g. to add additional properties
     * to the content. (Such as recording the results of after a long-running operation completes).
     */
    bool updateEntry( long long id, const QVariantMap &entry, Qgis::HistoryProviderBackend backend = Qgis::HistoryProviderBackend::LocalProfile );

    /**
     * Queries history entries which occurred between the specified \a start and \a end times.
     *
     * The optional \a providerId and \a backends arguments can be used to filter entries.
     */
    QList< QgsHistoryEntry > queryEntries( const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime(),
                                           const QString &providerId = QString(), Qgis::HistoryProviderBackends backends = Qgis::HistoryProviderBackend::LocalProfile ) const;

    /**
     * Returns the path to user's local history database.
     */
    static QString userHistoryDbPath();

    /**
     * Clears the history for the specified \a backend.
     */
    bool clearHistory( Qgis::HistoryProviderBackend backend );

  private:

    /**
     * Creates an on-disk history database.
     */
    bool createDatabase( const QString &filename, QString &error );

    //! Convenience function to open the DB
    bool openDatabase( const QString &filename, QString &error );

    /**
     * Creates tables structure for history database.
     */
    void createTables();

    /**
     * Convenience function that would run queries which don't generate return values
     *
     *  \param query query to run
     *  \returns success TRUE on success
     */
    bool runEmptyQuery( const QString &query );

    QMap< QString, QgsAbstractHistoryProvider * > mProviders;

    sqlite3_database_unique_ptr mLocalDB;

};

#endif //QGSHISTORYPROVIDERREGISTRY_H
