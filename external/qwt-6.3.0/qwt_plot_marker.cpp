/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_marker.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>

class QwtPlotMarker::PrivateData
{
  public:
    PrivateData()
        : labelAlignment( Qt::AlignCenter )
        , labelOrientation( Qt::Horizontal )
        , spacing( 2 )
        , symbol( NULL )
        , style( QwtPlotMarker::NoLine )
        , xValue( 0.0 )
        , yValue( 0.0 )
    {
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
    Qt::Alignment labelAlignment;
    Qt::Orientation labelOrientation;
    int spacing;

    QPen pen;
    const QwtSymbol* symbol;
    LineStyle style;

    double xValue;
    double yValue;
};

//! Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
QwtPlotMarker::QwtPlotMarker()
{
    m_data = new PrivateData;
    setZ( 30.0 );
}

//! Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
QwtPlotMarker::QwtPlotMarker( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    m_data = new PrivateData;
    setZ( 30.0 );
}

//! Sets alignment to Qt::AlignCenter, and style to QwtPlotMarker::NoLine
QwtPlotMarker::QwtPlotMarker( const QwtText& title )
    : QwtPlotItem( title )
{
    m_data = new PrivateData;
    setZ( 30.0 );
}

//! Destructor
QwtPlotMarker::~QwtPlotMarker()
{
    delete m_data;
}

//! \return QwtPlotItem::Rtti_PlotMarker
int QwtPlotMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotMarker;
}

//! Return Value
QPointF QwtPlotMarker::value() const
{
    return QPointF( m_data->xValue, m_data->yValue );
}

//! Return x Value
double QwtPlotMarker::xValue() const
{
    return m_data->xValue;
}

//! Return y Value
double QwtPlotMarker::yValue() const
{
    return m_data->yValue;
}

//! Set Value
void QwtPlotMarker::setValue( const QPointF& pos )
{
    setValue( pos.x(), pos.y() );
}

//! Set Value
void QwtPlotMarker::setValue( double x, double y )
{
    if ( x != m_data->xValue || y != m_data->yValue )
    {
        m_data->xValue = x;
        m_data->yValue = y;
        itemChanged();
    }
}

//! Set X Value
void QwtPlotMarker::setXValue( double x )
{
    setValue( x, m_data->yValue );
}

//! Set Y Value
void QwtPlotMarker::setYValue( double y )
{
    setValue( m_data->xValue, y );
}

/*!
   Draw the marker

   \param painter Painter
   \param xMap x Scale Map
   \param yMap y Scale Map
   \param canvasRect Contents rectangle of the canvas in painter coordinates
 */
void QwtPlotMarker::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    const QPointF pos( xMap.transform( m_data->xValue ),
        yMap.transform( m_data->yValue ) );

    drawLines( painter, canvasRect, pos );
    drawSymbol( painter, canvasRect, pos );
    drawLabel( painter, canvasRect, pos );
}

/*!
   Draw the lines marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLines( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->style == NoLine )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    painter->setPen( m_data->pen );
    if ( m_data->style == QwtPlotMarker::HLine ||
        m_data->style == QwtPlotMarker::Cross )
    {
        double y = pos.y();
        if ( doAlign )
            y = qRound( y );

        QwtPainter::drawLine( painter, canvasRect.left(),
            y, canvasRect.right() - 1.0, y );
    }
    if ( m_data->style == QwtPlotMarker::VLine ||
        m_data->style == QwtPlotMarker::Cross )
    {
        double x = pos.x();
        if ( doAlign )
            x = qRound( x );

        QwtPainter::drawLine( painter, x,
            canvasRect.top(), x, canvasRect.bottom() - 1.0 );
    }
}

/*!
   Draw the symbol of the marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), QwtSymbol::drawSymbol()
 */
void QwtPlotMarker::drawSymbol( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->symbol == NULL )
        return;

    const QwtSymbol& symbol = *m_data->symbol;

    if ( symbol.style() != QwtSymbol::NoSymbol )
    {
        const QSizeF sz = symbol.size();

        const QRectF clipRect = canvasRect.adjusted(
            -sz.width(), -sz.height(), sz.width(), sz.height() );

        if ( clipRect.contains( pos ) )
            symbol.drawSymbol( painter, pos );
    }
}

/*!
   Align and draw the text label of the marker

   \param painter Painter
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param pos Position of the marker, translated into widget coordinates

   \sa drawLabel(), drawSymbol()
 */
