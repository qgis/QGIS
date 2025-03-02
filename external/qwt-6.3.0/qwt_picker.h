/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PICKER
#define QWT_PICKER

#include "qwt_global.h"
#include "qwt_event_pattern.h"

#include <qobject.h>

class QwtPickerMachine;
class QwtWidgetOverlay;
class QwtText;
class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QPainter;
class QPen;
class QFont;
class QRegion;
class QPainterPath;
class QPoint;
class QRect;
class QSize;
class QPolygon;

/*!
   \brief QwtPicker provides selections on a widget

   QwtPicker filters all enter, leave, mouse and keyboard events of a widget
   and translates them into an array of selected points.

   The way how the points are collected depends on type of state machine
   that is connected to the picker. Qwt offers a couple of predefined
   state machines for selecting:

   - Nothing\n
    QwtPickerTrackerMachine
   - Single points\n
    QwtPickerClickPointMachine, QwtPickerDragPointMachine
   - Rectangles\n
    QwtPickerClickRectMachine, QwtPickerDragRectMachine
   - Polygons\n
    QwtPickerPolygonMachine

   While these state machines cover the most common ways to collect points
   it is also possible to implement individual machines as well.

   QwtPicker translates the picked points into a selection using the
   adjustedPoints() method. adjustedPoints() is intended to be reimplemented
   to fix up the selection according to application specific requirements.
   (F.e. when an application accepts rectangles of a fixed aspect ratio only.)

   Optionally QwtPicker support the process of collecting points by a
   rubber band and tracker displaying a text for the current mouse
   position.

   \par Example
   \code
 #include <qwt_picker.h>
 #include <qwt_picker_machine.h>

    QwtPicker *picker = new QwtPicker(widget);
    picker->setStateMachine(new QwtPickerDragRectMachine);
    picker->setTrackerMode(QwtPicker::ActiveOnly);
    picker->setRubberBand(QwtPicker::RectRubberBand);
   \endcode

   The state machine triggers the following commands:

   - begin()\n
    Activate/Initialize the selection.
   - append()\n
    Add a new point
   - move() \n
    Change the position of the last point.
   - remove()\n
    Remove the last point.
   - end()\n
    Terminate the selection and call accept to validate the picked points.

   The picker is active (isActive()), between begin() and end().
   In active state the rubber band is displayed, and the tracker is visible
   in case of trackerMode is ActiveOnly or AlwaysOn.

   The cursor can be moved using the arrow keys. All selections can be aborted
   using the abort key. (QwtEventPattern::KeyPatternCode)

   \warning In case of QWidget::NoFocus the focus policy of the observed
           widget is set to QWidget::WheelFocus and mouse tracking
           will be manipulated while the picker is active,
           or if trackerMode() is AlwayOn.
 */

class QWT_EXPORT QwtPicker : public QObject, public QwtEventPattern
{
    Q_OBJECT

    Q_ENUMS( RubberBand DisplayMode ResizeMode )

    Q_PROPERTY( bool isEnabled READ isEnabled WRITE setEnabled )
    Q_PROPERTY( ResizeMode resizeMode READ resizeMode WRITE setResizeMode )

    Q_PROPERTY( DisplayMode trackerMode READ trackerMode WRITE setTrackerMode )
    Q_PROPERTY( QPen trackerPen READ trackerPen WRITE setTrackerPen )
    Q_PROPERTY( QFont trackerFont READ trackerFont WRITE setTrackerFont )

    Q_PROPERTY( RubberBand rubberBand READ rubberBand WRITE setRubberBand )
    Q_PROPERTY( QPen rubberBandPen READ rubberBandPen WRITE setRubberBandPen )

  public:
    /*!
       Rubber band style

       The default value is QwtPicker::NoRubberBand.
       \sa setRubberBand(), rubberBand()
     */

    enum RubberBand
    {
        //! No rubberband.
        NoRubberBand = 0,

        //! A horizontal line ( only for QwtPickerMachine::PointSelection )
        HLineRubberBand,

        //! A vertical line ( only for QwtPickerMachine::PointSelection )
        VLineRubberBand,

        //! A crosshair ( only for QwtPickerMachine::PointSelection )
        CrossRubberBand,

        //! A rectangle ( only for QwtPickerMachine::RectSelection )
        RectRubberBand,

        //! An ellipse ( only for QwtPickerMachine::RectSelection )
        EllipseRubberBand,

        //! A polygon ( only for QwtPickerMachine::PolygonSelection )
        PolygonRubberBand,

        /*!
           Values >= UserRubberBand can be used to define additional
           rubber bands.
         */
        UserRubberBand = 100
    };

