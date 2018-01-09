/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatorfilter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLocatorResult
{
%Docstring
Encapsulates properties of an individual matching result found by a QgsLocatorFilter.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatorfilter.h"
%End
  public:

    QgsLocatorResult();
%Docstring
Constructor for QgsLocatorResult.
%End

    QgsLocatorResult( QgsLocatorFilter *filter, const QString &displayString, const QVariant &userData = QVariant() );
%Docstring
Constructor for QgsLocatorResult.
%End

    QgsLocatorFilter *filter;

    QString displayString;

    QString description;

    QVariant userData;

    QIcon icon;

    double score;

};

class QgsLocatorFilter : QObject
{
%Docstring
Abstract base class for filters which collect locator results.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatorfilter.h"
%End
  public:

    enum Priority
    {
      Highest,
      High,
      Medium,
      Low,
      Lowest
    };

    QgsLocatorFilter( QObject *parent = 0 );
%Docstring
Constructor for QgsLocatorFilter.
%End

    virtual QString name() const = 0;
%Docstring
Returns the unique name for the filter. This should be an untranslated string identifying the filter.

.. seealso:: :py:func:`displayName`
%End

    virtual QString displayName() const = 0;
%Docstring
Returns a translated, user-friendly name for the filter.

.. seealso:: :py:func:`name`
%End

    virtual Priority priority() const;
%Docstring
Returns the priority for the filter, which controls how results are
ordered in the locator.
%End

    virtual QString prefix() const;
%Docstring
Returns the search prefix character(s) for this filter. Prefix a search
with these characters will restrict the locator search to only include
results from this filter.

.. note::

   Plugins are not permitted to utilize prefixes with < 3 characters,
as these are reserved for core QGIS functions. If a plugin registers
a filter with a prefix shorter than 3 characters then the prefix will
be ignored.
%End

    virtual void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) = 0;
%Docstring
Retrieves the filter results for a specified search ``string``. The ``context``
argument encapsulates the context relating to the search (such as a map
extent to prioritize).

Implementations of fetchResults() should emit the resultFetched()
signal whenever they encounter a matching result.

Subclasses should periodically check the ``feedback`` object to determine
whether the query has been canceled. If so, the subclass should return
from this method as soon as possible.
%End

    virtual void triggerResult( const QgsLocatorResult &result ) = 0;
%Docstring
Triggers a filter ``result`` from this filter. This is called when
one of the results obtained by a call to fetchResults() is triggered
by a user. The filter subclass must implement logic here
to perform the desired operation for the search result.
E.g. a file search filter would open file associated with the triggered
result.
%End

    bool useWithoutPrefix() const;
%Docstring
Returns true if the filter should be used when no prefix
is entered.

.. seealso:: :py:func:`setUseWithoutPrefix`
%End

    void setUseWithoutPrefix( bool useWithoutPrefix );
%Docstring
Sets whether the filter should be used when no prefix
is entered.

.. seealso:: :py:func:`useWithoutPrefix`
%End

    static bool stringMatches( const QString &candidate, const QString &search );
%Docstring
Tests a ``candidate`` string to see if it should be considered a match for
a specified ``search`` string.
Filter subclasses should use this method when comparing strings instead
of directly using QString.contains() or Python 'in' checks.
%End

    bool enabled() const;
%Docstring
Returns true if the filter is enabled.

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether the filter is ``enabled``.

.. seealso:: :py:func:`enabled`
%End

    virtual bool hasConfigWidget() const;
%Docstring
Should return true if the filter has a configuration widget.

.. seealso:: :py:func:`createConfigWidget`
%End

    virtual void openConfigWidget( QWidget *parent = 0 );
%Docstring
Opens the configuration widget for the filter (if it has one), with the specified ``parent`` widget.
The base class implementation does nothing. Subclasses can override this to show their own
custom configuration widget.

.. note::

   hasConfigWidget() must return true to indicate that the filter supports configuration.
%End

  signals:

    void resultFetched( const QgsLocatorResult &result );
%Docstring
Should be emitted by filters whenever they encounter a matching result
during within their fetchResults() implementation.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/locator/qgslocatorfilter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
