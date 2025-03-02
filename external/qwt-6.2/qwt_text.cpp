/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_text.h"
#include "qwt_painter.h"
#include "qwt_text_engine.h"
#include "qwt_math.h"

#include <qmap.h>
#include <qfont.h>
#include <qcolor.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>

#if QT_VERSION >= 0x050200

static QwtText qwtStringToText( const QString& text )
{
    return QwtText( text );
}

#endif

namespace
{
    static const struct RegisterQwtText
    {
        inline RegisterQwtText()
        {
            qRegisterMetaType< QwtText >();

#if QT_VERSION >= 0x050200
            QMetaType::registerConverter< QString, QwtText >( qwtStringToText );
#endif
        }

    } qwtRegisterQwtText;
}

namespace
{
    class TextEngineDict
    {
      public:
        static TextEngineDict& dict();

        void setTextEngine( QwtText::TextFormat, QwtTextEngine* );

        const QwtTextEngine* textEngine( QwtText::TextFormat ) const;
        const QwtTextEngine* textEngine( const QString&,
            QwtText::TextFormat ) const;

      private:
        TextEngineDict();
        ~TextEngineDict();

        typedef QMap< int, QwtTextEngine* > EngineMap;

        inline const QwtTextEngine* engine( EngineMap::const_iterator& it ) const
        {
            return it.value();
        }

        EngineMap m_map;
    };

    TextEngineDict& TextEngineDict::dict()
    {
        static TextEngineDict engineDict;
        return engineDict;
    }

    TextEngineDict::TextEngineDict()
    {
        m_map.insert( QwtText::PlainText, new QwtPlainTextEngine() );
    #ifndef QT_NO_RICHTEXT
        m_map.insert( QwtText::RichText, new QwtRichTextEngine() );
    #endif
    }

    TextEngineDict::~TextEngineDict()
    {
        for ( EngineMap::const_iterator it = m_map.constBegin();
            it != m_map.constEnd(); ++it )
        {
            const QwtTextEngine* textEngine = engine( it );
            delete textEngine;
        }
    }

    const QwtTextEngine* TextEngineDict::textEngine( const QString& text,
        QwtText::TextFormat format ) const
    {
        if ( format == QwtText::AutoText )
        {
            for ( EngineMap::const_iterator it = m_map.begin();
                it != m_map.end(); ++it )
            {
                if ( it.key() != QwtText::PlainText )
                {
                    const QwtTextEngine* e = engine( it );
                    if ( e && e->mightRender( text ) )
                        return e;
                }
            }
        }

        EngineMap::const_iterator it = m_map.find( format );
        if ( it != m_map.end() )
        {
            const QwtTextEngine* e = engine( it );
            if ( e )
                return e;
        }

        it = m_map.find( QwtText::PlainText );
        return engine( it );
    }

    void TextEngineDict::setTextEngine( QwtText::TextFormat format,
        QwtTextEngine* engine )
    {
        if ( format == QwtText::AutoText )
            return;

        if ( format == QwtText::PlainText && engine == NULL )
            return;

        EngineMap::const_iterator it = m_map.constFind( format );
        if ( it != m_map.constEnd() )
        {
            delete this->engine( it );
            m_map.remove( format );
        }

        if ( engine != NULL )
            m_map.insert( format, engine );
    }

    const QwtTextEngine* TextEngineDict::textEngine(
        QwtText::TextFormat format ) const
    {
        const QwtTextEngine* e = NULL;

        EngineMap::const_iterator it = m_map.find( format );
        if ( it != m_map.end() )
            e = engine( it );

        return e;
    }
}

class QwtText::PrivateData
{
  public:
    PrivateData():
        renderFlags( Qt::AlignCenter ),
        borderRadius( 0 ),
        borderPen( Qt::NoPen ),
        backgroundBrush( Qt::NoBrush ),
        textEngine( NULL )
    {
    }

    int renderFlags;
    QString text;
    QFont font;
    QColor color;
    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;

    QwtText::PaintAttributes paintAttributes;
    QwtText::LayoutAttributes layoutAttributes;

    const QwtTextEngine* textEngine;
};

class QwtText::LayoutCache
{
  public:
    void invalidate()
    {
        textSize = QSizeF();
    }