void QwtPlotMarker::drawLabel( QPainter* painter,
    const QRectF& canvasRect, const QPointF& pos ) const
{
    if ( m_data->label.isEmpty() )
        return;

    Qt::Alignment align = m_data->labelAlignment;
    QPointF alignPos = pos;

    QSizeF symbolOff( 0, 0 );

    switch ( m_data->style )
    {
        case QwtPlotMarker::VLine:
        {
            // In VLine-style the y-position is pointless and
            // the alignment flags are relative to the canvas

            if ( m_data->labelAlignment & Qt::AlignTop )
            {
                alignPos.setY( canvasRect.top() );
                align &= ~Qt::AlignTop;
                align |= Qt::AlignBottom;
            }
            else if ( m_data->labelAlignment & Qt::AlignBottom )
            {
                // In HLine-style the x-position is pointless and
                // the alignment flags are relative to the canvas

                alignPos.setY( canvasRect.bottom() - 1 );
                align &= ~Qt::AlignBottom;
                align |= Qt::AlignTop;
            }
            else
            {
                alignPos.setY( canvasRect.center().y() );
            }
            break;
        }
        case QwtPlotMarker::HLine:
        {
            if ( m_data->labelAlignment & Qt::AlignLeft )
            {
                alignPos.setX( canvasRect.left() );
                align &= ~Qt::AlignLeft;
                align |= Qt::AlignRight;
            }
            else if ( m_data->labelAlignment & Qt::AlignRight )
            {
                alignPos.setX( canvasRect.right() - 1 );
                align &= ~Qt::AlignRight;
                align |= Qt::AlignLeft;
            }
            else
            {
                alignPos.setX( canvasRect.center().x() );
            }
            break;
        }
        default:
        {
            if ( m_data->symbol &&
                ( m_data->symbol->style() != QwtSymbol::NoSymbol ) )
            {
                symbolOff = m_data->symbol->size() + QSizeF( 1, 1 );
                symbolOff /= 2;
            }
        }
    }

    qreal pw2 = m_data->pen.widthF() / 2.0;
    if ( pw2 == 0.0 )
        pw2 = 0.5;

    const int spacing = m_data->spacing;

    const qreal xOff = qwtMaxF( pw2, symbolOff.width() );
    const qreal yOff = qwtMaxF( pw2, symbolOff.height() );

    const QSizeF textSize = m_data->label.textSize( painter->font() );

    if ( align & Qt::AlignLeft )
    {
        alignPos.rx() -= xOff + spacing;
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.rx() -= textSize.height();
        else
            alignPos.rx() -= textSize.width();
    }
    else if ( align & Qt::AlignRight )
    {
        alignPos.rx() += xOff + spacing;
    }
    else
    {
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.rx() -= textSize.height() / 2;
        else
            alignPos.rx() -= textSize.width() / 2;
    }

    if ( align & Qt::AlignTop )
    {
        alignPos.ry() -= yOff + spacing;
        if ( m_data->labelOrientation != Qt::Vertical )
            alignPos.ry() -= textSize.height();
    }
    else if ( align & Qt::AlignBottom )
    {
        alignPos.ry() += yOff + spacing;
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.ry() += textSize.width();
    }
    else
    {
        if ( m_data->labelOrientation == Qt::Vertical )
            alignPos.ry() += textSize.width() / 2;
        else
            alignPos.ry() -= textSize.height() / 2;
    }

    painter->translate( alignPos.x(), alignPos.y() );
    if ( m_data->labelOrientation == Qt::Vertical )
        painter->rotate( -90.0 );

    const QRectF textRect( 0, 0, textSize.width(), textSize.height() );
    m_data->label.draw( painter, textRect );
}

/*!
   \brief Set the line style
   \param style Line style.
   \sa lineStyle()
 */
