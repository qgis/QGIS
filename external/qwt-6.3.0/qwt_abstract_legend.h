/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_ABSTRACT_LEGEND_H
#define QWT_ABSTRACT_LEGEND_H

#include "qwt_global.h"
#include <qframe.h>

class QwtLegendData;
template< typename T > class QList;
class QVariant;

/*!
   \brief Abstract base class for legend widgets

   Legends, that need to be under control of the QwtPlot layout system
   need to be derived from QwtAbstractLegend.

   \note Other type of legends can be implemented by connecting to
        the QwtPlot::legendDataChanged() signal. But as these legends
        are unknown to the plot layout system the layout code
        ( on screen and for QwtPlotRenderer ) need to be organized
        in application code.

   \sa QwtLegend
 */
class QWT_EXPORT QwtAbstractLegend : public QFrame
{
    Q_OBJECT

  public:
    explicit QwtAbstractLegend( QWidget* parent = NULL );
    virtual ~QwtAbstractLegend();

    /*!
       Render the legend into a given rectangle.

       \param painter Painter
       \param rect Bounding rectangle
       \param fillBackground When true, fill rect with the widget background

       \sa renderLegend() is used by QwtPlotRenderer
     */
    virtual void renderLegend( QPainter* painter,
        const QRectF& rect, bool fillBackground ) const = 0;

    //! \return True, when no plot item is inserted
    virtual bool isEmpty() const = 0;

    virtual int scrollExtent( Qt::Orientation ) const;

  public Q_SLOTS:

    /*!
       \brief Update the entries for a plot item

       \param itemInfo Info about an item
       \param data List of legend entry attributes for the  item
     */
    virtual void updateLegend( const QVariant& itemInfo,
        const QList< QwtLegendData >& data ) = 0;
};

#endif