    QFont font;
    QSizeF textSize;
};

/*!
   Constructor
 */
QwtText::QwtText()
{
    m_data = new PrivateData;
    m_data->textEngine = textEngine( m_data->text, PlainText );

    m_layoutCache = new LayoutCache;
}

/*!
   Constructor

   \param text Text content
   \param textFormat Text format
 */
QwtText::QwtText( const QString& text, QwtText::TextFormat textFormat )
{
    m_data = new PrivateData;
    m_data->text = text;
    m_data->textEngine = textEngine( text, textFormat );

    m_layoutCache = new LayoutCache;
}

//! Copy constructor
QwtText::QwtText( const QwtText& other )
{
    m_data = new PrivateData;
    *m_data = *other.m_data;

    m_layoutCache = new LayoutCache;
    *m_layoutCache = *other.m_layoutCache;
}

//! Destructor
QwtText::~QwtText()
{
    delete m_data;
    delete m_layoutCache;
}

//! Assignment operator
QwtText& QwtText::operator=( const QwtText& other )
{
    *m_data = *other.m_data;
    *m_layoutCache = *other.m_layoutCache;
    return *this;
}

//! Relational operator
bool QwtText::operator==( const QwtText& other ) const
{
    return m_data->renderFlags == other.m_data->renderFlags &&
           m_data->text == other.m_data->text &&
           m_data->font == other.m_data->font &&
           m_data->color == other.m_data->color &&
           m_data->borderRadius == other.m_data->borderRadius &&
           m_data->borderPen == other.m_data->borderPen &&
           m_data->backgroundBrush == other.m_data->backgroundBrush &&
           m_data->paintAttributes == other.m_data->paintAttributes &&
           m_data->textEngine == other.m_data->textEngine;
}

//! Relational operator
bool QwtText::operator!=( const QwtText& other ) const // invalidate
{
    return !( other == *this );
}

/*!
   Assign a new text content

   \param text Text content
   \param textFormat Text format

   \sa text()
 */
void QwtText::setText( const QString& text,
    QwtText::TextFormat textFormat )
{
    m_data->text = text;
    m_data->textEngine = textEngine( text, textFormat );
    m_layoutCache->invalidate();
}

/*!
   \return Text as QString.
   \sa setText()
 */
QString QwtText::text() const
{
    return m_data->text;
}

/*!
   \brief Change the render flags

   The default setting is Qt::AlignCenter

   \param renderFlags Bitwise OR of the flags used like in QPainter::drawText()

   \sa renderFlags(), QwtTextEngine::draw()
   \note Some renderFlags might have no effect, depending on the text format.
 */
void QwtText::setRenderFlags( int renderFlags )
{
    if ( renderFlags != m_data->renderFlags )
    {
        m_data->renderFlags = renderFlags;
        m_layoutCache->invalidate();
    }
}

/*!
   \return Render flags
   \sa setRenderFlags()
 */
int QwtText::renderFlags() const
{
    return m_data->renderFlags;
}

/*!
   Set the font.

   \param font Font
   \note Setting the font might have no effect, when
         the text contains control sequences for setting fonts.
 */
void QwtText::setFont( const QFont& font )
{
    m_data->font = font;
    setPaintAttribute( PaintUsingTextFont );
}

//! Return the font.
QFont QwtText::font() const
{
    return m_data->font;
}

/*!
   Return the font of the text, if it has one.
   Otherwise return defaultFont.

   \param defaultFont Default font
   \return Font used for drawing the text

   \sa setFont(), font(), PaintAttributes
 */
QFont QwtText::usedFont( const QFont& defaultFont ) const
{
    if ( m_data->paintAttributes & PaintUsingTextFont )
        return m_data->font;

    return defaultFont;
}

/*!
   Set the pen color used for drawing the text.

   \param color Color
   \note Setting the color might have no effect, when
         the text contains control sequences for setting colors.
 */
void QwtText::setColor( const QColor& color )
{
    m_data->color = color;
    setPaintAttribute( PaintUsingTextColor );
}

//! Return the pen color, used for painting the text
QColor QwtText::color() const
{
    return m_data->color;
}

/*!
   Return the color of the text, if it has one.
   Otherwise return defaultColor.

   \param defaultColor Default color
   \return Color used for drawing the text

   \sa setColor(), color(), PaintAttributes
 */
