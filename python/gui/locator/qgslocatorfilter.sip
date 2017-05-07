/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocatorfilter.h                                   *
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

    QgsLocatorResult( QgsLocatorFilter *filter, const QString &displayString, const QVariant &userData = QVariant() );
%Docstring
 Constructor for QgsLocatorResult.
%End

    QgsLocatorFilter *filter;
%Docstring
 Filter from which the result was obtained.
%End

    QString displayString;
%Docstring
 String displayed for result.
%End

    QVariant userData;
%Docstring
 Custom reference or other data set by the filter.
%End

    QIcon icon;
%Docstring
 Icon for result.
%End

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

    QgsLocatorFilter( QObject *parent = 0 );
%Docstring
 Constructor for QgsLocatorFilter.
%End

    virtual void fetchResults( const QString &string, QgsFeedback *feedback ) = 0;
%Docstring
 Retrieves the filter results for a specified search ``string``.

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
 * src/gui/locator/qgslocatorfilter.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
