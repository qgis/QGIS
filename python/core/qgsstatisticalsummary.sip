/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstatisticalsummary.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsStatisticalSummary
{
%Docstring
 Calculator for summary statistics for a list of doubles.

Statistics are calculated by calling calculate() and passing a list of doubles. The
individual statistics can then be retrieved using the associated methods. Note that not all statistics
are calculated by default. Statistics which require slower computations are only calculated by
specifying the statistic in the constructor or via setStatistics().

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsstatisticalsummary.h"
%End
  public:

    enum Statistic
    {
      Count,
      CountMissing,
      Sum,
      Mean,
      Median,
      StDev,
      StDevSample,
      Min,
      Max,
      Range,
      Minority,
      Majority,
      Variety,
      FirstQuartile,
      ThirdQuartile,
      InterQuartileRange,
      All
    };
    typedef QFlags<QgsStatisticalSummary::Statistic> Statistics;


    QgsStatisticalSummary( QgsStatisticalSummary::Statistics stats = QgsStatisticalSummary::All );
%Docstring
Constructor for QgsStatisticalSummary

:param stats: flags for statistics to calculate
%End

    virtual ~QgsStatisticalSummary();

    Statistics statistics() const;
%Docstring
Returns flags which specify which statistics will be calculated. Some statistics
are always calculated (e.g., sum, min and max).

.. seealso:: :py:func:`setStatistics`
%End

    void setStatistics( QgsStatisticalSummary::Statistics stats );
%Docstring
Sets flags which specify which statistics will be calculated. Some statistics
are always calculated (e.g., sum, min and max).

:param stats: flags for statistics to calculate

.. seealso:: :py:func:`statistics`
%End

    void reset();
%Docstring
Resets the calculated values
%End

    void calculate( const QList<double> &values );
%Docstring
Calculates summary statistics for a list of values

:param values: list of doubles
%End

    void addValue( double value );
%Docstring
Adds a single value to the statistics calculation. Calling this method
allows values to be added to the calculation one at a time. For large
quantities of values this may be more efficient then first adding all the
values to a list and calling calculate().

:param value: value to add

.. note::

   call reset() before adding the first value using this method
to clear the results from any previous calculations

.. note::

   finalize() must be called after adding the final value and before
retrieving calculated statistics.

.. seealso:: :py:func:`calculate`

.. seealso:: :py:func:`addVariant`

.. seealso:: :py:func:`finalize`

.. versionadded:: 2.16
%End

    void addVariant( const QVariant &value );
%Docstring
Adds a single value to the statistics calculation. Calling this method
allows values to be added to the calculation one at a time. For large
quantities of values this may be more efficient then first adding all the
values to a list and calling calculate().

:param value: variant containing to add. Non-numeric values are treated as null.

.. note::

   call reset() before adding the first value using this method
to clear the results from any previous calculations

.. note::

   finalize() must be called after adding the final value and before
retrieving calculated statistics.

.. seealso:: :py:func:`addValue`

.. seealso:: :py:func:`calculate`

.. seealso:: :py:func:`finalize`

.. versionadded:: 2.16
%End

    void finalize();
%Docstring
Must be called after adding all values with addValues() and before retrieving
any calculated statistics.

.. seealso:: :py:func:`addValue`

.. seealso:: :py:func:`addVariant`

.. versionadded:: 2.16
%End

    double statistic( QgsStatisticalSummary::Statistic stat ) const;
%Docstring
Returns the value of a specified statistic

:param stat: statistic to return

:return: calculated value of statistic. A NaN value may be returned for invalid
statistics.
%End

    int count() const;
%Docstring
Returns calculated count of values
%End

    int countMissing() const;
%Docstring
Returns the number of missing (null) values

.. versionadded:: 2.16
%End

    double sum() const;
%Docstring
Returns calculated sum of values
%End

    double mean() const;
%Docstring
Returns calculated mean of values. A NaN value may be returned if the mean cannot
be calculated.
%End

    double median() const;
%Docstring
Returns calculated median of values. This is only calculated if Statistic.Median has
been specified in the constructor or via setStatistics. A NaN value may be returned if the median cannot
be calculated.
%End

    double min() const;
%Docstring
Returns calculated minimum from values. A NaN value may be returned if the minimum cannot
be calculated.
%End

    double max() const;
%Docstring
Returns calculated maximum from values. A NaN value may be returned if the maximum cannot
be calculated.
%End

    double range() const;
%Docstring
Returns calculated range (difference between maximum and minimum values). A NaN value may be returned if the range cannot
be calculated.
%End

    double stDev() const;
%Docstring
Returns population standard deviation. This is only calculated if Statistic.StDev has
been specified in the constructor or via setStatistics. A NaN value may be returned if the standard deviation cannot
be calculated.

.. seealso:: :py:func:`sampleStDev`
%End

    double sampleStDev() const;
%Docstring
Returns sample standard deviation. This is only calculated if Statistic.StDev has
been specified in the constructor or via setStatistics. A NaN value may be returned if the standard deviation cannot
be calculated.

.. seealso:: :py:func:`stDev`
%End

    int variety() const;
%Docstring
Returns variety of values. The variety is the count of unique values from the list.
This is only calculated if Statistic.Variety has been specified in the constructor
or via setStatistics.
%End

    double minority() const;
%Docstring
Returns minority of values. The minority is the value with least occurrences in the list
This is only calculated if Statistic.Minority has been specified in the constructor
or via setStatistics. A NaN value may be returned if the minority cannot
be calculated.

.. seealso:: :py:func:`majority`
%End

    double majority() const;
%Docstring
Returns majority of values. The majority is the value with most occurrences in the list
This is only calculated if Statistic.Majority has been specified in the constructor
or via setStatistics. A NaN value may be returned if the majority cannot
be calculated.

.. seealso:: :py:func:`minority`
%End

    double firstQuartile() const;
%Docstring
Returns the first quartile of the values. The quartile is calculated using the
"Tukey's hinges" method. A NaN value may be returned if the first quartile cannot
be calculated.

.. seealso:: :py:func:`thirdQuartile`

.. seealso:: :py:func:`interQuartileRange`
%End

    double thirdQuartile() const;
%Docstring
Returns the third quartile of the values. The quartile is calculated using the
"Tukey's hinges" method. A NaN value may be returned if the third quartile cannot
be calculated.

.. seealso:: :py:func:`firstQuartile`

.. seealso:: :py:func:`interQuartileRange`
%End

    double interQuartileRange() const;
%Docstring
Returns the inter quartile range of the values. The quartiles are calculated using the
"Tukey's hinges" method. A NaN value may be returned if the IQR cannot
be calculated.

.. seealso:: :py:func:`firstQuartile`

.. seealso:: :py:func:`thirdQuartile`
%End

    static QString displayName( QgsStatisticalSummary::Statistic statistic );
%Docstring
Returns the friendly display name for a statistic

:param statistic: statistic to return name for
%End

};

QFlags<QgsStatisticalSummary::Statistic> operator|(QgsStatisticalSummary::Statistic f1, QFlags<QgsStatisticalSummary::Statistic> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstatisticalsummary.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
