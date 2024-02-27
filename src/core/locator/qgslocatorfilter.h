/***************************************************************************
                         qgslocatorfilter.h
                         ------------------
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

#ifndef QGSLOCATORFILTER_H
#define QGSLOCATORFILTER_H

#include <QIcon>
#include <QString>
#include <QVariant>

#include "qgis_core.h"
#include "qgslocatorcontext.h"
#include "qgslogger.h"

class QgsFeedback;
class QgsLocatorFilter;

/**
 * \class QgsLocatorResult
 * \ingroup core
 * \brief Encapsulates properties of an individual matching result found by a QgsLocatorFilter.
 */
class CORE_EXPORT QgsLocatorResult
{
  public:

    /**
     * Constructor for QgsLocatorResult.
     */
    QgsLocatorResult() = default;

    /**
     * Constructor for QgsLocatorResult.
     */
    QgsLocatorResult( QgsLocatorFilter *filter, const QString &displayString, const QVariant &userData = QVariant() )
      : filter( filter )
      , displayString( displayString )
      , mUserData( userData )
    {}

    /**
     * Returns the ``userData``.
     *
     * \since QGIS 3.18
     */
    QVariant userData() const SIP_PYNAME( _userData );

    /**
     * Set \a userData for the locator result
     *
     * \since QGIS 3.34
     */
    void setUserData( const QVariant &userData );

    /**
     * Filter from which the result was obtained. This is automatically set.
     */
    QgsLocatorFilter *filter = nullptr;

    /**
     * String displayed for result.
     */
    QString displayString;

    /**
     * Descriptive text for result.
     */
    QString description;

    /**
     * Icon for result.
     */
    QIcon icon;

    /**
     * Match score, from 0 - 1, where 1 represents a perfect match.
     */
    double score = 0.5;

    /**
      * Group the results by categories
      * If left as empty string, this means that results are all shown without being grouped.
      * If a group is given, the results will be grouped by \a group under a header.
      * \note This should be translated.
      * \since QGIS 3.2
      */
    QString group;

    /**
     * The ResultAction stores basic information for additional
     * actions to be used in a locator widget for the result.
     * They could be used in a context menu for instance.
     * \since QGIS 3.6
     */
    struct CORE_EXPORT ResultAction
    {
      public:
        //! Constructor for ResultAction
        ResultAction() = default;

        /**
         * Constructor for ResultAction
         * The \a id used to recognized the action when the result is triggered.
         * It should be 0 or greater as otherwise, the result will be triggered
         * normally.
         */
        ResultAction( int id, QString text, QString iconPath = QString() )
          : id( id )
          , text( text )
          , iconPath( iconPath )
        {}
        int id = -1;
        QString text;
        QString iconPath;
    };

    /**
      * Additional actions to be used in a locator widget
      * for the given result. They could be displayed in
      * a context menu.
      * \since QGIS 3.6
      */
    QList<QgsLocatorResult::ResultAction> actions;

  private:

    /**
     * Custom reference or other data set by the filter.
     */
    QVariant mUserData;


};

Q_DECLARE_METATYPE( QgsLocatorResult::ResultAction )


/**
 * \class QgsLocatorFilter
 * \ingroup core
 * \brief Abstract base class for filters which collect locator results.
 *
 * \note If the configuration of the filter is changed outside of the main application settings,
 * one needs to invalidate current results of the locator widget: \see QgisInterface::invalidateLocatorResults
 */
class CORE_EXPORT QgsLocatorFilter : public QObject
{
    Q_OBJECT

  public:

    //! Filter priority. Controls the order of results in the locator.
    enum Priority
    {
      Highest, //!< Highest priority
      High, //!< High priority
      Medium, //!< Medium priority
      Low, //!< Low priority
      Lowest //!< Lowest priority
    };
    Q_ENUM( Priority )

