/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsinterval.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsInterval
{
%Docstring
 A representation of the interval between two datetime values.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsinterval.h"
%End
  public:

    static const int YEARS;
    static const int MONTHS;
    static const int WEEKS;
    static const int DAY;
    static const int HOUR;
    static const int MINUTE;

    QgsInterval();
%Docstring
Default constructor for QgsInterval. Creates an invalid interval.
%End

    QgsInterval( double seconds );
%Docstring
Constructor for QgsInterval.

:param seconds: duration of interval in seconds
%End

    double years() const;
%Docstring
Returns the interval duration in years (based on an average year length)

.. seealso:: :py:func:`setYears`
%End

    void setYears( double years );
%Docstring
Sets the interval duration in years.

:param years: duration in years (based on average year length)

.. seealso:: :py:func:`years`
%End

    double months() const;
%Docstring
Returns the interval duration in months (based on a 30 day month).

.. seealso:: :py:func:`setMonths`
%End

    void setMonths( double months );
%Docstring
Sets the interval duration in months.

:param months: duration in months (based on a 30 day month)

.. seealso:: :py:func:`months`
%End

    double weeks() const;
%Docstring
Returns the interval duration in weeks.

.. seealso:: :py:func:`setWeeks`
%End

    void setWeeks( double weeks );
%Docstring
Sets the interval duration in weeks.

:param weeks: duration in weeks

.. seealso:: :py:func:`weeks`
%End

    double days() const;
%Docstring
Returns the interval duration in days.

.. seealso:: :py:func:`setDays`
%End

    void setDays( double days );
%Docstring
Sets the interval duration in days.

:param days: duration in days

.. seealso:: :py:func:`days`
%End

    double hours() const;
%Docstring
Returns the interval duration in hours.

.. seealso:: :py:func:`setHours`
%End

    void setHours( double hours );
%Docstring
Sets the interval duration in hours.

:param hours: duration in hours

.. seealso:: :py:func:`hours`
%End

    double minutes() const;
%Docstring
Returns the interval duration in minutes.

.. seealso:: :py:func:`setMinutes`
%End

    void setMinutes( double minutes );
%Docstring
Sets the interval duration in minutes.

:param minutes: duration in minutes

.. seealso:: :py:func:`minutes`
%End

    double seconds() const;
%Docstring
Returns the interval duration in seconds.

.. seealso:: :py:func:`setSeconds`
%End

    void setSeconds( double seconds );
%Docstring
Sets the interval duration in seconds.

:param seconds: duration in seconds

.. seealso:: :py:func:`seconds`
%End

    bool isValid() const;
%Docstring
Returns true if the interval is valid.

.. seealso:: :py:func:`setValid`
%End

    void setValid( bool valid );
%Docstring
Sets whether the interval is valid.

:param valid: set to true to set the interval as valid.

.. seealso:: :py:func:`isValid`
%End

    bool operator==( QgsInterval other ) const;

    static QgsInterval fromString( const QString &string );
%Docstring
Converts a string to an interval

:param string: string to parse

:return: interval, or invalid interval if string could not be parsed
%End

    operator QVariant() const;

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsinterval.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
