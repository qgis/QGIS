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

#include "qgis_gui.h"
#include "qgslocatorcontext.h"
#include "qgslogger.h"
#include <QString>
#include <QVariant>
#include <QIcon>

class QgsFeedback;
class QgsLocatorFilter;

/**
 * \class QgsLocatorResult
 * \ingroup gui
 * Encapsulates properties of an individual matching result found by a QgsLocatorFilter.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLocatorResult
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
      , userData( userData )
    {}

    /**
     * Filter from which the result was obtained.
     */
    QgsLocatorFilter *filter = nullptr;

    /**
     * String displayed for result.
     */
    QString displayString;

    /**
     * Custom reference or other data set by the filter.
     */
    QVariant userData;

    /**
     * Icon for result.
     */
    QIcon icon;

};

/**
 * \class QgsLocatorFilter
 * \ingroup gui
 * Abstract base class for filters which collect locator results.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLocatorFilter : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLocatorFilter.
     */
    QgsLocatorFilter( QObject *parent = nullptr );

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

  signals:

    /**
     * Should be emitted by filters whenever they encounter a matching result
     * during within their fetchResults() implementation.
     */
    void resultFetched( const QgsLocatorResult &result );

};

Q_DECLARE_METATYPE( QgsLocatorResult )

#endif // QGSLOCATORFILTER_H


