/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_text_label.h"
#include "qwt_text.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmargins.h>

class QwtTextLabel::PrivateData
{
  public:
    PrivateData()
        : indent( 4 )
        , margin( 0 )
    {
    }

    int indent;
    int margin;
    QwtText text;
};

/*!
   Constructs an empty label.
   \param parent Parent widget
 */
QwtTextLabel::QwtTextLabel( QWidget* parent )
    : QFrame( parent )
{
    init();
}

/*!
   Constructs a label that displays the text, text
   \param parent Parent widget
   \param text Text
 */
QwtTextLabel::QwtTextLabel( const QwtText& text, QWidget* parent )
    : QFrame( parent )
{
    init();
    m_data->text = text;
}

//! Destructor
QwtTextLabel::~QwtTextLabel()
{
    delete m_data;
}

void QwtTextLabel::init()
{
    m_data = new PrivateData();
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

/*!
   Interface for the designer plugin - does the same as setText()
   \sa plainText()
 */
void QwtTextLabel::setPlainText( const QString& text )
{
    setText( QwtText( text ) );
}

/*!
   Interface for the designer plugin

   \return Text as plain text
   \sa setPlainText(), text()
 */
QString QwtTextLabel::plainText() const
{
    return m_data->text.text();
}

/*!
   Change the label's text, keeping all other QwtText attributes
   \param text New text
   \param textFormat Format of text

   \sa QwtText
 */
void QwtTextLabel::setText( const QString& text,
    QwtText::TextFormat textFormat )
{
    m_data->text.setText( text, textFormat );

    update();
    updateGeometry();
}

/*!
   Change the label's text
   \param text New text
 */
void QwtTextLabel::setText( const QwtText& text )
{
    m_data->text = text;

    update();
    updateGeometry();
}

//! Return the text
const QwtText& QwtTextLabel::text() const
{
    return m_data->text;
}

//! Clear the text and all QwtText attributes
void QwtTextLabel::clear()
{
    m_data->text = QwtText();

    update();
    updateGeometry();
}

//! Return label's text indent in pixels
int QwtTextLabel::indent() const
{
    return m_data->indent;
}

/*!
   Set label's text indent in pixels
   \param indent Indentation in pixels
 */
void QwtTextLabel::setIndent( int indent )
{
    if ( indent < 0 )
        indent = 0;

    m_data->indent = indent;

    update();
    updateGeometry();
}

//! Return label's text margin in pixels
int QwtTextLabel::margin() const
{
    return m_data->margin;
}

/*!
   Set label's margin in pixels
   \param margin Margin in pixels
 */
void QwtTextLabel::setMargin( int margin )
{
    m_data->margin = margin;

    update();
    updateGeometry();
}

//! Return a size hint
QSize QwtTextLabel::sizeHint() const
{
    return minimumSizeHint();
}

//! Return a minimum size hint
QSize QwtTextLabel::minimumSizeHint() const
{
    QSizeF sz = m_data->text.textSize( font() );

    const QMargins m = contentsMargins();

    int mw = m.left() + m.right() + 2 * m_data->margin;
    int mh = m.top() + m.bottom() + 2 * m_data->margin;

    int indent = m_data->indent;
    if ( indent <= 0 )
        indent = defaultIndent();

    if ( indent > 0 )
    {
        const int align = m_data->text.renderFlags();
        if ( align & Qt::AlignLeft || align & Qt::AlignRight )
            mw += m_data->indent;
        else if ( align & Qt::AlignTop || align & Qt::AlignBottom )
            mh += m_data->indent;
    }

    sz += QSizeF( mw, mh );

    return QSize( qwtCeil( sz.width() ), qwtCeil( sz.height() ) );
}

/*!
   \param width Width
   \return Preferred height for this widget, given the width.
 */
int QwtTextLabel::heightForWidth( int width ) const
{
    const int renderFlags = m_data->text.renderFlags();

    int indent = m_data->indent;
    if ( indent <= 0 )
        indent = defaultIndent();

    const QMargins m = contentsMargins();

    width -= m.left() + m.right() - 2 * m_data->margin;
    if ( renderFlags & Qt::AlignLeft || renderFlags & Qt::AlignRight )
        width -= indent;

    int height = qwtCeil( m_data->text.heightForWidth( width, font() ) );
    if ( ( renderFlags & Qt::AlignTop ) || ( renderFlags & Qt::AlignBottom ) )
        height += indent;

    height += m.top() + m.bottom() + 2 * m_data->margin;

    return height;
}

/*!
   Qt paint event
   \param event Paint event
 */
void QwtTextLabel::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if ( !contentsRect().contains( event->rect() ) )
    {
        painter.setClipRegion( event->region() & frameRect() );
        drawFrame( &painter );
    }

    painter.setClipRegion( event->region() & contentsRect() );

    drawContents( &painter );
}

//! Redraw the text and focus indicator
void QwtTextLabel::drawContents( QPainter* painter )
{
    const QRect r = textRect();
    if ( r.isEmpty() )
        return;

    painter->setFont( font() );
    painter->setPen( palette().color( QPalette::Active, QPalette::Text ) );

    drawText( painter, QRectF( r ) );

    if ( hasFocus() )
    {
        const int m = 2;

        QRect focusRect = contentsRect().adjusted( m, m, -m + 1, -m + 1);

        QwtPainter::drawFocusRect( painter, this, focusRect );
    }
}

//! Redraw the text
void QwtTextLabel::drawText( QPainter* painter, const QRectF& textRect )
{
    m_data->text.draw( painter, textRect );
}

/*!
   Calculate geometry for the text in widget coordinates
   \return Geometry for the text
 */
QRect QwtTextLabel::textRect() const
{
    QRect r = contentsRect();

    if ( !r.isEmpty() && m_data->margin > 0 )
    {
        const int m = m_data->margin;
        r.adjust( m, m, -m, -m );
    }

    if ( !r.isEmpty() )
    {
        int indent = m_data->indent;
        if ( indent <= 0 )
            indent = defaultIndent();

        if ( indent > 0 )
        {
            const int renderFlags = m_data->text.renderFlags();

            if ( renderFlags & Qt::AlignLeft )
            {
                r.setX( r.x() + indent );
            }
            else if ( renderFlags & Qt::AlignRight )
            {
                r.setWidth( r.width() - indent );
            }
            else if ( renderFlags & Qt::AlignTop )
            {
                r.setY( r.y() + indent );
            }
            else if ( renderFlags & Qt::AlignBottom )
            {
                r.setHeight( r.height() - indent );
            }
        }
    }

    return r;
}

int QwtTextLabel::defaultIndent() const
{
    if ( frameWidth() <= 0 )
        return 0;

    QFont fnt;
    if ( m_data->text.testPaintAttribute( QwtText::PaintUsingTextFont ) )
        fnt = m_data->text.font();
    else
        fnt = font();

    return QwtPainter::horizontalAdvance( QFontMetrics( fnt ), 'x' ) / 2;
}

#include "moc_qwt_text_label.cpp"
