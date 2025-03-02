/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SYSTEM_CLOCK_H
#define QWT_SYSTEM_CLOCK_H

#include "qwt_global.h"
#include <qelapsedtimer.h>

/*!
   \brief QwtSystemClock provides high resolution clock time functions.

   Precision and time intervals are multiples of milliseconds (ms).

   ( QwtSystemClock is deprecated as QElapsedTimer offers the same precision )
 */

class QWT_EXPORT QwtSystemClock
{
  public:
    bool isNull() const;

    void start();
    double restart();
    double elapsed() const;

  private:
    QElapsedTimer m_timer;
};

#endif
