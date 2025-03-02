/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_sampling_thread.h"
#include <qelapsedtimer.h>

class QwtSamplingThread::PrivateData
{
  public:
    QElapsedTimer timer;
    double msecsInterval;
};

//! Constructor
QwtSamplingThread::QwtSamplingThread( QObject* parent )
    : QThread( parent )
{
    m_data = new PrivateData;
    m_data->msecsInterval = 1e3; // 1 second
}

//! Destructor
QwtSamplingThread::~QwtSamplingThread()
{
    delete m_data;
}

/*!
   Change the interval (in ms), when sample() is called.
   The default interval is 1000.0 ( = 1s )

   \param msecs Interval
   \sa interval()
 */
void QwtSamplingThread::setInterval( double msecs )
{
    if ( msecs < 0.0 )
        msecs = 0.0;

    m_data->msecsInterval = msecs;
}

/*!
   \return Interval (in ms), between 2 calls of sample()
   \sa setInterval()
 */
double QwtSamplingThread::interval() const
{
    return m_data->msecsInterval;
}

/*!
   \return Time (in ms) since the thread was started
   \sa QThread::start(), run()
 */
double QwtSamplingThread::elapsed() const
{
    if ( m_data->timer.isValid() )
        return m_data->timer.nsecsElapsed() / 1e6;

    return 0.0;
}

/*!
   Terminate the collecting thread
   \sa QThread::start(), run()
 */
void QwtSamplingThread::stop()
{
    m_data->timer.invalidate();
}

/*!
   Loop collecting samples started from QThread::start()
   \sa stop()
 */
void QwtSamplingThread::run()
{
    m_data->timer.start();

    /*
        We should have all values in nsecs/qint64, but
        this would break existing code. TODO ...
        Anyway - for QThread::usleep we even need microseconds( usecs )
     */
    while ( m_data->timer.isValid() )
    {
        const qint64 timestamp = m_data->timer.nsecsElapsed();
        sample( timestamp / 1e9 ); // seconds

        if ( m_data->msecsInterval > 0.0 )
        {
            const double interval = m_data->msecsInterval * 1e3;
            const double elapsed = ( m_data->timer.nsecsElapsed() - timestamp ) / 1e3;

            const double usecs = interval - elapsed;

            if ( usecs > 0.0 )
                QThread::usleep( qRound( usecs ) );
        }
    }
}

#include "moc_qwt_sampling_thread.cpp"
