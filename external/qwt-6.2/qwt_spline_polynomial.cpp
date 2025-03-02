/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_polynomial.h"

namespace
{
    static const struct RegisterQwtSplinePolynomial
    {
        inline RegisterQwtSplinePolynomial()
            { qRegisterMetaType< QwtSplinePolynomial >(); }

    } qwtRegisterQwtSplinePolynomial;
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<( QDebug debug, const QwtSplinePolynomial& polynomial )
{
    debug.nospace() << "Polynom(" << polynomial.c3 << ", "
                    << polynomial.c2 << ", " << polynomial.c1 << ")";
    return debug.space();
}

#endif

