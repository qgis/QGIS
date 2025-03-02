/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_DATE_SCALE_ENGINE_H
#define QWT_DATE_SCALE_ENGINE_H

#include "qwt_global.h"
#include "qwt_date.h"
#include "qwt_scale_engine.h"

/*!
   \brief A scale engine for date/time values

   QwtDateScaleEngine builds scales from a time intervals.
   Together with QwtDateScaleDraw it can be used for
   axes according to date/time values.

   Years, months, weeks, days, hours and minutes are organized
   in steps with non constant intervals. QwtDateScaleEngine
   classifies intervals and aligns the boundaries and tick positions
   according to this classification.

   QwtDateScaleEngine supports representations depending
   on Qt::TimeSpec specifications. The valid range for scales
   is limited by the range of QDateTime, that differs
   between Qt4 and Qt5.

   Datetime values are expected as the number of milliseconds since
   1970-01-01T00:00:00 Universal Coordinated Time - also known
   as "The Epoch", that can be converted to QDateTime using
   QwtDate::toDateTime().

   \sa QwtDate, QwtPlot::setAxisScaleEngine(),
      QwtAbstractScale::setScaleEngine()
 */
class QWT_EXPORT QwtDateScaleEngine : public QwtLinearScaleEngine
{
  public:
    explicit QwtDateScaleEngine( Qt::TimeSpec = Qt::LocalTime );
    virtual ~QwtDateScaleEngine();

    void setTimeSpec( Qt::TimeSpec );
    Qt::TimeSpec timeSpec() const;

    void setUtcOffset( int seconds );
    int utcOffset() const;

    void setWeek0Type( QwtDate::Week0Type );
    QwtDate::Week0Type week0Type() const;

    void setMaxWeeks( int );
    int maxWeeks() const;

    virtual void autoScale(
        int maxNumSteps, double& x1, double& x2,
        double& stepSize ) const QWT_OVERRIDE;

    virtual QwtScaleDiv divideScale(
        double x1, double x2,
        int maxMajorSteps, int maxMinorSteps,
        double stepSize = 0.0 ) const QWT_OVERRIDE;

    virtual QwtDate::IntervalType intervalType(
        const QDateTime&, const QDateTime&, int maxSteps ) const;

    QDateTime toDateTime( double ) const;

  protected:
    virtual QDateTime alignDate( const QDateTime&, double stepSize,
        QwtDate::IntervalType, bool up ) const;

  private:
    QwtScaleDiv buildScaleDiv( const QDateTime&, const QDateTime&,
        int maxMajorSteps, int maxMinorSteps,
        QwtDate::IntervalType ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
