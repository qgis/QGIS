/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_INTERVAL_SYMBOL_H
#define QWT_INTERVAL_SYMBOL_H

#include "qwt_global.h"
#include <qnamespace.h>

class QPainter;
class QPen;
class QBrush;
class QPointF;
class QColor;

/*!
   \brief A drawing primitive for displaying an interval like an error bar

   \sa QwtPlotIntervalCurve
 */
class QWT_EXPORT QwtIntervalSymbol
{
  public:
    //! Symbol style
    enum Style
    {
        //! No Style. The symbol cannot be drawn.
        NoSymbol = -1,

        /*!
           The symbol displays a line with caps at the beginning/end.
           The size of the caps depends on the symbol width().
         */
        Bar,

        /*!
           The symbol displays a plain rectangle using pen() and brush().
           The size of the rectangle depends on the translated interval and
           the width(),
         */
        Box,

        /*!
           Styles >= UserSymbol are reserved for derived
           classes of QwtIntervalSymbol that overload draw() with
           additional application specific symbol types.
         */
        UserSymbol = 1000
    };

  public:
    explicit QwtIntervalSymbol( Style = NoSymbol );
    QwtIntervalSymbol( const QwtIntervalSymbol& );

    virtual ~QwtIntervalSymbol();

    QwtIntervalSymbol& operator=( const QwtIntervalSymbol& );
    bool operator==( const QwtIntervalSymbol& ) const;
    bool operator!=( const QwtIntervalSymbol& ) const;

    void setWidth( int );
    int width() const;

    void setBrush( const QBrush& );
    const QBrush& brush() const;

    void setPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setPen( const QPen& );
    const QPen& pen() const;

    void setStyle( Style );
    Style style() const;

    virtual void draw( QPainter*, Qt::Orientation,
        const QPointF& from, const QPointF& to ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
