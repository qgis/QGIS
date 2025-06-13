/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_ABSTRACT_SLIDER_H
#define QWT_ABSTRACT_SLIDER_H

#include "qwt_global.h"
#include "qwt_abstract_scale.h"

/*!
   \brief An abstract base class for slider widgets with a scale

   A slider widget displays a value according to a scale.
   The class is designed as a common super class for widgets like
   QwtKnob, QwtDial and QwtSlider.

   When the slider is nor readOnly() its value can be modified
   by keyboard, mouse and wheel inputs.

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.
   Only for linear scales the number of steps correspond with
   a fixed step size.
 */

class QWT_EXPORT QwtAbstractSlider : public QwtAbstractScale
{
    Q_OBJECT

    Q_PROPERTY( double value READ value WRITE setValue NOTIFY valueChanged USER true )

    Q_PROPERTY( uint totalSteps READ totalSteps WRITE setTotalSteps )
    Q_PROPERTY( uint singleSteps READ singleSteps WRITE setSingleSteps )
    Q_PROPERTY( uint pageSteps READ pageSteps WRITE setPageSteps )
    Q_PROPERTY( bool stepAlignment READ stepAlignment WRITE setStepAlignment )

    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool tracking READ isTracking WRITE setTracking )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )

    Q_PROPERTY( bool invertedControls READ invertedControls WRITE setInvertedControls )

  public:
    explicit QwtAbstractSlider( QWidget* parent = NULL );
    virtual ~QwtAbstractSlider();

    void setValid( bool );
    bool isValid() const;

    double value() const;

    void setWrapping( bool );
    bool wrapping() const;

    void setTotalSteps( uint );
    uint totalSteps() const;

    void setSingleSteps( uint );
    uint singleSteps() const;

    void setPageSteps( uint );
    uint pageSteps() const;

    void setStepAlignment( bool );
    bool stepAlignment() const;

    void setTracking( bool );
    bool isTracking() const;

    void setReadOnly( bool );
    bool isReadOnly() const;

    void setInvertedControls( bool );
    bool invertedControls() const;

  public Q_SLOTS:
    void setValue( double value );

  Q_SIGNALS:

    /*!
       \brief Notify a change of value.

       When tracking is enabled (default setting),
       this signal will be emitted every time the value changes.

       \param value New value

       \sa setTracking(), sliderMoved()
     */
    void valueChanged( double value );

    /*!
       This signal is emitted when the user presses the
       movable part of the slider.
     */
    void sliderPressed();

    /*!
       This signal is emitted when the user releases the
       movable part of the slider.
     */
    void sliderReleased();

    /*!
       This signal is emitted when the user moves the
       slider with the mouse.

       \param value New value

       \sa valueChanged()
     */
    void sliderMoved( double value );

  protected:
    virtual void mousePressEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void mouseReleaseEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void mouseMoveEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void keyPressEvent( QKeyEvent* ) QWT_OVERRIDE;
    virtual void wheelEvent( QWheelEvent* ) QWT_OVERRIDE;

    /*!
       \brief Determine what to do when the user presses a mouse button.

       \param pos Mouse position

       \retval True, when pos is a valid scroll position
       \sa scrolledTo()
     */
    virtual bool isScrollPosition( const QPoint& pos ) const = 0;

    /*!
       \brief Determine the value for a new position of the
             movable part of the slider

       \param pos Mouse position

       \return Value for the mouse position
       \sa isScrollPosition()
     */
    virtual double scrolledTo( const QPoint& pos ) const = 0;

    void incrementValue( int stepCount );

    virtual void scaleChange() QWT_OVERRIDE;

  protected:
    virtual void sliderChange();

    double incrementedValue(
        double value, int stepCount ) const;

  private:
    double alignedValue( double ) const;
    double boundedValue( double ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
