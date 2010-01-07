/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_MARKER_H
#define QWT_POLAR_MARKER_H

#include <qstring.h>
#include "qwt_polar_global.h"
#include "qwt_polar_point.h"
#include "qwt_polar_item.h"

class QRect;
class QwtText;
class QwtSymbol;

/*!
  \brief A class for drawing markers

  A marker can be a a symbol, a label or a combination of them, which can
  be drawn around a center point inside a bounding rectangle.

  The setSymbol() member assigns a symbol to the marker.
  The symbol is drawn at the specified point.

  With setLabel(), a label can be assigned to the marker.
  The setLabelAlignment() member specifies where the label is
  drawn. All the Align*-constants in Qt::AlignmentFlags (see Qt documentation)
  are valid. The alignment refers to the center point of
  the marker, which means, for example, that the label would be painted
  left above the center point if the alignment was set to AlignLeft|AlignTop.
*/

class QWT_POLAR_EXPORT QwtPolarMarker: public QwtPolarItem
{
public:
    explicit QwtPolarMarker();
    virtual ~QwtPolarMarker();

    virtual int rtti() const;

    void setPosition(const QwtPolarPoint &);
    QwtPolarPoint position() const;

    void setSymbol(const QwtSymbol &s);
    const QwtSymbol &symbol() const;

    void setLabel(const QwtText&);
    QwtText label() const;

#if QT_VERSION < 0x040000
    void setLabelAlignment(int align);
    int labelAlignment() const;
#else
    void setLabelAlignment(Qt::Alignment);
    Qt::Alignment labelAlignment() const;
#endif

    virtual void draw(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const;

    virtual QwtDoubleInterval boundingInterval(int scaleId) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
