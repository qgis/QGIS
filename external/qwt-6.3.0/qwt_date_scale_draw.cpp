/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_date_scale_draw.h"
#include "qwt_text.h"

class QwtDateScaleDraw::PrivateData
{
  public:
    explicit PrivateData( Qt::TimeSpec spec )
        : timeSpec( spec )
        , utcOffset( 0 )
        , week0Type( QwtDate::FirstThursday )
    {
        dateFormats[ QwtDate::Millisecond ] = "hh:mm:ss:zzz\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Second ] = "hh:mm:ss\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Minute ] = "hh:mm\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Hour ] = "hh:mm\nddd dd MMM yyyy";
        dateFormats[ QwtDate::Day ] = "ddd dd MMM yyyy";
        dateFormats[ QwtDate::Week ] = "Www yyyy";
        dateFormats[ QwtDate::Month ] = "MMM yyyy";
        dateFormats[ QwtDate::Year ] = "yyyy";
    }

    Qt::TimeSpec timeSpec;
    int utcOffset;
    QwtDate::Week0Type week0Type;
    QString dateFormats[ QwtDate::Year + 1 ];
};

/*!
   \brief Constructor

   The default setting is to display tick labels for the
   given time specification. The first week of a year is defined like
   for QwtDate::FirstThursday.

   \param timeSpec Time specification

   \sa setTimeSpec(), setWeek0Type()
 */
QwtDateScaleDraw::QwtDateScaleDraw( Qt::TimeSpec timeSpec )
{
    m_data = new PrivateData( timeSpec );
}

//! Destructor
QwtDateScaleDraw::~QwtDateScaleDraw()
{
    delete m_data;
}

/*!
   Set the time specification used for the tick labels

   \param timeSpec Time specification
   \sa timeSpec(), setUtcOffset(), toDateTime()
 */
void QwtDateScaleDraw::setTimeSpec( Qt::TimeSpec timeSpec )
{
    m_data->timeSpec = timeSpec;
}

/*!
   \return Time specification used for the tick labels
   \sa setTimeSpec(), utcOffset(), toDateTime()
 */
Qt::TimeSpec QwtDateScaleDraw::timeSpec() const
{
    return m_data->timeSpec;
}

/*!
   Set the offset in seconds from Coordinated Universal Time

   \param seconds Offset in seconds

   \note The offset has no effect beside for the time specification
        Qt::OffsetFromUTC.

   \sa QDate::utcOffset(), setTimeSpec(), toDateTime()
 */
void QwtDateScaleDraw::setUtcOffset( int seconds )
{
    m_data->utcOffset = seconds;
}

/*!
   \return Offset in seconds from Coordinated Universal Time
   \note The offset has no effect beside for the time specification
        Qt::OffsetFromUTC.

   \sa QDate::setUtcOffset(), setTimeSpec(), toDateTime()
 */
int QwtDateScaleDraw::utcOffset() const
{
    return m_data->utcOffset;
}

/*!
   Sets how to identify the first week of a year.

   \param week0Type Mode how to identify the first week of a year

   \sa week0Type().
   \note week0Type has no effect beside for intervals classified as
        QwtDate::Week.
 */
void QwtDateScaleDraw::setWeek0Type( QwtDate::Week0Type week0Type )
{
    m_data->week0Type = week0Type;
}

/*!
   \return Setting how to identify the first week of a year.
   \sa setWeek0Type()
 */
QwtDate::Week0Type QwtDateScaleDraw::week0Type() const
{
    return m_data->week0Type;
}

/*!
   Set the default format string for an datetime interval type

   \param intervalType Interval type
   \param format Default format string

   \sa dateFormat(), dateFormatOfDate(), QwtDate::toString()
 */
void QwtDateScaleDraw::setDateFormat(
    QwtDate::IntervalType intervalType, const QString& format )
{
    if ( intervalType >= QwtDate::Millisecond &&
        intervalType <= QwtDate::Year )
    {
        m_data->dateFormats[ intervalType ] = format;
    }
}

/*!
   \param intervalType Interval type
   \return Default format string for an datetime interval type
   \sa setDateFormat(), dateFormatOfDate()
 */
QString QwtDateScaleDraw::dateFormat(
    QwtDate::IntervalType intervalType ) const
{
    if ( intervalType >= QwtDate::Millisecond &&
        intervalType <= QwtDate::Year )
    {
        return m_data->dateFormats[ intervalType ];
    }

    return QString();
}

/*!
   Format string for the representation of a datetime

   dateFormatOfDate() is intended to be overloaded for
   situations, where formats are individual for specific
   datetime values.

   The default setting ignores dateTime and return
   the default format for the interval type.

   \param dateTime Datetime value
   \param intervalType Interval type
   \return Format string

   \sa setDateFormat(), QwtDate::toString()
 */
QString QwtDateScaleDraw::dateFormatOfDate( const QDateTime& dateTime,
    QwtDate::IntervalType intervalType ) const
{
    Q_UNUSED( dateTime )

    if ( intervalType >= QwtDate::Millisecond &&
        intervalType <= QwtDate::Year )
    {
        return m_data->dateFormats[ intervalType ];
    }

    return m_data->dateFormats[ QwtDate::Second ];
}

/*!
   \brief Convert a value into its representing label

   The value is converted to a datetime value using toDateTime()
   and converted to a plain text using QwtDate::toString().

   \param value Value
   \return Label string.

   \sa dateFormatOfDate()
 */
QwtText QwtDateScaleDraw::label( double value ) const
{
    const QDateTime dt = toDateTime( value );
    const QString fmt = dateFormatOfDate(
        dt, intervalType( scaleDiv() ) );

    return QwtDate::toString( dt, fmt, m_data->week0Type );
}

/*!
   Find the less detailed datetime unit, where no rounding
   errors happen.

   \param scaleDiv Scale division
   \return Interval type

   \sa dateFormatOfDate()
 */
QwtDate::IntervalType QwtDateScaleDraw::intervalType(
    const QwtScaleDiv& scaleDiv ) const
{
    int intvType = QwtDate::Year;

    bool alignedToWeeks = true;

    const QList< double > ticks = scaleDiv.ticks( QwtScaleDiv::MajorTick );
    for ( int i = 0; i < ticks.size(); i++ )
    {
        const QDateTime dt = toDateTime( ticks[i] );
        for ( int j = QwtDate::Second; j <= intvType; j++ )
        {
            const QDateTime dt0 = QwtDate::floor( dt,
                static_cast< QwtDate::IntervalType >( j ) );

            if ( dt0 != dt )
            {
                if ( j == QwtDate::Week )
                {
                    alignedToWeeks = false;
                }
                else
                {
                    intvType = j - 1;
                    break;
                }
            }
        }

        if ( intvType == QwtDate::Millisecond )
            break;
    }

    if ( intvType == QwtDate::Week && !alignedToWeeks )
        intvType = QwtDate::Day;

    return static_cast< QwtDate::IntervalType >( intvType );
}

/*!
   Translate a double value into a QDateTime object.

   \return QDateTime object initialized with timeSpec() and utcOffset().
   \sa timeSpec(), utcOffset(), QwtDate::toDateTime()
 */
QDateTime QwtDateScaleDraw::toDateTime( double value ) const
{
    QDateTime dt = QwtDate::toDateTime( value, m_data->timeSpec );
    if ( m_data->timeSpec == Qt::OffsetFromUTC )
    {
        dt = dt.addSecs( m_data->utcOffset );
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc( m_data->utcOffset );
#else
        dt.setUtcOffset( m_data->utcOffset );
#endif
    }

    return dt;
}