    //! Flags for locator behavior.
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagFast = 1 << 1, //!< Filter finds results quickly and can be safely run in the main thread
    };
    Q_DECLARE_FLAGS( Flags, Flag )
    Q_FLAG( Flags )

    /**
     * Constructor for QgsLocatorFilter.
     */
    QgsLocatorFilter( QObject *parent = nullptr );

    /**
     * Creates a clone of the filter. New requests are always executed in a
     * clone of the original filter.
     */
    virtual QgsLocatorFilter *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the unique name for the filter. This should be an untranslated string identifying the filter.
     * \see displayName()
     */
    virtual QString name() const = 0;

    /**
     * Returns a translated, user-friendly name for the filter.
     * \see name()
     */
    virtual QString displayName() const = 0;

    /**
     * Returns a translated, description for the filter.
     * \since QGIS 3.20
     */
    virtual QString description() const { return QString(); }

    /**
     * Returns flags which specify the filter's behavior.
     */
    virtual QgsLocatorFilter::Flags flags() const;

    /**
     * Returns the priority for the filter, which controls how results are
     * ordered in the locator.
     */
    virtual Priority priority() const { return Medium; }

    /**
     * Returns the search prefix character(s) for this filter. Prefix a search
     * with these characters will restrict the locator search to only include
     * results from this filter.
     * \note Plugins are not permitted to utilize prefixes with < 3 characters,
     * as these are reserved for core QGIS functions. If a plugin registers
     * a filter with a prefix shorter than 3 characters then the prefix will
     * be ignored.
     * \note Prefixes might be overridden by user preferences.
     * \see activePrefix()
     */
    virtual QString prefix() const { return QString(); }

    /**
     * Prepares the filter instance for an upcoming search for the specified \a string. This method is always called
     * from the main thread, and individual filter subclasses should perform whatever
     * tasks are required in order to allow a subsequent search to safely execute
     * on a background thread.
     * The method returns an autocompletion list
     */
    virtual QStringList prepare( const QString &string, const QgsLocatorContext &context ) { Q_UNUSED( string ) Q_UNUSED( context ); return QStringList();}

    /**
     * Retrieves the filter results for a specified search \a string. The \a context
     * argument encapsulates the context relating to the search (such as a map
     * extent to prioritize).
     *
     * Implementations of fetchResults() should emit the resultFetched()
     * signal whenever they encounter a matching result.
     *
     * Subclasses should periodically check the \a feedback object to determine
     * whether the query has been canceled. If so, the subclass should return
     * from this method as soon as possible.
     *
     * This will be called from a background thread unless flags() returns the
     * QgsLocatorFilter::FlagFast flag.
     */
    virtual void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) = 0;

    /**
     * Triggers a filter \a result from this filter. This is called when
     * one of the results obtained by a call to fetchResults() is triggered
     * by a user. The filter subclass must implement logic here
     * to perform the desired operation for the search result.
     * E.g. a file search filter would open file associated with the triggered
     * result.
     */
    virtual void triggerResult( const QgsLocatorResult &result ) = 0;

    /**
     * Triggers a filter \a result from this filter for an entry in the context menu.
     * The entry is identified by its \a actionId as specified in the result of this filter.
     * \see triggerResult()
     * \since QGIS 3.6
     */
    virtual void triggerResultFromAction( const QgsLocatorResult &result, const int actionId );

    /**
     * This method will be called on main thread on the original filter (not a clone)
     * before fetching results or before triggering a result to clear any change made
     * by a former call to triggerResult.
     * For instance, this can be used to remove any on-canvas rubber bands which have been created
     * when a previous search result was triggered.
     * \since QGIS 3.2
     */
    virtual void clearPreviousResults() {}

    /**
     * Returns TRUE if the filter should be used when no prefix
     * is entered.
     * \see setUseWithoutPrefix()
     */
    bool useWithoutPrefix() const;

    /**
     * Sets whether the filter should be used when no prefix
     * is entered.
     * \see useWithoutPrefix()
     */
    void setUseWithoutPrefix( bool useWithoutPrefix );

    /**
     * Returns the prefix in use in the locator
     * is entered.
     * \see setActivePrefix()
     * \since QGIS 3.2
     */
    QString activePrefix() const;

    /**
     * Sets the prefix as being used by the locator
     * \see activePrefix()
     * \note If activePrefix is empty, no prefix is used. If activePrefix is NULL, the default prefix is used.
     * \since QGIS 3.2
     */
    void setActivePrefix( const QString &activePrefix ) SIP_SKIP;

    /**
     * Tests a \a candidate string to see if it should be considered a match for
     * a specified \a search string.
     * Filter subclasses should use this method when comparing strings instead
     * of directly using QString::contains() or Python 'in' checks.
     */
    static bool stringMatches( const QString &candidate, const QString &search );

    /**
     * Tests a \a candidate string to see how likely it is a match for
     * a specified \a search string.
     * \since QGIS 3.14
     */
    static double fuzzyScore( const QString &candidate, const QString &search );

    /**
     * Returns TRUE if the filter is enabled.
     * \see setEnabled()
     */
    bool enabled() const;

    /**
     * Sets whether the filter is \a enabled.
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Should return TRUE if the filter has a configuration widget.
     * \see openConfigWidget()
     */
    virtual bool hasConfigWidget() const;

    /**
     * Opens the configuration widget for the filter (if it has one), with the specified \a parent widget.
     * The base class implementation does nothing. Subclasses can override this to show their own
     * custom configuration widget.
     * \note hasConfigWidget() must return TRUE to indicate that the filter supports configuration.
     */
    virtual void openConfigWidget( QWidget *parent = nullptr );

    /**
     * Logs a \a message to the log panel
     * \warning in Python, do not use print() method as it might result in crashes
     *          since fetching results does not happen in the main thread.
     * \since QGIS 3.2
     */
    void logMessage( const QString &message, Qgis::MessageLevel level = Qgis::MessageLevel::Info );

    /**
     * Returns the delay (in milliseconds) for the filter to wait prior to fetching results.
     * \see setFetchResultsDelay()
     * \since QGIS 3.18
     */
    int fetchResultsDelay() const { return mFetchResultsDelay; }

    /**
     * Sets a \a delay (in milliseconds) for the filter to wait prior to fetching results.
     * \see fetchResultsDelay()
     * \note If the locator filter has a FastFlag, this value is ignored.
     * \since QGIS 3.18
     */
    void setFetchResultsDelay( int delay ) { mFetchResultsDelay = delay; }

  signals:

    /**
     * Emitted when the filter finishes fetching results.
     */
    void finished();

    /**
     * Should be emitted by filters whenever they encounter a matching result
     * during within their fetchResults() implementation.
     */
    void resultFetched( const QgsLocatorResult &result );

  private:

    bool mEnabled = true;
    bool mUseWithoutPrefix = true;
    QString mActivePrefifx = QString();
    int mFetchResultsDelay = 0;

};

Q_DECLARE_METATYPE( QgsLocatorResult )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLocatorFilter::Flags )


#endif // QGSLOCATORFILTER_H
