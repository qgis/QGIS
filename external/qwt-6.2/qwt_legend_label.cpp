/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_legend_label.h"
#include "qwt_legend_data.h"
#include "qwt_graphic.h"
#include "qwt.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qevent.h>
#include <qstyleoption.h>

static const int ButtonFrame = 2;
static const int Margin = 2;

static QSize buttonShift( const QwtLegendLabel* w )
{
    QStyleOption option;
    option.initFrom( w );

    const int ph = w->style()->pixelMetric(
        QStyle::PM_ButtonShiftHorizontal, &option, w );
    const int pv = w->style()->pixelMetric(
        QStyle::PM_ButtonShiftVertical, &option, w );
    return QSize( ph, pv );
}

class QwtLegendLabel::PrivateData
{
  public:
    PrivateData()
        : itemMode( QwtLegendData::ReadOnly )
        , isDown( false )
        , spacing( Margin )
    {
    }

    QwtLegendData::Mode itemMode;
    QwtLegendData legendData;
    bool isDown;

    QPixmap icon;

    int spacing;
};

/*!
   Set the attributes of the legend label

   \param legendData Attributes of the label
   \sa data()
 */
void QwtLegendLabel::setData( const QwtLegendData& legendData )
{
    m_data->legendData = legendData;

    const bool doUpdate = updatesEnabled();
    if ( doUpdate )
        setUpdatesEnabled( false );

    setText( legendData.title() );
    setIcon( legendData.icon().toPixmap() );

    if ( legendData.hasRole( QwtLegendData::ModeRole ) )
        setItemMode( legendData.mode() );

    if ( doUpdate )
        setUpdatesEnabled( true );
}

/*!
   \return Attributes of the label
   \sa setData(), QwtPlotItem::legendData()
 */
const QwtLegendData& QwtLegendLabel::data() const
{
    return m_data->legendData;
}

/*!
   \param parent Parent widget
 */
QwtLegendLabel::QwtLegendLabel( QWidget* parent )
    : QwtTextLabel( parent )
{
    m_data = new PrivateData;
    setMargin( Margin );
    setIndent( Margin );
}

//! Destructor
QwtLegendLabel::~QwtLegendLabel()
{
    delete m_data;
    m_data = NULL;
}

/*!
   Set the text to the legend item

   \param text Text label
    \sa QwtTextLabel::text()
 */
void QwtLegendLabel::setText( const QwtText& text )
{
    const int flags = Qt::AlignLeft | Qt::AlignVCenter
        | Qt::TextExpandTabs | Qt::TextWordWrap;

    QwtText txt = text;
    txt.setRenderFlags( flags );

    QwtTextLabel::setText( txt );
}

/*!
   Set the item mode
   The default is QwtLegendData::ReadOnly

   \param mode Item mode
   \sa itemMode()
 */
void QwtLegendLabel::setItemMode( QwtLegendData::Mode mode )
{
    if ( mode != m_data->itemMode )
    {
        m_data->itemMode = mode;
        m_data->isDown = false;

        setFocusPolicy( ( mode != QwtLegendData::ReadOnly )
            ? Qt::TabFocus : Qt::NoFocus );
        setMargin( ButtonFrame + Margin );

        updateGeometry();
    }
}

/*!
   \return Item mode
   \sa setItemMode()
 */
QwtLegendData::Mode QwtLegendLabel::itemMode() const
{
    return m_data->itemMode;
}

/*!
   Assign the icon

   \param icon Pixmap representing a plot item

   \sa icon(), QwtPlotItem::legendIcon()
 */
void QwtLegendLabel::setIcon( const QPixmap& icon )
{
    m_data->icon = icon;

    int indent = margin() + m_data->spacing;
    if ( icon.width() > 0 )
        indent += icon.width() + m_data->spacing;

    setIndent( indent );
}

/*!
   \return Pixmap representing a plot item
   \sa setIcon()
 */
QPixmap QwtLegendLabel::icon() const
{
    return m_data->icon;
}

/*!
   \brief Change the spacing between icon and text

   \param spacing Spacing
   \sa spacing(), QwtTextLabel::margin()
 */
void QwtLegendLabel::setSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != m_data->spacing )
    {
        m_data->spacing = spacing;

        int indent = margin() + m_data->spacing;
        if ( m_data->icon.width() > 0 )
            indent += m_data->icon.width() + m_data->spacing;

        setIndent( indent );
    }
}

/*!
   \return Spacing between icon and text
   \sa setSpacing(), QwtTextLabel::margin()
 */
int QwtLegendLabel::spacing() const
{
    return m_data->spacing;
}

