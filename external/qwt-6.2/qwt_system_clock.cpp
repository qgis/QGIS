/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_system_clock.h"
#include <qelapsedtimer.h>

//! \return true, if the elapsed timer is valid
bool QwtSystemClock::isNull() const
{
    return m_timer.isValid();
}

//! Start the elapsed timer
void QwtSystemClock::start()
{
    m_timer.start();
}

/*!
    Restart the elapsed timer
    \return elapsed time in multiples of milliseconds
 */
double QwtSystemClock::restart()
{
    const qint64 nsecs = m_timer.restart();
    return nsecs / 1e6;
}

//! \return elapsed time in multiples of milliseconds
double QwtSystemClock::elapsed() const
{
    const qint64 nsecs = m_timer.nsecsElapsed();
    return nsecs / 1e6;
}
