/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include "qwt_graphic.h"

#include <qapplication.h>
#include <qscrollbar.h>
#include <qscrollarea.h>
#include <qpainter.h>
#include <qmargins.h>

namespace
{
    class LegendMap
    {
      public:
        inline bool isEmpty() const { return m_entries.isEmpty(); }

        void insert( const QVariant&, const QList< QWidget* >& );
        void remove( const QVariant& );

        void removeWidget( const QWidget* );

        QList< QWidget* > legendWidgets( const QVariant& ) const;
        QVariant itemInfo( const QWidget* ) const;

      private:
        // we don't know anything about itemInfo and therefore don't have
        // any key that can be used for a map or hashtab.
        // But a simple linear list is o.k. here, as we will never have
        // more than a few entries.

        class Entry
        {
          public:
            QVariant itemInfo;
            QList< QWidget* > widgets;
        };

        QList< Entry > m_entries;
    };

    void LegendMap::insert( const QVariant& itemInfo,
        const QList< QWidget* >& widgets )
    {
        for ( int i = 0; i < m_entries.size(); i++ )
        {
            Entry& entry = m_entries[i];
            if ( entry.itemInfo == itemInfo )
            {
                entry.widgets = widgets;
                return;
            }
        }

        Entry newEntry;
        newEntry.itemInfo = itemInfo;
        newEntry.widgets = widgets;

        m_entries += newEntry;
    }

    void LegendMap::remove( const QVariant& itemInfo )
    {
        for ( int i = 0; i < m_entries.size(); i++ )
        {
            Entry& entry = m_entries[i];
            if ( entry.itemInfo == itemInfo )
            {
                m_entries.removeAt( i );
                return;
            }
        }
    }

    void LegendMap::removeWidget( const QWidget* widget )
    {
        QWidget* w = const_cast< QWidget* >( widget );

        for ( int i = 0; i < m_entries.size(); i++ )
            m_entries[ i ].widgets.removeAll( w );
    }

    QVariant LegendMap::itemInfo( const QWidget* widget ) const
    {
        if ( widget != NULL )
        {
            QWidget* w = const_cast< QWidget* >( widget );

            for ( int i = 0; i < m_entries.size(); i++ )
            {
                const Entry& entry = m_entries[i];
                if ( entry.widgets.indexOf( w ) >= 0 )
                    return entry.itemInfo;
            }
        }

        return QVariant();
    }

    QList< QWidget* > LegendMap::legendWidgets( const QVariant& itemInfo ) const
    {
        if ( itemInfo.isValid() )
        {
            for ( int i = 0; i < m_entries.size(); i++ )
            {
                const Entry& entry = m_entries[i];
                if ( entry.itemInfo == itemInfo )
                    return entry.widgets;
            }
        }

        return QList< QWidget* >();
    }
}

class QwtLegend::PrivateData
{
  public:
    PrivateData()
        : itemMode( QwtLegendData::ReadOnly )
        , view( NULL )
    {
    }

    QwtLegendData::Mode itemMode;
    LegendMap itemMap;

    class LegendView;
    LegendView* view;
};

class QwtLegend::PrivateData::LegendView QWT_FINAL : public QScrollArea
{
  public:
    explicit LegendView( QWidget * parent ) :
        QScrollArea( parent )
    {
        contentsWidget = new QWidget( this );
        contentsWidget->setObjectName( "QwtLegendViewContents" );

        setWidget( contentsWidget );
        setWidgetResizable( false );

        viewport()->setObjectName( "QwtLegendViewport" );

        // QScrollArea::setWidget internally sets autoFillBackground to true
        // But we don't want a background.
        contentsWidget->setAutoFillBackground( false );
        viewport()->setAutoFillBackground( false );
    }

    virtual bool event( QEvent* event ) QWT_OVERRIDE
    {
        if ( event->type() == QEvent::PolishRequest )
        {
            setFocusPolicy( Qt::NoFocus );
        }

        if ( event->type() == QEvent::Resize )
        {
            // adjust the size to en/disable the scrollbars
            // before QScrollArea adjusts the viewport size

            const QRect cr = contentsRect();

            int w = cr.width();
            int h = contentsWidget->heightForWidth( cr.width() );
            if ( h > w )
            {
                w -= verticalScrollBar()->sizeHint().width();
                h = contentsWidget->heightForWidth( w );
            }

            contentsWidget->resize( w, h );
        }

        return QScrollArea::event( event );
    }

