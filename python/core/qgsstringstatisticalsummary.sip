/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstringstatisticalsummary.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsStringStatisticalSummary
{
%Docstring
 Calculator for summary statistics and aggregates for a list of strings.

Statistics are calculated by calling calculate() and passing a list of strings. The
individual statistics can then be retrieved using the associated methods. Note that not all statistics
are calculated by default. Statistics which require slower computations are only calculated by
specifying the statistic in the constructor or via setStatistics().

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsstringstatisticalsummary.h"
%End
  public:

    enum Statistic
    {
      Count,
      CountDistinct,
      CountMissing,
      Min,
      Max,
      MinimumLength,
      MaximumLength,
      MeanLength,
      All,
    };
    typedef QFlags<QgsStringStatisticalSummary::Statistic> Statistics;


    QgsStringStatisticalSummary( QgsStringStatisticalSummary::Statistics stats = QgsStringStatisticalSummary::All );
%Docstring
Constructor for :py:class:`QgsStringStatistics`

:param stats: flags for statistics to calculate
%End

    Statistics statistics() const;
%Docstring
Returns flags which specify which statistics will be calculated. Some statistics
are always calculated (e.g., count).

.. seealso:: :py:func:`setStatistics`
%End

    void setStatistics( QgsStringStatisticalSummary::Statistics stats );
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

    void calculate( const QStringList &values );
%Docstring
Calculates summary statistics for an entire list of strings at once.

:param values: list of strings

.. seealso:: :py:func:`calculateFromVariants`

.. seealso:: :py:func:`addString`
%End

    void calculateFromVariants( const QVariantList &values );
%Docstring
Calculates summary statistics for an entire list of variants at once. Any
non-string variants will be ignored.

:param values: list of variants

.. seealso:: :py:func:`calculate`

.. seealso:: :py:func:`addValue`
%End

    void addString( const QString &string );
%Docstring
Adds a single string to the statistics calculation. Calling this method
allows strings to be added to the calculation one at a time. For large
quantities of strings this may be more efficient then first adding all the
strings to a list and calling calculate().

:param string: string to add

.. note::

   call reset() before adding the first string using this method
to clear the results from any previous calculations

.. note::

   finalize() must be called after adding the final string and before
retrieving calculated statistics.

.. seealso:: :py:func:`calculate`

.. seealso:: :py:func:`addValue`

.. seealso:: :py:func:`finalize`
%End

    void addValue( const QVariant &value );
%Docstring
Adds a single variant to the statistics calculation. Calling this method
allows variants to be added to the calculation one at a time. For large
quantities of variants this may be more efficient then first adding all the
variants to a list and calling calculateFromVariants().

:param value: variant to add

.. note::

   call reset() before adding the first string using this method
to clear the results from any previous calculations

.. note::

   finalize() must be called after adding the final value and before
retrieving calculated statistics.

.. seealso:: :py:func:`calculateFromVariants`

.. seealso:: :py:func:`finalize`
%End

    void finalize();
%Docstring
Must be called after adding all strings with addString() and before retrieving
any calculated string statistics.

.. seealso:: :py:func:`addString`
%End

    QVariant statistic( QgsStringStatisticalSummary::Statistic stat ) const;
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
Returns the number of distinct string values.

.. seealso:: :py:func:`distinctValues`
%End

    QSet< QString > distinctValues() const;
%Docstring
Returns the set of distinct string values.

.. seealso:: :py:func:`countDistinct`
%End

    int countMissing() const;
%Docstring
Returns the number of missing (null) string values.
%End

    QString min() const;
%Docstring
Returns the minimum (non-null) string value.
%End

    QString max() const;
%Docstring
Returns the maximum (non-null) string value.
%End

    int minLength() const;
%Docstring
Returns the minimum length of strings.
%End

    int maxLength() const;
%Docstring
Returns the maximum length of strings.
%End

    double meanLength() const;
%Docstring
Returns the mean length of strings.

.. versionadded:: 3.0
%End

    static QString displayName( QgsStringStatisticalSummary::Statistic statistic );
%Docstring
Returns the friendly display name for a statistic

:param statistic: statistic to return name for
%End

};

QFlags<QgsStringStatisticalSummary::Statistic> operator|(QgsStringStatisticalSummary::Statistic f1, QFlags<QgsStringStatisticalSummary::Statistic> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstringstatisticalsummary.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