QColor QwtText::usedColor( const QColor& defaultColor ) const
{
    if ( m_data->paintAttributes & PaintUsingTextColor )
        return m_data->color;

    return defaultColor;
}

/*!
   Set the radius for the corners of the border frame

   \param radius Radius of a rounded corner
   \sa borderRadius(), setBorderPen(), setBackgroundBrush()
 */
void QwtText::setBorderRadius( double radius )
{
    m_data->borderRadius = qwtMaxF( 0.0, radius );
}

/*!
   \return Radius for the corners of the border frame
   \sa setBorderRadius(), borderPen(), backgroundBrush()
 */
double QwtText::borderRadius() const
{
    return m_data->borderRadius;
}

/*!
   Set the background pen

   \param pen Background pen
   \sa borderPen(), setBackgroundBrush()
 */
void QwtText::setBorderPen( const QPen& pen )
{
    m_data->borderPen = pen;
    setPaintAttribute( PaintBackground );
}

/*!
   \return Background pen
   \sa setBorderPen(), backgroundBrush()
 */
QPen QwtText::borderPen() const
{
    return m_data->borderPen;
}

/*!
   Set the background brush

   \param brush Background brush
   \sa backgroundBrush(), setBorderPen()
 */
void QwtText::setBackgroundBrush( const QBrush& brush )
{
    m_data->backgroundBrush = brush;
    setPaintAttribute( PaintBackground );
}

/*!
   \return Background brush
   \sa setBackgroundBrush(), borderPen()
 */
QBrush QwtText::backgroundBrush() const
{
    return m_data->backgroundBrush;
}

/*!
   Change a paint attribute

   \param attribute Paint attribute
   \param on On/Off

   \note Used by setFont(), setColor(),
         setBorderPen() and setBackgroundBrush()
   \sa testPaintAttribute()
 */
void QwtText::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/*!
   Test a paint attribute

   \param attribute Paint attribute
   \return true, if attribute is enabled

   \sa setPaintAttribute()
 */
bool QwtText::testPaintAttribute( PaintAttribute attribute ) const
{
    return m_data->paintAttributes & attribute;
}

/*!
   Change a layout attribute

   \param attribute Layout attribute
   \param on On/Off
   \sa testLayoutAttribute()
 */
void QwtText::setLayoutAttribute( LayoutAttribute attribute, bool on )
{
    if ( on )
        m_data->layoutAttributes |= attribute;
    else
        m_data->layoutAttributes &= ~attribute;
}

/*!
   Test a layout attribute

   \param attribute Layout attribute
   \return true, if attribute is enabled

   \sa setLayoutAttribute()
 */
bool QwtText::testLayoutAttribute( LayoutAttribute attribute ) const
{
    return m_data->layoutAttributes | attribute;
}

/*!
   Find the height for a given width

   \param width Width
   \return Calculated height
 */

double QwtText::heightForWidth( double width ) const
{
    return heightForWidth( width, QFont() );
}

/*!
   Find the height for a given width

   \param defaultFont Font, used for the calculation if the text has no font
   \param width Width

   \return Calculated height
 */
double QwtText::heightForWidth( double width, const QFont& defaultFont ) const
{
    // We want to calculate in screen metrics. So
    // we need a font that uses screen metrics

    const QFont font = QwtPainter::scaledFont( usedFont( defaultFont ) );

    double h = 0;

    if ( m_data->layoutAttributes & MinimumLayout )
    {
        double left, right, top, bottom;
        m_data->textEngine->textMargins( font, m_data->text,
            left, right, top, bottom );

        h = m_data->textEngine->heightForWidth(
            font, m_data->renderFlags, m_data->text,
            width + left + right );

        h -= top + bottom;
    }
    else
    {
        h = m_data->textEngine->heightForWidth(
            font, m_data->renderFlags, m_data->text, width );
    }

    return h;
}

/*!
   Returns the size, that is needed to render text

   \return Calculated size
 */
QSizeF QwtText::textSize() const
{
    return textSize( QFont() );
}

/*!
   Returns the size, that is needed to render text

   \param defaultFont Font of the text
   \return Calculated size
 */
