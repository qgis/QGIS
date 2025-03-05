/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_DATE_H
#define QWT_DATE_H

#include "qwt_global.h"
#include <qdatetime.h>

/*!
   \brief A collection of methods around date/time values

   Qt offers convenient classes for dealing with date/time values,
   but Qwt uses coordinate systems that are based on doubles.
   QwtDate offers methods to translate from QDateTime to double and v.v.

   A double is interpreted as the number of milliseconds since
   1970-01-01T00:00:00 Universal Coordinated Time - also known
   as "The Epoch".

   While the range of the Julian day in Qt4 is limited to [0, MAX_INT],
   Qt5 stores it as qint64 offering a huge range of valid dates.
   As the significance of a double is below this ( assuming a
   fraction of 52 bits ) the translation is not
   bijective with rounding errors for dates very far from Epoch.
   For a resolution of 1 ms those start to happen for dates above the
   year 144683.

   An axis for a date/time interval is expected to be aligned
   and divided in time/date units like seconds, minutes, ...
   QwtDate offers several algorithms that are needed to
   calculate these axes.

   \sa QwtDateScaleEngine, QwtDateScaleDraw, QDate, QTime
 */
class QWT_EXPORT QwtDate
{
  public:
    /*!
       How to identify the first week of year differs between
       countries.
     */
    enum Week0Type
    {
        /*!
           According to ISO 8601 the first week of a year is defined
           as "the week with the year's first Thursday in it".

           FirstThursday corresponds to the numbering that is
           implemented in QDate::weekNumber().
         */
        FirstThursday,

        /*!
            "The week with January 1.1 in it."

            In the U.S. this definition is more common than
            FirstThursday.
         */
        FirstDay
    };

    /*!
       Classification of an time interval

       Time intervals needs to be classified to decide how to
       align and divide it.
     */
    enum IntervalType
    {
        //! The interval is related to milliseconds
        Millisecond,

        //! The interval is related to seconds
        Second,

        //! The interval is related to minutes
        Minute,

        //! The interval is related to hours
        Hour,

        //! The interval is related to days
        Day,

        //! The interval is related to weeks
        Week,

        //! The interval is related to months
        Month,

        //! The interval is related to years
        Year
    };

    enum
    {
        //! The Julian day of "The Epoch"
        JulianDayForEpoch = 2440588
    };

    static QDate minDate();
    static QDate maxDate();

    static QDateTime toDateTime( double value,
        Qt::TimeSpec = Qt::UTC );

    static double toDouble( const QDateTime& );

    static QDateTime ceil( const QDateTime&, IntervalType );
    static QDateTime floor( const QDateTime&, IntervalType );

    static QDate dateOfWeek0( int year, Week0Type );
    static int weekNumber( const QDate&, Week0Type );

    static int utcOffset( const QDateTime& );

    static QString toString( const QDateTime&,
        const QString& format, Week0Type );
};

#endif
