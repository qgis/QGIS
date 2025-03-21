/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_pixel_matrix.h"

/*!
   \brief Constructor

   \param rect Bounding rectangle for the matrix
 */
QwtPixelMatrix::QwtPixelMatrix( const QRect& rect )
    : QBitArray( qMax( rect.width() * rect.height(), 0 ) )
    , m_rect( rect )
{
}

//! Destructor
QwtPixelMatrix::~QwtPixelMatrix()
{
}

/*!
    Set the bounding rectangle of the matrix

    \param rect Bounding rectangle

    \note All bits are cleared
 */
void QwtPixelMatrix::setRect( const QRect& rect )
{
    if ( rect != m_rect )
    {
        m_rect = rect;
        const int sz = qMax( rect.width() * rect.height(), 0 );
        resize( sz );
    }

    fill( false );
}

//! \return Bounding rectangle
QRect QwtPixelMatrix::rect() const
{
    return m_rect;
}
