/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocator.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLocator : QObject
{
%Docstring
 Handles the management of QgsLocatorFilter objects and async collection of search results from them.

 QgsLocator acts as both a registry for QgsLocatorFilter objects and a means of firing up
 asynchronous queries against these filter objects.

 Filters are first registered to the locator by calling registerFilter(). Registering filters
 transfers their ownership to the locator object. Plugins which register filters to the locator
 must take care to correctly call deregisterFilter() and deregister their filter upon plugin
 unload to avoid crashes.

 In order to trigger a search across registered filters, the fetchResults() method is called.
 This triggers threaded calls to QgsLocatorFilter.fetchResults() for all registered filters.
 As individual filters find matching results, the foundResult() signal will be triggered
 for each result. Callers should connect this signal to an appropriate slot designed
 to collect and handle these results. Since foundResult() is triggered whenever a filter
 encounters an individual result, it will usually be triggered many times for a single
 call to fetchResults().

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocator.h"
%End
  public:

    QgsLocator( QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsLocator.
%End

    ~QgsLocator();
%Docstring
 Destructor for QgsLocator. Destruction will block while any currently running query is terminated.
%End

    void registerFilter( QgsLocatorFilter *filter /Transfer/ );
%Docstring
 Registers a ``filter`` within the locator. Ownership of the filter is transferred to the
 locator.
 \warning Plugins which register filters to the locator must take care to correctly call
 deregisterFilter() and deregister their filters upon plugin unload to avoid crashes.
.. seealso:: deregisterFilter()
%End

    void deregisterFilter( QgsLocatorFilter *filter );
%Docstring
 Deregisters a ``filter`` from the locator and deletes it. Calling this will block whilst
 any currently running query is terminated.

 Plugins which register filters to the locator must take care to correctly call
 deregisterFilter() to deregister their filters upon plugin unload to avoid crashes.

.. seealso:: registerFilter()
%End

    QList< QgsLocatorFilter *> filters();
%Docstring
 Returns the list of filters registered in the locator.
.. seealso:: prefixedFilters()
 :rtype: list of QgsLocatorFilter
%End

    QMap< QString, QgsLocatorFilter *> prefixedFilters() const;
%Docstring
 Returns a map of prefix to filter, for all registered filters
 with valid prefixes.
.. seealso:: filters()
 :rtype: QMap< str, QgsLocatorFilter *>
%End

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback = 0 );
%Docstring
 Triggers the background fetching of filter results for a specified search ``string``.
 The ``context`` argument encapsulates the context relating to the search (such as a map
 extent to prioritize).

 If specified, the ``feedback`` object must exist for the lifetime of this query.

 The foundResult() signal will be emitted for each individual result encountered
 by the registered filters.
%End

    void cancel();
%Docstring
 Cancels any current running query, and blocks until query is completely canceled by
 all filters.
.. seealso:: cancelWithoutBlocking()
%End

    void cancelWithoutBlocking();
%Docstring
 Triggers cancellation of any current running query without blocking. The query may
 take some time to cancel after calling this.
.. seealso:: cancel()
%End

    bool isRunning() const;
%Docstring
 Returns true if a query is currently being executed by the locator.
 :rtype: bool
%End

  signals:

    void foundResult( const QgsLocatorResult &result );
%Docstring
 Emitted whenever a filter encounters a matching ``result`` after the fetchResults() method
 is called.
%End

    void finished();
%Docstring
 Emitted when locator has finished a query, either as a result
 of successful completion or early cancellation.
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocator.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