    virtual bool viewportEvent( QEvent* event ) QWT_OVERRIDE
    {
        bool ok = QScrollArea::viewportEvent( event );

        if ( event->type() == QEvent::Resize )
        {
            layoutContents();
        }
        return ok;
    }

    QSize viewportSize( int w, int h ) const
    {
        const int sbHeight = horizontalScrollBar()->sizeHint().height();
        const int sbWidth = verticalScrollBar()->sizeHint().width();

        const int cw = contentsRect().width();
        const int ch = contentsRect().height();

        int vw = cw;
        int vh = ch;

        if ( w > vw )
            vh -= sbHeight;

        if ( h > vh )
        {
            vw -= sbWidth;
            if ( w > vw && vh == ch )
                vh -= sbHeight;
        }
        return QSize( vw, vh );
    }

    void layoutContents()
    {
        const QwtDynGridLayout* tl = qobject_cast< QwtDynGridLayout* >(
            contentsWidget->layout() );
        if ( tl == NULL )
            return;

        const QSize visibleSize = viewport()->contentsRect().size();

        const QMargins m = tl->contentsMargins();
        const int minW = tl->maxItemWidth() + m.left() + m.right();

        int w = qMax( visibleSize.width(), minW );
        int h = qMax( tl->heightForWidth( w ), visibleSize.height() );

        const int vpWidth = viewportSize( w, h ).width();
        if ( w > vpWidth )
        {
            w = qMax( vpWidth, minW );
            h = qMax( tl->heightForWidth( w ), visibleSize.height() );
        }

        contentsWidget->resize( w, h );
    }

    QWidget* contentsWidget;
};

/*!
   Constructor
   \param parent Parent widget
 */
QwtLegend::QwtLegend( QWidget* parent )
    : QwtAbstractLegend( parent )
{
    setFrameStyle( NoFrame );

    m_data = new QwtLegend::PrivateData;

    m_data->view = new QwtLegend::PrivateData::LegendView( this );
    m_data->view->setObjectName( "QwtLegendView" );
    m_data->view->setFrameStyle( NoFrame );

    QwtDynGridLayout* gridLayout = new QwtDynGridLayout(
        m_data->view->contentsWidget );
    gridLayout->setAlignment( Qt::AlignHCenter | Qt::AlignTop );

    m_data->view->contentsWidget->installEventFilter( this );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( m_data->view );
}

//! Destructor
QwtLegend::~QwtLegend()
{
    delete m_data;
}

/*!
   \brief Set the maximum number of entries in a row

   F.e when the maximum is set to 1 all items are aligned
   vertically. 0 means unlimited

   \param numColums Maximum number of entries in a row

   \sa maxColumns(), QwtDynGridLayout::setMaxColumns()
 */
void QwtLegend::setMaxColumns( uint numColums )
{
    QwtDynGridLayout* tl = qobject_cast< QwtDynGridLayout* >(
        m_data->view->contentsWidget->layout() );
    if ( tl )
        tl->setMaxColumns( numColums );

    updateGeometry();
}

/*!
   \return Maximum number of entries in a row
   \sa setMaxColumns(), QwtDynGridLayout::maxColumns()
 */
uint QwtLegend::maxColumns() const
{
    uint maxCols = 0;

    const QwtDynGridLayout* tl = qobject_cast< const QwtDynGridLayout* >(
        m_data->view->contentsWidget->layout() );
    if ( tl )
        maxCols = tl->maxColumns();

    return maxCols;
}

/*!
   \brief Set the default mode for legend labels

   Legend labels will be constructed according to the
   attributes in a QwtLegendData object. When it doesn't
   contain a value for the QwtLegendData::ModeRole the
   label will be initialized with the default mode of the legend.

   \param mode Default item mode

   \sa itemMode(), QwtLegendData::value(), QwtPlotItem::legendData()
   \note Changing the mode doesn't have any effect on existing labels.
 */
void QwtLegend::setDefaultItemMode( QwtLegendData::Mode mode )
{
    m_data->itemMode = mode;
}

/*!
   \return Default item mode
   \sa setDefaultItemMode()
 */
QwtLegendData::Mode QwtLegend::defaultItemMode() const
{
    return m_data->itemMode;
}

