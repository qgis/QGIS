/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_AXIS_H
#define QWT_AXIS_H

#include "qwt_global.h"

/*!
   Enums and methods for axes
 */
namespace QwtAxis
{
    //! \brief Axis position
    enum Position
    {
        //! Y axis left of the canvas
        YLeft,

        //! Y axis right of the canvas
        YRight,

        //! X axis below the canvas
        XBottom,

        //! X axis above the canvas
        XTop
    };

    //! \brief Number of axis positions
    enum { AxisPositions = XTop + 1 };

    bool isValid( int axisPos );
    bool isYAxis( int axisPos );
    bool isXAxis( int axisPos );
}

//! \return true, when axisPos is in the valid range [ YLeft, XTop ]
inline bool QwtAxis::isValid( int axisPos )
{
    return ( axisPos >= 0 && axisPos < AxisPositions );
}

//! \return true, when axisPos is XBottom or XTop
inline bool QwtAxis::isXAxis( int axisPos )
{
    return ( axisPos == XBottom ) || ( axisPos == XTop );
}

//! \return true, when axisPos is YLeft or YRight
inline bool QwtAxis::isYAxis( int axisPos )
{
    return ( axisPos == YLeft ) || ( axisPos == YRight );
}

#endif
