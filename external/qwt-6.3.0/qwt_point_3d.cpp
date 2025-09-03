/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_point_3d.h"

#if QT_VERSION >= 0x050200

static QwtPoint3D qwtPointToPoint3D( const QPointF& point )
{
    return QwtPoint3D( point );
}

#endif

namespace
{
    static const struct RegisterQwtPoint3D
    {
        inline RegisterQwtPoint3D()
        {
            qRegisterMetaType< QwtPoint3D >();

#if QT_VERSION >= 0x050200
            QMetaType::registerConverter< QPointF, QwtPoint3D >( qwtPointToPoint3D );
#endif
        }
    } qwtRegisterQwtPoint3D;
}

#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<( QDebug debug, const QwtPoint3D& point )
{
    debug.nospace() << "QwtPoint3D(" << point.x()
                    << "," << point.y() << "," << point.z() << ")";
    return debug.space();
}

#endif

