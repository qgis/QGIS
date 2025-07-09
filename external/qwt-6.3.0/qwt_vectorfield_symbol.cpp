/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_vectorfield_symbol.h"

#include <qpainter.h>
#include <qpainterpath.h>

//! Constructor
QwtVectorFieldSymbol::QwtVectorFieldSymbol()
{
}

//! Destructor
QwtVectorFieldSymbol::~QwtVectorFieldSymbol()
{
}

class QwtVectorFieldArrow::PrivateData
{
  public:
    PrivateData( qreal headW, qreal tailW )
        : headWidth( headW )
        , tailWidth( tailW )
        , length( headW + 4.0 )
    {
        /*
            Arrow is drawn horizontally, pointing into positive x direction
            with tip at 0,0.
         */

        path.lineTo( -headWidth, headWidth );
        path.lineTo( -headWidth, tailWidth );
        path.lineTo( -length, tailWidth );
        path.lineTo( -length, -tailWidth );
        path.lineTo( -headWidth, -tailWidth );
        path.lineTo( -headWidth, -headWidth );

        path.closeSubpath();
    }

    void setLength( qreal l )
    {
        length = qMax( l, headWidth );

        path.setElementPositionAt( 3, -length, tailWidth );
        path.setElementPositionAt( 4, -length, -tailWidth );
    }

    const qreal headWidth;
    const qreal tailWidth;
    qreal length;

    QPainterPath path;

};

/*!
    \brief Constructor

    The length is initialized by headWidth + 4

    \param headWidth Width of the triangular head
    \param tailWidth Width of the arrow tail

    \sa setLength()
 */
QwtVectorFieldArrow::QwtVectorFieldArrow( qreal headWidth, qreal tailWidth )
{
    m_data = new PrivateData( headWidth, tailWidth );
}

//! Destructor
QwtVectorFieldArrow::~QwtVectorFieldArrow()
{
    delete m_data;
}

void QwtVectorFieldArrow::setLength( qreal length )
{
    m_data->setLength( length );
}

qreal QwtVectorFieldArrow::length() const
{
    return m_data->length;
}

void QwtVectorFieldArrow::paint( QPainter* painter ) const
{
    painter->drawPath( m_data->path );
}

class QwtVectorFieldThinArrow::PrivateData
{
  public:
    PrivateData( qreal headW )
        : headWidth( headW )
        , length( headW + 4.0 )
    {
        path.lineTo( -headWidth, headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -headWidth, -headWidth * 0.6 );
        path.moveTo( 0, 0 );
        path.lineTo( -length, 0 );
    }

    const qreal headWidth;
    qreal length;

    QPainterPath path;
};

/*!
    \brief Constructor

    The length is initialized by headWidth + 4

    \param headWidth Width of the triangular head
    \sa setLength()
 */
QwtVectorFieldThinArrow::QwtVectorFieldThinArrow( qreal headWidth )
{
    m_data = new PrivateData( headWidth );
}

//! \brief Destructor
QwtVectorFieldThinArrow::~QwtVectorFieldThinArrow()
{
    delete m_data;
}

void QwtVectorFieldThinArrow::setLength( qreal length )
{
    m_data->length = length;

    const qreal headWidth = qMin( m_data->headWidth, length / 3.0 );

    QPainterPath& path = m_data->path;

    path.setElementPositionAt( 1, -headWidth, headWidth * 0.6 );
    path.setElementPositionAt( 3, -headWidth, -headWidth * 0.6 );
    path.setElementPositionAt( 5, -length, 0 );
}

qreal QwtVectorFieldThinArrow::length() const
{
    return m_data->length;
}

void QwtVectorFieldThinArrow::paint(QPainter* p) const
{
    p->drawPath( m_data->path );
}