/*!
   The contents widget is the only child of the viewport of
   the internal QScrollArea and the parent widget of all legend items.

   \return Container widget of the legend items
 */
QWidget* QwtLegend::contentsWidget()
{
    return m_data->view->contentsWidget;
}

/*!
   \return Horizontal scrollbar
   \sa verticalScrollBar()
 */
QScrollBar* QwtLegend::horizontalScrollBar() const
{
    return m_data->view->horizontalScrollBar();
}

/*!
   \return Vertical scrollbar
   \sa horizontalScrollBar()
 */
QScrollBar* QwtLegend::verticalScrollBar() const
{
    return m_data->view->verticalScrollBar();
}

/*!
   The contents widget is the only child of the viewport of
   the internal QScrollArea and the parent widget of all legend items.

   \return Container widget of the legend items

 */
const QWidget* QwtLegend::contentsWidget() const
{
    return m_data->view->contentsWidget;
}

/*!
   \brief Update the entries for an item

   \param itemInfo Info for an item
   \param legendData List of legend entry attributes for the item
 */
void QwtLegend::updateLegend( const QVariant& itemInfo,
    const QList< QwtLegendData >& legendData )
{
    QList< QWidget* > widgetList = legendWidgets( itemInfo );

    if ( widgetList.size() != legendData.size() )
    {
        QLayout* contentsLayout = m_data->view->contentsWidget->layout();

        while ( widgetList.size() > legendData.size() )
        {
            QWidget* w = widgetList.takeLast();

            contentsLayout->removeWidget( w );

            // updates might be triggered by signals from the legend widget
            // itself. So we better don't delete it here.

            w->hide();
            w->deleteLater();
        }

        widgetList.reserve( legendData.size() );

        for ( int i = widgetList.size(); i < legendData.size(); i++ )
        {
            QWidget* widget = createWidget( legendData[i] );

            if ( contentsLayout )
                contentsLayout->addWidget( widget );

            if ( isVisible() )
            {
                // QLayout does a delayed show, with the effect, that
                // the size hint will be wrong, when applications
                // call replot() right after changing the list
                // of plot items. So we better do the show now.

                widget->setVisible( true );
            }

            widgetList += widget;
        }

        if ( widgetList.isEmpty() )
        {
            m_data->itemMap.remove( itemInfo );
        }
        else
        {
            m_data->itemMap.insert( itemInfo, widgetList );
        }

        updateTabOrder();
    }

    for ( int i = 0; i < legendData.size(); i++ )
        updateWidget( widgetList[i], legendData[i] );
}

/*!
   \brief Create a widget to be inserted into the legend

   The default implementation returns a QwtLegendLabel.

   \param legendData Attributes of the legend entry
   \return Widget representing data on the legend

   \note updateWidget() will called soon after createWidget()
        with the same attributes.
 */
QWidget* QwtLegend::createWidget( const QwtLegendData& legendData ) const
{
    Q_UNUSED( legendData );

    QwtLegendLabel* label = new QwtLegendLabel();
    label->setItemMode( defaultItemMode() );

    connect( label, SIGNAL(clicked()), SLOT(itemClicked()) );
    connect( label, SIGNAL(checked(bool)), SLOT(itemChecked(bool)) );

    return label;
}

/*!
   \brief Update the widget

   \param widget Usually a QwtLegendLabel
   \param legendData Attributes to be displayed

   \sa createWidget()
   \note When widget is no QwtLegendLabel updateWidget() does nothing.
 */
void QwtLegend::updateWidget( QWidget* widget, const QwtLegendData& legendData )
{
    QwtLegendLabel* label = qobject_cast< QwtLegendLabel* >( widget );
    if ( label )
    {
        label->setData( legendData );
        if ( !legendData.value( QwtLegendData::ModeRole ).isValid() )
        {
            // use the default mode, when there is no specific
            // hint from the legend data

            label->setItemMode( defaultItemMode() );
        }
    }
}

void QwtLegend::updateTabOrder()
{
    QLayout* contentsLayout = m_data->view->contentsWidget->layout();
    if ( contentsLayout )
    {
        // set tab focus chain

        QWidget* w = NULL;

        for ( int i = 0; i < contentsLayout->count(); i++ )
        {
            QLayoutItem* item = contentsLayout->itemAt( i );
            if ( w && item->widget() )
                QWidget::setTabOrder( w, item->widget() );

            w = item->widget();
        }
    }
}

