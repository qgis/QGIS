/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_CLIPPER_H
#define QWT_CLIPPER_H

#include "qwt_global.h"

class QwtInterval;
class QPointF;
class QRect;
class QRectF;
class QPolygon;
class QPolygonF;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/*!
   \brief Some clipping algorithms
 */

namespace QwtClipper
{
    QWT_EXPORT void clipPolygon( const QRect&,
        QPolygon&, bool closePolygon = false );

    QWT_EXPORT void clipPolygon( const QRectF&,
        QPolygon&, bool closePolygon = false );

    QWT_EXPORT void clipPolygonF( const QRectF&,
        QPolygonF&, bool closePolygon = false );

    QWT_EXPORT QPolygon clippedPolygon( const QRect&,
        const QPolygon&, bool closePolygon = false );

    QWT_EXPORT QPolygon clippedPolygon( const QRectF&,
        const QPolygon&, bool closePolygon = false );

    QWT_EXPORT QPolygonF clippedPolygonF( const QRectF&,
        const QPolygonF&, bool closePolygon = false );

    QWT_EXPORT QVector< QwtInterval > clipCircle(
        const QRectF&, const QPointF&, double radius );
};

#endif
