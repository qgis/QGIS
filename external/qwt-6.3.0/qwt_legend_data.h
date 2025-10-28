/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_LEGEND_DATA_H
#define QWT_LEGEND_DATA_H

#include "qwt_global.h"

#include <qvariant.h>
#include <qmap.h>

class QwtText;
class QwtGraphic;

/*!
   \brief Attributes of an entry on a legend

   QwtLegendData is an abstract container ( like QAbstractModel )
   to exchange attributes, that are only known between to
   the plot item and the legend.

   By overloading QwtPlotItem::legendData() any other set of attributes
   could be used, that can be handled by a modified ( or completely
   different ) implementation of a legend.

   \sa QwtLegend, QwtPlotLegendItem
   \note The stockchart example implements a legend as a tree
        with checkable items
 */
class QWT_EXPORT QwtLegendData
{
  public:
    //! Mode defining how a legend entry interacts
    enum Mode
    {
        //! The legend item is not interactive, like a label
        ReadOnly,

        //! The legend item is clickable, like a push button
        Clickable,

        //! The legend item is checkable, like a checkable button
        Checkable
    };

    //! Identifier how to interpret a QVariant
    enum Role
    {
        // The value is a Mode
        ModeRole,

        // The value is a title
        TitleRole,

        // The value is an icon
        IconRole,

        // Values < UserRole are reserved for internal use
        UserRole  = 32
    };

    QwtLegendData();
    ~QwtLegendData();

    void setValues( const QMap< int, QVariant >& );
    const QMap< int, QVariant >& values() const;

    void setValue( int role, const QVariant& );
    QVariant value( int role ) const;

    bool hasRole( int role ) const;
    bool isValid() const;

    QwtGraphic icon() const;
    QwtText title() const;
    Mode mode() const;

  private:
    QMap< int, QVariant > m_map;
};

#endif
