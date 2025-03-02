/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_KNOB_H
#define QWT_KNOB_H

#include "qwt_global.h"
#include "qwt_abstract_slider.h"

class QwtRoundScaleDraw;

/*!
   \brief The Knob Widget

   The QwtKnob widget imitates look and behavior of a volume knob on a radio.
   It looks similar to QDial - not to QwtDial.

   The value range of a knob might be divided into several turns.

   The layout of the knob depends on the knobWidth().

   - width > 0
    The diameter of the knob is fixed and the knob is aligned
    according to the alignment() flags inside of the contentsRect().

   - width <= 0
    The knob is extended to the minimum of width/height of the contentsRect()
    and aligned in the other direction according to alignment().

   Setting a fixed knobWidth() is helpful to align several knobs with different
   scale labels.

   \image html knob.png
 */

class QWT_EXPORT QwtKnob : public QwtAbstractSlider
{
    Q_OBJECT

    Q_ENUMS ( KnobStyle MarkerStyle )

    Q_PROPERTY( KnobStyle knobStyle READ knobStyle WRITE setKnobStyle )
    Q_PROPERTY( int knobWidth READ knobWidth WRITE setKnobWidth )
    Q_PROPERTY( Qt::Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( double totalAngle READ totalAngle WRITE setTotalAngle )
    Q_PROPERTY( int numTurns READ numTurns WRITE setNumTurns )
    Q_PROPERTY( MarkerStyle markerStyle READ markerStyle WRITE setMarkerStyle )
    Q_PROPERTY( int markerSize READ markerSize WRITE setMarkerSize )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )

  public:
    /*!
       \brief Style of the knob surface

       Depending on the KnobStyle the surface of the knob is
       filled from the brushes of the widget palette().

       \sa setKnobStyle(), knobStyle()
     */
    enum KnobStyle
    {
        //! Fill the knob with a brush from QPalette::Button.
        Flat,

        //! Build a gradient from QPalette::Midlight and QPalette::Button
        Raised,

        /*!
           Build a gradient from QPalette::Midlight, QPalette::Button
           and QPalette::Midlight
         */
        Sunken,

        /*!
           Build a radial gradient from QPalette::Button
           like it is used for QDial in various Qt styles.
         */
        Styled
    };

    /*!
        \brief Marker type

        The marker indicates the current value on the knob
        The default setting is a Notch marker.

        \sa setMarkerStyle(), setMarkerSize()
     */
    enum MarkerStyle
    {
        //! Don't paint any marker
        NoMarker = -1,

        //! Paint a single tick in QPalette::ButtonText color
        Tick,

        //! Paint a triangle in QPalette::ButtonText color
        Triangle,

        //! Paint a circle in QPalette::ButtonText color
        Dot,

        /*!
           Draw a raised ellipse with a gradient build from
           QPalette::Light and QPalette::Mid
         */
        Nub,

        /*!
           Draw a sunken ellipse with a gradient build from
           QPalette::Light and QPalette::Mid
         */
        Notch
    };

    explicit QwtKnob( QWidget* parent = NULL );
    virtual ~QwtKnob();

    void setAlignment( Qt::Alignment );
    Qt::Alignment alignment() const;

    void setKnobWidth( int );
    int knobWidth() const;

    void setNumTurns( int );
    int numTurns() const;

    void setTotalAngle ( double angle );
    double totalAngle() const;

    void setKnobStyle( KnobStyle );
    KnobStyle knobStyle() const;

    void setBorderWidth( int );
    int borderWidth() const;

    void setMarkerStyle( MarkerStyle );
    MarkerStyle markerStyle() const;

    void setMarkerSize( int );
    int markerSize() const;

    virtual QSize sizeHint() const QWT_OVERRIDE;
    virtual QSize minimumSizeHint() const QWT_OVERRIDE;

    void setScaleDraw( QwtRoundScaleDraw* );

    const QwtRoundScaleDraw* scaleDraw() const;
    QwtRoundScaleDraw* scaleDraw();

    QRect knobRect() const;

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void changeEvent( QEvent* ) QWT_OVERRIDE;

    virtual void drawKnob( QPainter*, const QRectF& ) const;

    virtual void drawFocusIndicator( QPainter* ) const;

    virtual void drawMarker( QPainter*,
        const QRectF&, double angle ) const;

    virtual double scrolledTo( const QPoint& ) const QWT_OVERRIDE;
    virtual bool isScrollPosition( const QPoint& ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
