/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_MAGNIFIER_H
#define QWT_MAGNIFIER_H

#include "qwt_global.h"
#include <qobject.h>

class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

/*!
   \brief QwtMagnifier provides zooming, by magnifying in steps.

   Using QwtMagnifier a plot can be zoomed in/out in steps using
   keys, the mouse wheel or moving a mouse button in vertical direction.
 */
class QWT_EXPORT QwtMagnifier : public QObject
{
    Q_OBJECT

  public:
    explicit QwtMagnifier( QWidget* );
    virtual ~QwtMagnifier();

    QWidget* parentWidget();
    const QWidget* parentWidget() const;

    void setEnabled( bool );
    bool isEnabled() const;

    // mouse
    void setMouseFactor( double );
    double mouseFactor() const;

    void setMouseButton( Qt::MouseButton, Qt::KeyboardModifiers = Qt::NoModifier );
    void getMouseButton( Qt::MouseButton&, Qt::KeyboardModifiers& ) const;

    // mouse wheel
    void setWheelFactor( double );
    double wheelFactor() const;

    void setWheelModifiers( Qt::KeyboardModifiers );
    Qt::KeyboardModifiers wheelModifiers() const;

    // keyboard
    void setKeyFactor( double );
    double keyFactor() const;

    void setZoomInKey( int key, Qt::KeyboardModifiers = Qt::NoModifier );
    void getZoomInKey( int& key, Qt::KeyboardModifiers& ) const;

    void setZoomOutKey( int key, Qt::KeyboardModifiers = Qt::NoModifier );
    void getZoomOutKey( int& key, Qt::KeyboardModifiers& ) const;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

  protected:
    /*!
       Rescale the parent widget
       \param factor Scale factor
     */
    virtual void rescale( double factor ) = 0;

    virtual void widgetMousePressEvent( QMouseEvent* );
    virtual void widgetMouseReleaseEvent( QMouseEvent* );
    virtual void widgetMouseMoveEvent( QMouseEvent* );
    virtual void widgetWheelEvent( QWheelEvent* );
    virtual void widgetKeyPressEvent( QKeyEvent* );
    virtual void widgetKeyReleaseEvent( QKeyEvent* );

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