//! Return a size hint.
QSize QwtLegend::sizeHint() const
{
    QSize hint = m_data->view->contentsWidget->sizeHint();
    hint += QSize( 2 * frameWidth(), 2 * frameWidth() );

    return hint;
}

/*!
   \return The preferred height, for a width.
   \param width Width
 */
int QwtLegend::heightForWidth( int width ) const
{
    width -= 2 * frameWidth();

    int h = m_data->view->contentsWidget->heightForWidth( width );
    if ( h >= 0 )
        h += 2 * frameWidth();

    return h;
}


/*!
   Handle QEvent::ChildRemoved and QEvent::LayoutRequest events
   for the contentsWidget().

   \param object Object to be filtered
   \param event Event

   \return Forwarded to QwtAbstractLegend::eventFilter()
 */
bool QwtLegend::eventFilter( QObject* object, QEvent* event )
{
    if ( object == m_data->view->contentsWidget )
    {
        switch ( event->type() )
        {
            case QEvent::ChildRemoved:
            {
                const QChildEvent* ce =
                    static_cast< const QChildEvent* >( event );

                if ( ce->child()->isWidgetType() )
                {
                    /*
                        We are called from the ~QObject and ce->child() is
                        no widget anymore. But all we need is the address
                        to remove it from the map.
                     */
                    QWidget* w = reinterpret_cast< QWidget* >( ce->child() );
                    m_data->itemMap.removeWidget( w );
                }
                break;
            }
            case QEvent::LayoutRequest:
            {
                m_data->view->layoutContents();

                if ( parentWidget() && parentWidget()->layout() == NULL )
                {
                    /*
                       We want the parent widget ( usually QwtPlot ) to recalculate
                       its layout, when the contentsWidget has changed. But
                       because of the scroll view we have to forward the LayoutRequest
                       event manually.

                       We don't use updateGeometry() because it doesn't post LayoutRequest
                       events when the legend is hidden. But we want the
                       parent widget notified, so it can show/hide the legend
                       depending on its items.
                     */
                    QApplication::postEvent( parentWidget(),
                        new QEvent( QEvent::LayoutRequest ) );
                }
                break;
            }
            default:
                break;
        }
    }

    return QwtAbstractLegend::eventFilter( object, event );
}

/*!
   Called internally when the legend has been clicked on.
   Emits a clicked() signal.
 */
void QwtLegend::itemClicked()
{
    QWidget* w = qobject_cast< QWidget* >( sender() );
    if ( w )
    {
        const QVariant itemInfo = m_data->itemMap.itemInfo( w );
        if ( itemInfo.isValid() )
        {
            const QList< QWidget* > widgetList =
                m_data->itemMap.legendWidgets( itemInfo );

            const int index = widgetList.indexOf( w );
            if ( index >= 0 )
                Q_EMIT clicked( itemInfo, index );
        }
    }
}

/*!
   Called internally when the legend has been checked
   Emits a checked() signal.
 */
void QwtLegend::itemChecked( bool on )
{
    QWidget* w = qobject_cast< QWidget* >( sender() );
    if ( w )
    {
        const QVariant itemInfo = m_data->itemMap.itemInfo( w );
        if ( itemInfo.isValid() )
        {
            const QList< QWidget* > widgetList =
                m_data->itemMap.legendWidgets( itemInfo );

            const int index = widgetList.indexOf( w );
            if ( index >= 0 )
                Q_EMIT checked( itemInfo, on, index );
        }
    }
}

/*!
   Render the legend into a given rectangle.

   \param painter Painter
   \param rect Bounding rectangle
   \param fillBackground When true, fill rect with the widget background

   \sa renderLegend() is used by QwtPlotRenderer - not by QwtLegend itself
 */
