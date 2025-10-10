/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PANNER_H
#define QWT_PANNER_H

#include "qwt_global.h"
#include <qwidget.h>

class QCursor;
class QPixmap;

/*!
   \brief QwtPanner provides panning of a widget

   QwtPanner grabs the contents of a widget, that can be dragged
   in all directions. The offset between the start and the end position
   is emitted by the panned signal.

   QwtPanner grabs the content of the widget into a pixmap and moves
   the pixmap around, without initiating any repaint events for the widget.
   Areas, that are not part of content are not painted  while panning.
   This makes panning fast enough for widgets, where
   repaints are too slow for mouse movements.

   For widgets, where repaints are very fast it might be better to
   implement panning manually by mapping mouse events into paint events.
 */
class QWT_EXPORT QwtPanner : public QWidget
{
    Q_OBJECT

  public:
    explicit QwtPanner( QWidget* parent );
    virtual ~QwtPanner();

    void setEnabled( bool );
    bool isEnabled() const;

    void setMouseButton( Qt::MouseButton,
        Qt::KeyboardModifiers = Qt::NoModifier );
    void getMouseButton( Qt::MouseButton& button,
        Qt::KeyboardModifiers& ) const;

    void setAbortKey( int key, Qt::KeyboardModifiers = Qt::NoModifier );
    void getAbortKey( int& key, Qt::KeyboardModifiers& ) const;

    void setCursor( const QCursor& );
    const QCursor cursor() const;

    void setOrientations( Qt::Orientations );
    Qt::Orientations orientations() const;

    bool isOrientationEnabled( Qt::Orientation ) const;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

  Q_SIGNALS:
    /*!
       Signal emitted, when panning is done

       \param dx Offset in horizontal direction
       \param dy Offset in vertical direction
     */
    void panned( int dx, int dy );

    /*!
       Signal emitted, while the widget moved, but panning
       is not finished.

       \param dx Offset in horizontal direction
       \param dy Offset in vertical direction
     */
    void moved( int dx, int dy );

  protected:
    virtual void widgetMousePressEvent( QMouseEvent* );
    virtual void widgetMouseReleaseEvent( QMouseEvent* );
    virtual void widgetMouseMoveEvent( QMouseEvent* );
    virtual void widgetKeyPressEvent( QKeyEvent* );
    virtual void widgetKeyReleaseEvent( QKeyEvent* );

    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;

    virtual QBitmap contentsMask() const;
    virtual QPixmap grab() const;

  private:
#ifndef QT_NO_CURSOR
    void showCursor( bool );
#endif

    class PrivateData;
    PrivateData* m_data;
};

#endif
