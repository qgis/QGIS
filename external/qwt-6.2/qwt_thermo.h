/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_THERMO_H
#define QWT_THERMO_H

#include "qwt_global.h"
#include "qwt_abstract_scale.h"
#include "qwt_interval.h"

class QwtScaleDraw;
class QwtColorMap;

/*!
   \brief The Thermometer Widget

   QwtThermo is a widget which displays a value in an interval. It supports:
   - a horizontal or vertical layout;
   - a range;
   - a scale;
   - an alarm level.

   \image html sysinfo.png

   The fill colors might be calculated from an optional color map
   If no color map has been assigned QwtThermo uses the
   following colors/brushes from the widget palette:

   - QPalette::Base
    Background of the pipe
   - QPalette::ButtonText
    Fill brush below the alarm level
   - QPalette::Highlight
    Fill brush for the values above the alarm level
   - QPalette::WindowText
    For the axis of the scale
   - QPalette::Text
    For the labels of the scale
 */
class QWT_EXPORT QwtThermo : public QwtAbstractScale
{
    Q_OBJECT

    Q_ENUMS( ScalePosition )
    Q_ENUMS( OriginMode )

    Q_PROPERTY( Qt::Orientation orientation
        READ orientation WRITE setOrientation )
    Q_PROPERTY( ScalePosition scalePosition
        READ scalePosition WRITE setScalePosition )
    Q_PROPERTY( OriginMode originMode READ originMode WRITE setOriginMode )

    Q_PROPERTY( bool alarmEnabled READ alarmEnabled WRITE setAlarmEnabled )
    Q_PROPERTY( double alarmLevel READ alarmLevel WRITE setAlarmLevel )
    Q_PROPERTY( double origin READ origin WRITE setOrigin )
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( int pipeWidth READ pipeWidth WRITE setPipeWidth )
    Q_PROPERTY( double value READ value WRITE setValue USER true )

  public:

    /*!
       Position of the scale
       \sa setScalePosition(), setOrientation()
     */
    enum ScalePosition
    {
        //! The slider has no scale
        NoScale,

        //! The scale is right of a vertical or below of a horizontal slider
        LeadingScale,

        //! The scale is left of a vertical or above of a horizontal slider
        TrailingScale
    };

    /*!
       Origin mode. This property specifies where the beginning of the liquid
       is placed.

       \sa setOriginMode(), setOrigin()
     */
    enum OriginMode
    {
        //! The origin is the minimum of the scale
        OriginMinimum,

        //! The origin is the maximum of the scale
        OriginMaximum,

        //! The origin is specified using the origin() property
        OriginCustom
    };

    explicit QwtThermo( QWidget* parent = NULL );
    virtual ~QwtThermo();

    void setOrientation( Qt::Orientation );
    Qt::Orientation orientation() const;

    void setScalePosition( ScalePosition );
    ScalePosition scalePosition() const;

    void setSpacing( int );
    int spacing() const;

    void setBorderWidth( int );
    int borderWidth() const;

    void setOriginMode( OriginMode );
    OriginMode originMode() const;

    void setOrigin( double );
    double origin() const;

    void setFillBrush( const QBrush& );
    QBrush fillBrush() const;

    void setAlarmBrush( const QBrush& );
    QBrush alarmBrush() const;

    void setAlarmLevel( double );
    double alarmLevel() const;

    void setAlarmEnabled( bool );
    bool alarmEnabled() const;

    void setColorMap( QwtColorMap* );
    QwtColorMap* colorMap();
    const QwtColorMap* colorMap() const;

    void setPipeWidth( int );
    int pipeWidth() const;

    void setRangeFlags( QwtInterval::BorderFlags );
    QwtInterval::BorderFlags rangeFlags() const;

    double value() const;

    virtual QSize sizeHint() const QWT_OVERRIDE;
    virtual QSize minimumSizeHint() const QWT_OVERRIDE;

    void setScaleDraw( QwtScaleDraw* );
    const QwtScaleDraw* scaleDraw() const;

  public Q_SLOTS:
    virtual void setValue( double );

  protected:
    virtual void drawLiquid( QPainter*, const QRect& ) const;
    virtual void scaleChange() QWT_OVERRIDE;

    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;
    virtual void changeEvent( QEvent* ) QWT_OVERRIDE;

    QwtScaleDraw* scaleDraw();

    QRect pipeRect() const;
    QRect fillRect( const QRect& ) const;
    QRect alarmRect( const QRect& ) const;

  private:
    void layoutThermo( bool );

    class PrivateData;
    PrivateData* m_data;
};

#endif
