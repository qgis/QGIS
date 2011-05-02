/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_H
#define QWT_POLAR_H 1

#include "qwt_polar_global.h"

namespace QwtPolar
{
  /*!
    An enum, that identifies the type of a coordinate

    - Azimuth\n
    - Radius\n
   */
  enum Coordinate
  {
    Azimuth,
    Radius
  };

  /*!
    Indices used to identify an axis.

    - AxisAzimuth\n
    - AxisLeft\n
    - AxisRight\n
    - AxisTop\n
    - AxisBottom\n
    - AxesCount\n

    \sa Scale
   */
  enum Axis
  {
    AxisAzimuth,

    AxisLeft,
    AxisRight,
    AxisTop,
    AxisBottom,

    AxesCount
  };

  /*!
    Indices used to identify a scale.

    - ScaleAzimuth\n
    - ScaleRadius\n

    \sa Axis
   */
  enum Scale
  {
    ScaleAzimuth = Azimuth,
    ScaleRadius = Radius,

    ScaleCount
  };

};

#endif