void QwtLegend::renderLegend( QPainter* painter,
    const QRectF& rect, bool fillBackground ) const
{
    if ( m_data->itemMap.isEmpty() )
        return;

    if ( fillBackground )
    {
        if ( autoFillBackground() ||
            testAttribute( Qt::WA_StyledBackground ) )
        {
            QwtPainter::drawBackgound( painter, rect, this );
        }
    }

    const QwtDynGridLayout* legendLayout =
        qobject_cast< QwtDynGridLayout* >( contentsWidget()->layout() );
    if ( legendLayout == NULL )
        return;

    const QMargins m = contentsMargins();

    QRect layoutRect;
    layoutRect.setLeft( qwtCeil( rect.left() ) + m.left() );
    layoutRect.setTop( qwtCeil( rect.top() ) + m.top() );
    layoutRect.setRight( qwtFloor( rect.right() ) - m.right() );
    layoutRect.setBottom( qwtFloor( rect.bottom() ) - m.bottom() );

    uint numCols = legendLayout->columnsForWidth( layoutRect.width() );
    const QList< QRect > itemRects =
        legendLayout->layoutItems( layoutRect, numCols );

    int index = 0;

    for ( int i = 0; i < legendLayout->count(); i++ )
    {
        QLayoutItem* item = legendLayout->itemAt( i );
        QWidget* w = item->widget();
        if ( w )
        {
            painter->save();

            painter->setClipRect( itemRects[index], Qt::IntersectClip );
            renderItem( painter, w, itemRects[index], fillBackground );

            index++;
            painter->restore();
        }
    }
}

/*!
   Render a legend entry into a given rectangle.

   \param painter Painter
   \param widget Widget representing a legend entry
   \param rect Bounding rectangle
   \param fillBackground When true, fill rect with the widget background

   \note When widget is not derived from QwtLegendLabel renderItem
        does nothing beside the background
 */
void QwtLegend::renderItem( QPainter* painter,
    const QWidget* widget, const QRectF& rect, bool fillBackground ) const
{
    if ( fillBackground )
    {
        if ( widget->autoFillBackground() ||
            widget->testAttribute( Qt::WA_StyledBackground ) )
        {
            QwtPainter::drawBackgound( painter, rect, widget );
        }
    }

    const QwtLegendLabel* label = qobject_cast< const QwtLegendLabel* >( widget );
    if ( label )
    {
        // icon

        const QwtGraphic& icon = label->data().icon();
        const QSizeF sz = icon.defaultSize();

        const QRectF iconRect( rect.x() + label->margin(),
            rect.center().y() - 0.5 * sz.height(),
            sz.width(), sz.height() );

        icon.render( painter, iconRect, Qt::KeepAspectRatio );

        // title

        QRectF titleRect = rect;
        titleRect.setX( iconRect.right() + 2 * label->spacing() );

        QFont labelFont = label->font();
#if QT_VERSION >= 0x060000
        labelFont.setResolveMask( QFont::AllPropertiesResolved );
#else
        labelFont.resolve( QFont::AllPropertiesResolved );
#endif

        painter->setFont( labelFont );
        painter->setPen( label->palette().color( QPalette::Text ) );

        const_cast< QwtLegendLabel* >( label )->drawText( painter, titleRect );
    }
}

/*!
   \return List of widgets associated to a item
   \param itemInfo Info about an item
   \sa legendWidget(), itemInfo(), QwtPlot::itemToInfo()
 */
QList< QWidget* > QwtLegend::legendWidgets( const QVariant& itemInfo ) const
{
    return m_data->itemMap.legendWidgets( itemInfo );
}

/*!
   \return First widget in the list of widgets associated to an item
   \param itemInfo Info about an item
   \sa itemInfo(), QwtPlot::itemToInfo()
   \note Almost all types of items have only one widget
 */
QWidget* QwtLegend::legendWidget( const QVariant& itemInfo ) const
{
    const QList< QWidget* > list = m_data->itemMap.legendWidgets( itemInfo );
    if ( list.isEmpty() )
        return NULL;

    return list[0];
}

/*!
   Find the item that is associated to a widget

   \param widget Widget on the legend
   \return Associated item info
   \sa legendWidget()
 */
QVariant QwtLegend::itemInfo( const QWidget* widget ) const
{
    return m_data->itemMap.itemInfo( widget );
}

//! \return True, when no item is inserted
bool QwtLegend::isEmpty() const
{
    return m_data->itemMap.isEmpty();
}

/*!
    Return the extent, that is needed for the scrollbars

    \param orientation Orientation
    \return The width of the vertical scrollbar for Qt::Horizontal and v.v.
 */
int QwtLegend::scrollExtent( Qt::Orientation orientation ) const
{
    int extent = 0;

    if ( orientation == Qt::Horizontal )
        extent = verticalScrollBar()->sizeHint().width();
    else
        extent = horizontalScrollBar()->sizeHint().height();

    return extent;
}

#include "moc_qwt_legend.cpp"
