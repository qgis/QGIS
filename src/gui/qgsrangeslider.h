/***************************************************************************
    qgsrangeslider.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGESLIDER_H
#define QGSRANGESLIDER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QWidget>
#include <QStyleOptionSlider>

/**
 * \ingroup gui
 * \brief A slider control with two interactive endpoints, for interactive selection of a range of values.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRangeSlider : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRangeSlider, with the specified \a parent widget.
     */
    QgsRangeSlider( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsRangeSlider, with the specified \a parent widget.
     *
     * The \a orientation parameter determines whether the slider is horizontal or vertical.
     */
    QgsRangeSlider( Qt::Orientation orientation, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the minimum value allowed by the widget.
     *
     * \see setMinimum()
     * \see maximum()
     */
    int minimum() const;

    /**
     * Returns the maximum value allowed by the widget.
     *
     * \see setMaximum()
     * \see minimum()
     */
    int maximum() const;

    /**
     * Returns the lower value for the range selected in the widget.
     *
     * \see upperValue()
     * \see setLowerValue()
     */
    int lowerValue() const;

    /**
     * Returns the upper value for the range selected in the widget.
     *
     * \see lowerValue()
     * \see setUpperValue()
     */
    int upperValue() const;

    /**
     * Sets the \a position of the tick marks shown in the widget.
     *
     * \see tickPosition()
     */
    void setTickPosition( QSlider::TickPosition position );

    /**
     * Returns the position of the tick marks shown in the widget.
     *
     * \see setTickPosition()
     */
    QSlider::TickPosition tickPosition() const;

    /**
     * Sets the \a interval for tick marks shown in the widget.
     *
     * \see tickInterval()
     */
    void setTickInterval( int interval );

    /**
     * Returns the interval for tick marks shown in the widget.
     *
     * \see setTickInterval()
     */
    int tickInterval() const;

    /**
     * Sets the \a orientation of the slider.
     *
     * \see orientation()
     */
    void setOrientation( Qt::Orientation orientation );

    /**
     * Returns the orientation of the slider.
     *
     * \see setOrientation()
     */
    Qt::Orientation orientation() const;

    /**
     * Returns TRUE if the slider has its values flipped.
     *
     * If this property is FALSE (the default), the minimum and maximum will be shown in its classic
     * position for the widget. If the value is TRUE, the minimum and maximum appear at their opposite location.
     *
     * \see setFlippedDirection()
     */
    bool flippedDirection() const;

    /**
     * Sets whether the slider has its values \a flipped.
     *
     * If this property is FALSE (the default), the minimum and maximum will be shown in its classic
     * position for the widget. If the value is TRUE, the minimum and maximum appear at their opposite location.
     * (i.e. minimum at the bottom of a vertical slider, maximum at the top of a vertical slider).
     *
     * \see flippedDirection()
     */
    void setFlippedDirection( bool flipped );

    void paintEvent( QPaintEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    /**
     * Returns the single step value for the widget.
     *
     * This corresponds to the smaller increment or decrement applied when the user presses an arrow key.
     *
     * \see setSingleStep()
     * \see pageStep()
     */
    int singleStep() const;

    /**
     * Returns the page step value for the widget.
     *
     * This corresponds to the larger increment or decrement applied when the user presses the page increment key (usually PageUp or PageDown).
     *
     * \see setPageStep()
     * \see singleStep()
     */
    int pageStep() const;

    /**
     * Returns the slider's fixed range size, or -1 if not set.
     *
     * If a fixed range size is set then moving either the lower or upper slider will automatically
     * move the other slider accordingly, in order to keep the selected range at the specified
     * fixed size.
     *
     * \see setFixedRangeSize()
     * \since QGIS 3.38
     */
    int fixedRangeSize() const;

    /**
     * Sets the slider's fixed range \a size. Set to -1 if no fixed size is desired.
     *
     * If a fixed range size is set then moving either the lower or upper slider will automatically
     * move the other slider accordingly, in order to keep the selected range at the specified
     * fixed size.
     *
     * \see fixedRangeSize()
     * \since QGIS 3.38
     */
    void setFixedRangeSize( int size );

  public slots:

    /**
     * Sets the \a maximum value allowed in the widget.
     *
     * \see maximum()
     * \see setMinimum()
     */
    void setMaximum( int maximum );

    /**
     * Sets the \a minimum value allowed in the widget.
     *
     * \see minimum()
     * \see setMaximum()
     */
    void setMinimum( int minimum );

    /**
     * Sets the \a minimum and \a maximum range limits for values allowed in the widget.
     *
     * \see setMinimum()
     * \see setMaximum()
     */
    void setRangeLimits( int minimum, int maximum );

    /**
     * Sets the lower \a value for the range currently selected in the widget.
     *
     * \see lowerValue()
     * \see setRange()
     * \see setUpperValue()
     */
    void setLowerValue( int value );

    /**
     * Sets the upper \a value for the range currently selected in the widget.
     *
     * \see upperValue()
     * \see setRange()
     * \see setLowerValue()
     */
    void setUpperValue( int value );

    /**
     * Sets the current range selected in the widget.
     *
     * \see setLowerValue()
     * \see setUpperValue()
     */
    void setRange( int lower, int upper );

    /**
     * Sets the single \a step value for the widget.
     *
     * This corresponds to the smaller increment or decrement applied when the user presses an arrow key.
     *
     * \see singleStep()
     * \see pageStep()
     */
    void setSingleStep( int step );

    /**
     * Sets the page \a step value for the widget.
     *
     * This corresponds to the larger increment or decrement applied when the user presses the page increment key (usually PageUp or PageDown).
     *
     * \see pageStep()
     * \see setSingleStep()
     */
    void setPageStep( int step );

    bool event( QEvent *event ) override;

  signals:

    /**
     * Emitted when the range selected in the widget is changed.
     */
    void rangeChanged( int minimum, int maximum );

    /**
     * Emitted when the limits of values allowed in the widget is changed.
     */
    void rangeLimitsChanged( int minimum, int maximum );

    /**
     * Emitted when the widget's fixed range size is changed.
     *
     * \see fixedRangeSize()
     * \see setFixedRangeSize()
     *
     * \since QGIS 3.38
     */
    void fixedRangeSizeChanged( int size );

  private:

    int pick( const QPoint &pt ) const;
    int pixelPosToRangeValue( int pos ) const;
    bool updateHoverControl( const QPoint &pos );
    bool newHoverControl( const QPoint &pos );
    QRect selectedRangeRect();
    void drawFocusRect();

    int mLowerValue = 0;
    int mUpperValue = 0;

    int mSingleStep = 1;
    int mPageStep = 10;

    int mFixedRangeSize = -1;

    QStyleOptionSlider mStyleOption;
    enum Control
    {
      None,
      Lower,
      Upper,
      Both,
      Range
    };

    void applyStep( int step );
    int unFlippedSliderPosition( int value ) const;

    Control mActiveControl = None;
    int mStartDragPos = -1;
    int mLowerClickOffset = 0;
    int mUpperClickOffset = 0;
    int mRangeDragOffset = 0;
    int mPreDragLowerValue = -1;
    int mPreDragUpperValue = -1;
    Control mHoverControl = None;
    Control mFocusControl = Lower;
    QStyle::SubControl mHoverSubControl = QStyle::SC_None;
    QRect mHoverRect;

    bool mFlipped = false;
};

#endif // QGSRANGESLIDER_H
