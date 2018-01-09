/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatetimestatisticalsummary.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDateTimeStatisticalSummary
{
%Docstring
 Calculator for summary statistics and aggregates for a list of datetimes.

Statistics are calculated by calling calculate() and passing a list of datetimes. The
individual statistics can then be retrieved using the associated methods. Note that not all statistics
are calculated by default. Statistics which require slower computations are only calculated by
specifying the statistic in the constructor or via setStatistics().

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsdatetimestatisticalsummary.h"
%End
  public:

    enum Statistic
    {
      Count,
      CountDistinct,
      CountMissing,
      Min,
      Max,
      Range,
      All,
    };
    typedef QFlags<QgsDateTimeStatisticalSummary::Statistic> Statistics;


    QgsDateTimeStatisticalSummary( QgsDateTimeStatisticalSummary::Statistics stats = All );
%Docstring
Constructor for QgsDateTimeStatisticalSummary

:param stats: flags for statistics to calculate
%End

    Statistics statistics() const;
%Docstring
Returns flags which specify which statistics will be calculated. Some statistics
are always calculated (e.g., count).

.. seealso:: :py:func:`setStatistics`
%End

    void setStatistics( Statistics stats );
%Docstring
Sets flags which specify which statistics will be calculated. Some statistics
are always calculated (e.g., count).

:param stats: flags for statistics to calculate

.. seealso:: :py:func:`statistics`
%End

    void reset();
%Docstring
Resets the calculated values
%End

    void calculate( const QVariantList &values );
%Docstring
Calculates summary statistics for a list of variants. Any non-datetime variants will be
ignored.

:param values: list of variants

.. seealso:: :py:func:`addValue`
%End

    void addValue( const QVariant &value );
%Docstring
Adds a single datetime to the statistics calculation. Calling this method
allows datetimes to be added to the calculation one at a time. For large
quantities of dates this may be more efficient then first adding all the
variants to a list and calling calculate().

:param value: datetime to add. Any non-datetime variants will be ignored.

.. note::

   call reset() before adding the first datetime using this method
to clear the results from any previous calculations

.. note::

   finalize() must be called after adding the final value and before
retrieving calculated statistics.

.. seealso:: :py:func:`calculate`

.. seealso:: :py:func:`finalize`
%End

    void finalize();
%Docstring
Must be called after adding all datetimes with addValue() and before retrieving
any calculated datetime statistics.

.. seealso:: :py:func:`addValue`
%End

    QVariant statistic( QgsDateTimeStatisticalSummary::Statistic stat ) const;
%Docstring
Returns the value of a specified statistic

:param stat: statistic to return

:return: calculated value of statistic
%End

    int count() const;
%Docstring
Returns the calculated count of values.
%End

    int countDistinct() const;
%Docstring
Returns the number of distinct datetime values.
%End

    QSet< QDateTime > distinctValues() const;
%Docstring
Returns the set of distinct datetime values.
%End

    int countMissing() const;
%Docstring
Returns the number of missing (null) datetime values.
%End

    QDateTime min() const;
%Docstring
Returns the minimum (earliest) non-null datetime value.
%End

    QDateTime max() const;
%Docstring
Returns the maximum (latest) non-null datetime value.
%End

    QgsInterval range() const;
%Docstring
Returns the range (interval between earliest and latest non-null datetime values).
%End

    static QString displayName( QgsDateTimeStatisticalSummary::Statistic statistic );
%Docstring
Returns the friendly display name for a statistic

:param statistic: statistic to return name for
%End

};

QFlags<QgsDateTimeStatisticalSummary::Statistic> operator|(QgsDateTimeStatisticalSummary::Statistic f1, QFlags<QgsDateTimeStatisticalSummary::Statistic> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdatetimestatisticalsummary.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
