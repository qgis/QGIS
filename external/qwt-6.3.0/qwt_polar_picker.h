/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_PICKER_H
#define QWT_POLAR_PICKER_H

#include "qwt_global.h"
#include "qwt_picker.h"

#include <qvector.h>
#include <qpainterpath.h>

class QwtPolarPlot;
class QwtPolarCanvas;
class QwtPointPolar;

/*!
   \brief QwtPolarPicker provides selections on a plot canvas

   QwtPolarPicker is a QwtPicker tailored for selections on
   a polar plot canvas.
 */
class QWT_EXPORT QwtPolarPicker : public QwtPicker
{
    Q_OBJECT

  public:
    explicit QwtPolarPicker( QwtPolarCanvas* );
    virtual ~QwtPolarPicker();

    explicit QwtPolarPicker(
        RubberBand rubberBand, DisplayMode trackerMode,
        QwtPolarCanvas* );

    QwtPolarPlot* plot();
    const QwtPolarPlot* plot() const;

    QwtPolarCanvas* canvas();
    const QwtPolarCanvas* canvas() const;

    virtual QRect pickRect() const;

  Q_SIGNALS:

    /*!
       A signal emitted in case of selectionFlags() & PointSelection.
       \param pos Selected point
     */
    void selected( const QwtPointPolar& pos );

    /*!
       A signal emitting the selected points,
       at the end of a selection.

       \param points Selected points
     */
    void selected( const QVector< QwtPointPolar >& points );

    /*!
       A signal emitted when a point has been appended to the selection

       \param pos Position of the appended point.
       \sa append(). moved()
     */
    void appended( const QwtPointPolar& pos );

    /*!
       A signal emitted whenever the last appended point of the
       selection has been moved.

       \param pos Position of the moved last point of the selection.
       \sa move(), appended()
     */
    void moved( const QwtPointPolar& pos );

  protected:
    QwtPointPolar invTransform( const QPoint& ) const;

    virtual QwtText trackerText( const QPoint& ) const QWT_OVERRIDE;
    virtual QwtText trackerTextPolar( const QwtPointPolar& ) const;

    virtual void move( const QPoint& ) QWT_OVERRIDE;
    virtual void append( const QPoint& ) QWT_OVERRIDE;
    virtual bool end( bool ok = true ) QWT_OVERRIDE;

  private:
    virtual QPainterPath pickArea() const QWT_OVERRIDE;

    class PrivateData;
    PrivateData* m_data;
};

#endif