    /*!
       \brief Display mode
       \sa setTrackerMode(), trackerMode(), isActive()
     */
    enum DisplayMode
    {
        //! Display never
        AlwaysOff,

        //! Display always
        AlwaysOn,

        //! Display only when the selection is active
        ActiveOnly
    };

    /*!
       Controls what to do with the selected points of an active
         selection when the observed widget is resized.

       The default value is QwtPicker::Stretch.
       \sa setResizeMode()
     */

    enum ResizeMode
    {
        //! All points are scaled according to the new size,
        Stretch,

        //! All points remain unchanged.
        KeepSize
    };

    explicit QwtPicker( QWidget* parent );
    explicit QwtPicker( RubberBand rubberBand,
        DisplayMode trackerMode, QWidget* );

    virtual ~QwtPicker();

    void setStateMachine( QwtPickerMachine* );
    const QwtPickerMachine* stateMachine() const;
    QwtPickerMachine* stateMachine();

    void setRubberBand( RubberBand );
    RubberBand rubberBand() const;

    void setTrackerMode( DisplayMode );
    DisplayMode trackerMode() const;

    void setResizeMode( ResizeMode );
    ResizeMode resizeMode() const;

    void setRubberBandPen( const QPen& );
    QPen rubberBandPen() const;

    void setTrackerPen( const QPen& );
    QPen trackerPen() const;

    void setTrackerFont( const QFont& );
    QFont trackerFont() const;

    bool isEnabled() const;
    bool isActive() const;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

    QWidget* parentWidget();
    const QWidget* parentWidget() const;

    virtual QPainterPath pickArea() const;

    virtual void drawRubberBand( QPainter* ) const;
    virtual void drawTracker( QPainter* ) const;

    virtual QRegion trackerMask() const;
    virtual QRegion rubberBandMask() const;

    virtual QwtText trackerText( const QPoint& pos ) const;
    QPoint trackerPosition() const;
    virtual QRect trackerRect( const QFont& ) const;

    QPolygon selection() const;

  public Q_SLOTS:
    void setEnabled( bool );

  Q_SIGNALS:
    /*!
       A signal indicating, when the picker has been activated.
       Together with setEnabled() it can be used to implement
       selections with more than one picker.

       \param on True, when the picker has been activated
     */
    void activated( bool on );

    /*!
       A signal emitting the selected points,
       at the end of a selection.

       \param polygon Selected points
     */
    void selected( const QPolygon& polygon );

    /*!
       A signal emitted when a point has been appended to the selection

       \param pos Position of the appended point.
       \sa append(). moved()
     */
    void appended( const QPoint& pos );

    /*!
       A signal emitted whenever the last appended point of the
       selection has been moved.

       \param pos Position of the moved last point of the selection.
       \sa move(), appended()
     */
    void moved( const QPoint& pos );

    /*!
       A signal emitted whenever the last appended point of the
       selection has been removed.

       \param pos Position of the point, that has been removed
       \sa remove(), appended()
     */
    void removed( const QPoint& pos );
    /*!
       A signal emitted when the active selection has been changed.
       This might happen when the observed widget is resized.

       \param selection Changed selection
       \sa stretchSelection()
     */
    void changed( const QPolygon& selection );

  protected:
    virtual QPolygon adjustedPoints( const QPolygon& ) const;

    virtual void transition( const QEvent* );

    virtual void begin();
    virtual void append( const QPoint& );
    virtual void move( const QPoint& );
    virtual void remove();
    virtual bool end( bool ok = true );

    virtual bool accept( QPolygon& ) const;
    virtual void reset();

    virtual void widgetMousePressEvent( QMouseEvent* );
    virtual void widgetMouseReleaseEvent( QMouseEvent* );
    virtual void widgetMouseDoubleClickEvent( QMouseEvent* );
    virtual void widgetMouseMoveEvent( QMouseEvent* );
    virtual void widgetWheelEvent( QWheelEvent* );
    virtual void widgetKeyPressEvent( QKeyEvent* );
    virtual void widgetKeyReleaseEvent( QKeyEvent* );
    virtual void widgetEnterEvent( QEvent* );
    virtual void widgetLeaveEvent( QEvent* );

    virtual void stretchSelection(
        const QSize& oldSize, const QSize& newSize );

    virtual void updateDisplay();

    const QwtWidgetOverlay* rubberBandOverlay() const;
    const QwtWidgetOverlay* trackerOverlay() const;

    const QPolygon& pickedPoints() const;
    QRect trackerRect( const QSize& ) const;

  private:
    void init( QWidget*, RubberBand rubberBand, DisplayMode trackerMode );

    void setMouseTracking( bool );

    class PrivateData;
    PrivateData* m_data;
};

#endif