/*!
    Check/Uncheck a the item

    \param on check/uncheck
    \sa setItemMode()
 */
void QwtLegendLabel::setChecked( bool on )
{
    if ( m_data->itemMode == QwtLegendData::Checkable )
    {
        const bool isBlocked = signalsBlocked();
        blockSignals( true );

        setDown( on );

        blockSignals( isBlocked );
    }
}

//! Return true, if the item is checked
bool QwtLegendLabel::isChecked() const
{
    return m_data->itemMode == QwtLegendData::Checkable && isDown();
}

//! Set the item being down
void QwtLegendLabel::setDown( bool down )
{
    if ( down == m_data->isDown )
        return;

    m_data->isDown = down;
    update();

    if ( m_data->itemMode == QwtLegendData::Clickable )
    {
        if ( m_data->isDown )
            Q_EMIT pressed();
        else
        {
            Q_EMIT released();
            Q_EMIT clicked();
        }
    }

    if ( m_data->itemMode == QwtLegendData::Checkable )
        Q_EMIT checked( m_data->isDown );
}

//! Return true, if the item is down
bool QwtLegendLabel::isDown() const
{
    return m_data->isDown;
}

//! Return a size hint
QSize QwtLegendLabel::sizeHint() const
{
    QSize sz = QwtTextLabel::sizeHint();
    sz.setHeight( qMax( sz.height(), m_data->icon.height() + 4 ) );

    if ( m_data->itemMode != QwtLegendData::ReadOnly )
    {
        sz += buttonShift( this );
        sz = qwtExpandedToGlobalStrut( sz );
    }

    return sz;
}

//! Paint event
void QwtLegendLabel::paintEvent( QPaintEvent* e )
{
    const QRect cr = contentsRect();

    QPainter painter( this );
    painter.setClipRegion( e->region() );

    if ( m_data->isDown )
    {
        qDrawWinButton( &painter, 0, 0, width(), height(),
            palette(), true );
    }

    painter.save();

    if ( m_data->isDown )
    {
        const QSize shiftSize = buttonShift( this );
        painter.translate( shiftSize.width(), shiftSize.height() );
    }

    painter.setClipRect( cr );

    drawContents( &painter );

    if ( !m_data->icon.isNull() )
    {
        QRect iconRect = cr;
        iconRect.setX( iconRect.x() + margin() );
        if ( m_data->itemMode != QwtLegendData::ReadOnly )
            iconRect.setX( iconRect.x() + ButtonFrame );

        iconRect.setSize( m_data->icon.size() );
        iconRect.moveCenter( QPoint( iconRect.center().x(), cr.center().y() ) );

        painter.drawPixmap( iconRect, m_data->icon );
    }

    painter.restore();
}

//! Handle mouse press events
void QwtLegendLabel::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
    {
        switch ( m_data->itemMode )
        {
            case QwtLegendData::Clickable:
            {
                setDown( true );
                return;
            }
            case QwtLegendData::Checkable:
            {
                setDown( !isDown() );
                return;
            }
            default:;
        }
    }
    QwtTextLabel::mousePressEvent( e );
}

//! Handle mouse release events
void QwtLegendLabel::mouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
    {
        switch ( m_data->itemMode )
        {
            case QwtLegendData::Clickable:
            {
                setDown( false );
                return;
            }
            case QwtLegendData::Checkable:
            {
                return; // do nothing, but accept
            }
            default:;
        }
    }
    QwtTextLabel::mouseReleaseEvent( e );
}

//! Handle key press events
void QwtLegendLabel::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Qt::Key_Space )
    {
        switch ( m_data->itemMode )
        {
            case QwtLegendData::Clickable:
            {
                if ( !e->isAutoRepeat() )
                    setDown( true );
                return;
            }
            case QwtLegendData::Checkable:
            {
                if ( !e->isAutoRepeat() )
                    setDown( !isDown() );
                return;
            }
            default:;
        }
    }

    QwtTextLabel::keyPressEvent( e );
}

//! Handle key release events
void QwtLegendLabel::keyReleaseEvent( QKeyEvent* e )
{
    if ( e->key() == Qt::Key_Space )
    {
        switch ( m_data->itemMode )
        {
            case QwtLegendData::Clickable:
            {
                if ( !e->isAutoRepeat() )
                    setDown( false );
                return;
            }
            case QwtLegendData::Checkable:
            {
                return; // do nothing, but accept
            }
            default:;
        }
    }

    QwtTextLabel::keyReleaseEvent( e );
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_legend_label.cpp"
#endif
