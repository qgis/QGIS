/***************************************************************************
                         qgslocator.h
                         ------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLOCATOR_H
#define QGSLOCATOR_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslocatorfilter.h"
#include "qgsfeedback.h"
#include "qgslocatorcontext.h"
#include "qgssettingsentryimpl.h"


/**
 * \class QgsLocator
 * \ingroup core
 * \brief Handles the management of QgsLocatorFilter objects and async collection of search results from them.
 *
 * QgsLocator acts as both a registry for QgsLocatorFilter objects and a means of firing up
 * asynchronous queries against these filter objects.
 *
 * Filters are first registered to the locator by calling registerFilter(). Registering filters
 * transfers their ownership to the locator object. Plugins which register filters to the locator
 * must take care to correctly call deregisterFilter() and deregister their filter upon plugin
 * unload to avoid crashes.
 *
 * In order to trigger a search across registered filters, the fetchResults() method is called.
 * This triggers threaded calls to QgsLocatorFilter::fetchResults() for all registered filters.
 * As individual filters find matching results, the foundResult() signal will be triggered
 * for each result. Callers should connect this signal to an appropriate slot designed
 * to collect and handle these results. Since foundResult() is triggered whenever a filter
 * encounters an individual result, it will usually be triggered many times for a single
 * call to fetchResults().
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLocator : public QObject
{
    Q_OBJECT

  public:

    //! List of core filters (i.e. not plugin filters)
    static const QList<QString> CORE_FILTERS;

    /**
     * Constructor for QgsLocator.
     */
    QgsLocator( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Destructor for QgsLocator. Destruction will block while any currently running query is terminated.
     */
    ~QgsLocator() override;

    /**
     * Registers a \a filter within the locator. Ownership of the filter is transferred to the
     * locator.
     * \warning Plugins which register filters to the locator must take care to correctly call
     * deregisterFilter() and deregister their filters upon plugin unload to avoid crashes.
     * \see deregisterFilter()
     */
    void registerFilter( QgsLocatorFilter *filter SIP_TRANSFER );

    /**
     * Deregisters a \a filter from the locator and deletes it. Calling this will block whilst
     * any currently running query is terminated.
     *
     * Plugins which register filters to the locator must take care to correctly call
     * deregisterFilter() to deregister their filters upon plugin unload to avoid crashes.
     *
     * \see registerFilter()
     */
    void deregisterFilter( QgsLocatorFilter *filter );

    /**
     * Returns the list of filters registered in the locator.
     * \param prefix If prefix is not empty, the list returned corresponds to the filter with the given active prefix
     * \see prefixedFilters()
     */
    QList< QgsLocatorFilter *> filters( const QString &prefix = QString() );

    /**
     * Returns a map of prefix to filter, for all registered filters
     * with valid prefixes.
     * \see filters()
     * \deprecated since QGIS 3.2 use filters() instead
     */
    Q_DECL_DEPRECATED QMap<QString, QgsLocatorFilter *> prefixedFilters() const;

    /**
     * Triggers the background fetching of filter results for a specified search \a string.
     * The \a context argument encapsulates the context relating to the search (such as a map
     * extent to prioritize).
     *
     * If specified, the \a feedback object must exist for the lifetime of this query.
     *
     * The foundResult() signal will be emitted for each individual result encountered
     * by the registered filters.
     */
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback = nullptr );

    /**
     * Cancels any current running query, and blocks until query is completely canceled by
     * all filters.
     * \see cancelWithoutBlocking()
     */
    void cancel();

    /**
     * Triggers cancellation of any current running query without blocking. The query may
     * take some time to cancel after calling this.
     * \see cancel()
     */
    void cancelWithoutBlocking();

    /**
     * Returns TRUE if a query is currently being executed by the locator.
     */
    bool isRunning() const;

    /**
     * Will call clearPreviousResults on all filters
     * \since QGIS 3.2
     */
    void clearPreviousResults();

    /**
     * Returns the list for auto completion
     * This list is updated when preparing the search
     * \since QGIS 3.16
     */
    QStringList completionList() const {return mAutocompletionList;}

#ifndef SIP_RUN
    //! Settings entry locator filter enabled
    static const inline QgsSettingsEntryBool settingsLocatorFilterEnabled = QgsSettingsEntryBool( QStringLiteral( "enabled_%1" ), QgsSettings::Prefix::GUI_LOCATORFILTERS, true, QObject::tr( "Locator filter enabled" ) );
    //! Settings entry locator filter default value
    static const inline QgsSettingsEntryBool settingsLocatorFilterDefault = QgsSettingsEntryBool( QStringLiteral( "default_%1" ), QgsSettings::Prefix::GUI_LOCATORFILTERS, false, QObject::tr( "Locator filter default value" ) );
    //! Settings entry locator filter prefix
    static const inline QgsSettingsEntryString settingsLocatorFilterPrefix = QgsSettingsEntryString( QStringLiteral( "prefix_%1" ), QgsSettings::Prefix::GUI_LOCATORFILTERS, QString(), QObject::tr( "Locator filter prefix" ) );
#endif

  signals:

    /**
     * Emitted whenever a filter encounters a matching \a result after the fetchResults() method
     * is called.
     */
    void foundResult( const QgsLocatorResult &result );

    /**
     * Emitted when locator has prepared the search (\see QgsLocatorFilter::prepare)
     * before the search is actually performed
     * \since QGIS 3.16
     */
    void searchPrepared();

    /**
     * Emitted when locator has finished a query, either as a result
     * of successful completion or early cancellation.
     */
    void finished();

  private slots:

    void filterSentResult( QgsLocatorResult result );

  private:

    QgsFeedback *mFeedback = nullptr;
    std::unique_ptr< QgsFeedback > mOwnedFeedback;

    QList< QgsLocatorFilter * > mFilters;
    QList< QThread * > mActiveThreads;

    QStringList mAutocompletionList;

    void cancelRunningQuery();

};

#endif // QGSLOCATOR_H


