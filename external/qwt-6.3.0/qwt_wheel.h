/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_WHEEL_H
#define QWT_WHEEL_H

#include "qwt_global.h"
#include <qwidget.h>

/*!
   \brief The Wheel Widget

   The wheel widget can be used to change values over a very large range
   in very small steps. Using the setMass() member, it can be configured
   as a flying wheel.

   The default range of the wheel is [0.0, 100.0]

   \sa The radio example.
 */
class QWT_EXPORT QwtWheel : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( Qt::Orientation orientation
        READ orientation WRITE setOrientation )

    Q_PROPERTY( double value READ value WRITE setValue NOTIFY valueChanged USER true  )

    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )

    Q_PROPERTY( double singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( int pageStepCount READ pageStepCount WRITE setPageStepCount )
    Q_PROPERTY( bool stepAlignment READ stepAlignment WRITE setStepAlignment )

    Q_PROPERTY( bool tracking READ isTracking WRITE setTracking )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )
    Q_PROPERTY( bool inverted READ isInverted WRITE setInverted )

    Q_PROPERTY( double mass READ mass WRITE setMass )
    Q_PROPERTY( int updateInterval READ updateInterval WRITE setUpdateInterval )

    Q_PROPERTY( double totalAngle READ totalAngle WRITE setTotalAngle )
    Q_PROPERTY( double viewAngle READ viewAngle WRITE setViewAngle )
    Q_PROPERTY( int tickCount READ tickCount WRITE setTickCount )
    Q_PROPERTY( int wheelWidth READ wheelWidth WRITE setWheelWidth )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( int wheelBorderWidth READ wheelBorderWidth WRITE setWheelBorderWidth )

  public:
    explicit QwtWheel( QWidget* parent = NULL );
    virtual ~QwtWheel();

    double value() const;

    void setOrientation( Qt::Orientation );
    Qt::Orientation orientation() const;

    double totalAngle() const;
    double viewAngle() const;

    void setTickCount( int );
    int tickCount() const;

    void setWheelWidth( int );
    int wheelWidth() const;

    void setWheelBorderWidth( int );
    int wheelBorderWidth() const;

    void setBorderWidth( int );
    int borderWidth() const;

    void setInverted( bool );
    bool isInverted() const;

    void setWrapping( bool );
    bool wrapping() const;

    void setSingleStep( double );
    double singleStep() const;

    void setPageStepCount( int );
    int pageStepCount() const;

    void setStepAlignment( bool on );
    bool stepAlignment() const;

    void setRange( double min, double max );

    void setMinimum( double );
    double minimum() const;

    void setMaximum( double );
    double maximum() const;

    void setUpdateInterval( int );
    int updateInterval() const;

    void setTracking( bool );
    bool isTracking() const;

    double mass() const;

  public Q_SLOTS:
    void setValue( double );
    void setTotalAngle ( double );
    void setViewAngle( double );
    void setMass( double );

  Q_SIGNALS:

    /*!
       \brief Notify a change of value.

       When tracking is enabled this signal will be emitted every
       time the value changes.

       \param value new value
       \sa setTracking()
     */
    void valueChanged( double value );

    /*!
       This signal is emitted when the user presses the
       the wheel with the mouse
     */
    void wheelPressed();

    /*!
       This signal is emitted when the user releases the mouse
     */
    void wheelReleased();

    /*!
       This signal is emitted when the user moves the
       wheel with the mouse.

       \param value new value
     */
    void wheelMoved( double value );

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void mousePressEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void mouseReleaseEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void mouseMoveEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void keyPressEvent( QKeyEvent* ) QWT_OVERRIDE;
    virtual void wheelEvent( QWheelEvent* ) QWT_OVERRIDE;
    virtual void timerEvent( QTimerEvent* ) QWT_OVERRIDE;

    void stopFlying();

    QRect wheelRect() const;

    virtual QSize sizeHint() const QWT_OVERRIDE;
    virtual QSize minimumSizeHint() const QWT_OVERRIDE;

    virtual void drawTicks( QPainter*, const QRectF& );
    virtual void drawWheelBackground( QPainter*, const QRectF& );

    virtual double valueAt( const QPoint& ) const;

  private:
    double alignedValue( double ) const;
    double boundedValue( double ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