QSizeF QwtText::textSize( const QFont& defaultFont ) const
{
    // We want to calculate in screen metrics. So
    // we need a font that uses screen metrics

    const QFont font = QwtPainter::scaledFont( usedFont( defaultFont ) );

    if ( !m_layoutCache->textSize.isValid()
        || m_layoutCache->font != font )
    {
        m_layoutCache->textSize = m_data->textEngine->textSize(
            font, m_data->renderFlags, m_data->text );
        m_layoutCache->font = font;
    }

    QSizeF sz = m_layoutCache->textSize;

    if ( m_data->layoutAttributes & MinimumLayout )
    {
        double left, right, top, bottom;
        m_data->textEngine->textMargins( font, m_data->text,
            left, right, top, bottom );
        sz -= QSizeF( left + right, top + bottom );
    }

    return sz;
}

/*!
   Draw a text into a rectangle

   \param painter Painter
   \param rect Rectangle
 */
void QwtText::draw( QPainter* painter, const QRectF& rect ) const
{
    if ( m_data->paintAttributes & PaintBackground )
    {
        if ( m_data->borderPen != Qt::NoPen ||
            m_data->backgroundBrush != Qt::NoBrush )
        {
            painter->save();

            painter->setPen( m_data->borderPen );
            painter->setBrush( m_data->backgroundBrush );

            if ( m_data->borderRadius == 0 )
            {
                QwtPainter::drawRect( painter, rect );
            }
            else
            {
                painter->setRenderHint( QPainter::Antialiasing, true );
                painter->drawRoundedRect( rect,
                    m_data->borderRadius, m_data->borderRadius );
            }

            painter->restore();
        }
    }

    painter->save();

    if ( m_data->paintAttributes & PaintUsingTextFont )
    {
        painter->setFont( m_data->font );
    }

    if ( m_data->paintAttributes & PaintUsingTextColor )
    {
        if ( m_data->color.isValid() )
            painter->setPen( m_data->color );
    }

    QRectF expandedRect = rect;
    if ( m_data->layoutAttributes & MinimumLayout )
    {
        // We want to calculate in screen metrics. So
        // we need a font that uses screen metrics

        const QFont font = QwtPainter::scaledFont( painter->font() );

        double left, right, top, bottom;
        m_data->textEngine->textMargins(
            font, m_data->text, left, right, top, bottom );

        expandedRect.setTop( rect.top() - top );
        expandedRect.setBottom( rect.bottom() + bottom );
        expandedRect.setLeft( rect.left() - left );
        expandedRect.setRight( rect.right() + right );
    }

    m_data->textEngine->draw( painter, expandedRect,
        m_data->renderFlags, m_data->text );

    painter->restore();
}

/*!
   Find the text engine for a text format

   In case of QwtText::AutoText the first text engine
   (beside QwtPlainTextEngine) is returned, where QwtTextEngine::mightRender
   returns true. If there is none QwtPlainTextEngine is returned.

   If no text engine is registered for the format QwtPlainTextEngine
   is returned.

   \param text Text, needed in case of AutoText
   \param format Text format

   \return Corresponding text engine
 */
const QwtTextEngine* QwtText::textEngine( const QString& text,
    QwtText::TextFormat format )
{
    return TextEngineDict::dict().textEngine( text, format );
}

/*!
   Assign/Replace a text engine for a text format

   With setTextEngine it is possible to extend Qwt with
   other types of text formats.

   For QwtText::PlainText it is not allowed to assign a engine == NULL.

   \param format Text format
   \param engine Text engine

   \warning Using QwtText::AutoText does nothing.
 */
void QwtText::setTextEngine( QwtText::TextFormat format,
    QwtTextEngine* engine )
{
    TextEngineDict::dict().setTextEngine( format, engine );
}

/*!
   \brief Find the text engine for a text format

   textEngine can be used to find out if a text format is supported.

   \param format Text format
   \return The text engine, or NULL if no engine is available.
 */
const QwtTextEngine* QwtText::textEngine( QwtText::TextFormat format )
{
    return TextEngineDict::dict().textEngine( format );
}

//! \return text().isNull()
bool QwtText::isNull() const
{
    return m_data->text.isNull();
}

//! \return text().isEmpty()
bool QwtText::isEmpty() const
{
    return m_data->text.isEmpty();
}

