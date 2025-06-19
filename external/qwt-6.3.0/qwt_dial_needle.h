/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_DIAL_NEEDLE_H
#define QWT_DIAL_NEEDLE_H

#include "qwt_global.h"
#include <qpalette.h>

class QPainter;

/*!
   \brief Base class for needles that can be used in a QwtDial.

   QwtDialNeedle is a pointer that indicates a value by pointing
   to a specific direction.

   \sa QwtDial, QwtCompass
 */

class QWT_EXPORT QwtDialNeedle
{
  public:
    QwtDialNeedle();
    virtual ~QwtDialNeedle();

    virtual void setPalette( const QPalette& );
    const QPalette& palette() const;

    virtual void draw( QPainter*, const QPointF& center,
        double length, double direction,
        QPalette::ColorGroup = QPalette::Active ) const;

  protected:
    /*!
       \brief Draw the needle

       The origin of the needle is at position (0.0, 0.0 )
       pointing in direction 0.0 ( = east ).

       The painter is already initialized with translation and
       rotation.

       \param painter Painter
       \param length Length of the needle
       \param colorGroup Color group, used for painting

       \sa setPalette(), palette()
     */
    virtual void drawNeedle( QPainter* painter,
        double length, QPalette::ColorGroup colorGroup ) const = 0;

    virtual void drawKnob( QPainter*, double width,
        const QBrush&, bool sunken ) const;

  private:
    Q_DISABLE_COPY(QwtDialNeedle)

    QPalette m_palette;
};

/*!
   \brief A needle for dial widgets

   The following colors are used:

   - QPalette::Mid\n
    Pointer
   - QPalette::Base\n
    Knob

   \sa QwtDial, QwtCompass
 */

class QWT_EXPORT QwtDialSimpleNeedle : public QwtDialNeedle
{
  public:
    //! Style of the needle
    enum Style
    {
        //! Arrow
        Arrow,

        //! A straight line from the center
        Ray
    };

    QwtDialSimpleNeedle( Style, bool hasKnob = true,
        const QColor& mid = Qt::gray, const QColor& base = Qt::darkGray );

    void setWidth( double width );
    double width() const;

  protected:
    virtual void drawNeedle( QPainter*, double length,
        QPalette::ColorGroup ) const QWT_OVERRIDE;

  private:
    Style m_style;
    bool m_hasKnob;
    double m_width;
};

/*!
   \brief A magnet needle for compass widgets

   A magnet needle points to two opposite directions indicating
   north and south.

   The following colors are used:
   - QPalette::Light\n
    Used for pointing south
   - QPalette::Dark\n
    Used for pointing north
   - QPalette::Base\n
    Knob (ThinStyle only)

   \sa QwtDial, QwtCompass
 */

class QWT_EXPORT QwtCompassMagnetNeedle : public QwtDialNeedle
{
  public:
    //! Style of the needle
    enum Style
    {
        //! A needle with a triangular shape
        TriangleStyle,

        //! A thin needle
        ThinStyle
    };

    QwtCompassMagnetNeedle( Style = TriangleStyle,
        const QColor& light = Qt::white, const QColor& dark = Qt::red );

  protected:
    virtual void drawNeedle( QPainter*,
        double length, QPalette::ColorGroup ) const QWT_OVERRIDE;

  private:
    Style m_style;
};

/*!
   \brief An indicator for the wind direction

   QwtCompassWindArrow shows the direction where the wind comes from.

   - QPalette::Light\n
    Used for Style1, or the light half of Style2
   - QPalette::Dark\n
    Used for the dark half of Style2

   \sa QwtDial, QwtCompass
 */

class QWT_EXPORT QwtCompassWindArrow : public QwtDialNeedle
{
  public:
    //! Style of the arrow
    enum Style
    {
        //! A needle pointing to the center
        Style1,

        //! A needle pointing to the center
        Style2
    };

    QwtCompassWindArrow( Style, const QColor& light = Qt::white,
        const QColor& dark = Qt::gray );

  protected:
    virtual void drawNeedle( QPainter*,
        double length, QPalette::ColorGroup ) const QWT_OVERRIDE;

  private:
    Style m_style;
};

#endif