void QwtPlotMarker::setLineStyle( LineStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return the line style
   \sa setLineStyle()
 */
QwtPlotMarker::LineStyle QwtPlotMarker::lineStyle() const
{
    return m_data->style;
}

/*!
   \brief Assign a symbol
   \param symbol New symbol
   \sa symbol()
 */
void QwtPlotMarker::setSymbol( const QwtSymbol* symbol )
{
    if ( symbol != m_data->symbol )
    {
        delete m_data->symbol;
        m_data->symbol = symbol;

        if ( symbol )
            setLegendIconSize( symbol->boundingRect().size() );

        legendChanged();
        itemChanged();
    }
}

/*!
   \return the symbol
   \sa setSymbol(), QwtSymbol
 */
const QwtSymbol* QwtPlotMarker::symbol() const
{
    return m_data->symbol;
}

/*!
   \brief Set the label
   \param label Label text
   \sa label()
 */
void QwtPlotMarker::setLabel( const QwtText& label )
{
    if ( label != m_data->label )
    {
        m_data->label = label;
        itemChanged();
    }
}

/*!
   \return the label
   \sa setLabel()
 */
QwtText QwtPlotMarker::label() const
{
    return m_data->label;
}

/*!
   \brief Set the alignment of the label

   In case of QwtPlotMarker::HLine the alignment is relative to the
   y position of the marker, but the horizontal flags correspond to the
   canvas rectangle. In case of QwtPlotMarker::VLine the alignment is
   relative to the x position of the marker, but the vertical flags
   correspond to the canvas rectangle.

   In all other styles the alignment is relative to the marker's position.

   \param align Alignment.
   \sa labelAlignment(), labelOrientation()
 */
void QwtPlotMarker::setLabelAlignment( Qt::Alignment align )
{
    if ( align != m_data->labelAlignment )
    {
        m_data->labelAlignment = align;
        itemChanged();
    }
}

/*!
   \return the label alignment
   \sa setLabelAlignment(), setLabelOrientation()
 */
Qt::Alignment QwtPlotMarker::labelAlignment() const
{
    return m_data->labelAlignment;
}

/*!
   \brief Set the orientation of the label

   When orientation is Qt::Vertical the label is rotated by 90.0 degrees
   ( from bottom to top ).

   \param orientation Orientation of the label

   \sa labelOrientation(), setLabelAlignment()
 */
void QwtPlotMarker::setLabelOrientation( Qt::Orientation orientation )
{
    if ( orientation != m_data->labelOrientation )
    {
        m_data->labelOrientation = orientation;
        itemChanged();
    }
}

/*!
   \return the label orientation
   \sa setLabelOrientation(), labelAlignment()
 */
Qt::Orientation QwtPlotMarker::labelOrientation() const
{
    return m_data->labelOrientation;
}

/*!
   \brief Set the spacing

   When the label is not centered on the marker position, the spacing
   is the distance between the position and the label.

   \param spacing Spacing
   \sa spacing(), setLabelAlignment()
 */
void QwtPlotMarker::setSpacing( int spacing )
{
    if ( spacing < 0 )
        spacing = 0;

    if ( spacing == m_data->spacing )
        return;

    m_data->spacing = spacing;
    itemChanged();
}

/*!
   \return the spacing
   \sa setSpacing()
 */
int QwtPlotMarker::spacing() const
{
    return m_data->spacing;
}

/*!
   Build and assign a line pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotMarker::setLinePen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setLinePen( QPen( color, width, style ) );
}

/*!
   Specify a pen for the line.

   \param pen New pen
   \sa linePen()
 */
void QwtPlotMarker::setLinePen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return the line pen
   \sa setLinePen()
 */
const QPen& QwtPlotMarker::linePen() const
{
    return m_data->pen;
}

QRectF QwtPlotMarker::boundingRect() const
{
    // width/height of -1 does not affect the autoscale calculation

    switch (m_data->style)
    {
        case QwtPlotMarker::HLine:
            return QRectF( m_data->xValue, m_data->yValue, -1.0, 0.0 );

        case QwtPlotMarker::VLine:
            return QRectF( m_data->xValue, m_data->yValue, 0.0, -1.0 );

        default:
            return QRectF( m_data->xValue, m_data->yValue, 0.0, 0.0 );
    }
}

/*!
   \return Icon representing the marker on the legend

   \param index Index of the legend entry
                ( usually there is only one )
   \param size Icon size

   \sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPlotMarker::legendIcon( int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    if ( size.isEmpty() )
        return QwtGraphic();

    QwtGraphic icon;
    icon.setDefaultSize( size );
    icon.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    if ( m_data->style != QwtPlotMarker::NoLine )
    {
        painter.setPen( m_data->pen );

        if ( m_data->style == QwtPlotMarker::HLine ||
            m_data->style == QwtPlotMarker::Cross )
        {
            const double y = 0.5 * size.height();

            QwtPainter::drawLine( &painter,
                0.0, y, size.width(), y );
        }

        if ( m_data->style == QwtPlotMarker::VLine ||
            m_data->style == QwtPlotMarker::Cross )
        {
            const double x = 0.5 * size.width();

            QwtPainter::drawLine( &painter,
                x, 0.0, x, size.height() );
        }
    }

    if ( m_data->symbol )
    {
        const QRect r( 0.0, 0.0, size.width(), size.height() );
        m_data->symbol->drawSymbol( &painter, r );
    }

    return icon;
}

