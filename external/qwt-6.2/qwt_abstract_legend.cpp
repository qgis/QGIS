/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_abstract_legend.h"
#include "qwt_legend_data.h"

/*!
   Constructor

   \param parent Parent widget
 */
QwtAbstractLegend::QwtAbstractLegend( QWidget* parent )
    : QFrame( parent )
{
}

//! Destructor
QwtAbstractLegend::~QwtAbstractLegend()
{
}

/*!
   Return the extent, that is needed for elements to scroll
   the legend ( usually scrollbars ),

   \param orientation Orientation
   \return Extent of the corresponding scroll element
 */
int QwtAbstractLegend::scrollExtent( Qt::Orientation orientation ) const
{
    Q_UNUSED( orientation );
    return 0;
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_abstract_legend.cpp"
#endif
