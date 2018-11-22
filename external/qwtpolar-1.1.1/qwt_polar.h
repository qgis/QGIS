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
    //! Unit of an angle
    enum AngleUnit
    {
        //! 0.0 -> 2_M_PI
        Radians,

        //! 0.0 -> 360.0
        Degrees,

        //! 0.0 - 400.0
        Gradians,

        //! 0.0 - 1.0
        Turns
    };

    //! An enum, that identifies the type of a coordinate
    enum Coordinate
    {
        //! Azimuth
        Azimuth,

        //! Radius
        Radius
    };

    /*!
      Indices used to identify an axis.
      \sa Scale
     */
    enum Axis
    {
        //! Azimuth axis
        AxisAzimuth,

        //! Left axis
        AxisLeft,

        //! Right axis
        AxisRight,

        //! Top axis
        AxisTop,

        //! Bottom axis
        AxisBottom,

        //! Number of available axis
        AxesCount
    };

    /*!
      Indices used to identify a scale.
      \sa Axis
     */
    enum Scale
    {
        //! Azimuth scale
        ScaleAzimuth = Azimuth,

        //! Radial scale
        ScaleRadius = Radius,

        //! Number of scales
        ScaleCount
    };
}

#endif
