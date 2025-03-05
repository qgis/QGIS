/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COMPASS_H
#define QWT_COMPASS_H

#include "qwt_global.h"
#include "qwt_dial.h"
#include "qwt_round_scale_draw.h"

class QwtCompassRose;
class QString;
template< class Key, class T > class QMap;


/*!
   \brief A special scale draw made for QwtCompass

   QwtCompassScaleDraw maps values to strings using
   a special map, that can be modified by the application

   The default map consists of the labels N, NE, E, SE, S, SW, W, NW.

   \sa QwtCompass
 */
class QWT_EXPORT QwtCompassScaleDraw : public QwtRoundScaleDraw
{
  public:
    explicit QwtCompassScaleDraw();
    explicit QwtCompassScaleDraw( const QMap< double, QString >& map );

    virtual ~QwtCompassScaleDraw();

    void setLabelMap( const QMap< double, QString >& map );
    QMap< double, QString > labelMap() const;

    virtual QwtText label( double value ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief A Compass Widget

   QwtCompass is a widget to display and enter directions. It consists
   of a scale, an optional needle and rose.

   \image html dials1.png

   \note The examples/dials example shows how to use QwtCompass.
 */

class QWT_EXPORT QwtCompass : public QwtDial
{
    Q_OBJECT

  public:
    explicit QwtCompass( QWidget* parent = NULL );
    virtual ~QwtCompass();

    void setRose( QwtCompassRose* rose );
    const QwtCompassRose* rose() const;
    QwtCompassRose* rose();

  protected:
    virtual void drawRose( QPainter*, const QPointF& center,
        double radius, double north, QPalette::ColorGroup ) const;

    virtual void drawScaleContents( QPainter*,
        const QPointF& center, double radius ) const QWT_OVERRIDE;

    virtual void keyPressEvent( QKeyEvent* ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
