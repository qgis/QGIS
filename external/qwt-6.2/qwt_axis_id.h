/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_AXIS_ID_H
#define QWT_AXIS_ID_H

#include "qwt_global.h"
#include "qwt_axis.h"

/*!
    \brief Axis identifier

    An axis id is one of values of QwtAxis::Position.

    QwtAxisId is a placeholder for future releases ( -> multiaxes branch ),
    where it is possible to have more than one axis at each side of a plot.

    \sa QwtAxis
 */
typedef int QwtAxisId;

#endif
