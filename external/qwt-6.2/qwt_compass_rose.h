/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COMPASS_ROSE_H
#define QWT_COMPASS_ROSE_H

#include "qwt_global.h"
#include <qpalette.h>

class QPainter;

/*!
   \brief Abstract base class for a compass rose
 */
class QWT_EXPORT QwtCompassRose
{
  public:
    QwtCompassRose();
    virtual ~QwtCompassRose();

    virtual void setPalette( const QPalette& );
    const QPalette& palette() const;

    /*!
        Draw the rose

        \param painter Painter
        \param center Center point
        \param radius Radius of the rose
        \param north Position
        \param colorGroup Color group
     */
    virtual void draw( QPainter* painter,
        const QPointF& center, double radius, double north,
        QPalette::ColorGroup colorGroup = QPalette::Active ) const = 0;

  private:
    Q_DISABLE_COPY(QwtCompassRose)

    QPalette m_palette;
};

/*!
   \brief A simple rose for QwtCompass
 */
class QWT_EXPORT QwtSimpleCompassRose : public QwtCompassRose
{
  public:
    QwtSimpleCompassRose( int numThorns = 8, int numThornLevels = -1 );
    virtual ~QwtSimpleCompassRose();

    void setWidth( double );
    double width() const;

    void setNumThorns( int );
    int numThorns() const;

    void setNumThornLevels( int );
    int numThornLevels() const;

    void setShrinkFactor( double factor );
    double shrinkFactor() const;

    virtual void draw( QPainter*,
        const QPointF& center, double radius, double north,
        QPalette::ColorGroup = QPalette::Active ) const QWT_OVERRIDE;

    static void drawRose( QPainter*, const QPalette&,
        const QPointF& center, double radius, double north, double width,
        int numThorns, int numThornLevels, double shrinkFactor );

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
