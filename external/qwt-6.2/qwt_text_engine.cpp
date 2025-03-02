/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_text_engine.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qmap.h>
#include <qwidget.h>
#include <qtextobject.h>
#include <qtextdocument.h>
#include <qabstracttextdocumentlayout.h>

static QString taggedRichText( const QString& text, int flags )
{
    QString richText = text;

    // By default QSimpleRichText is Qt::AlignLeft
    if ( flags & Qt::AlignJustify )
    {
        richText.prepend( QLatin1String( "<div align=\"justify\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }
    else if ( flags & Qt::AlignRight )
    {
        richText.prepend( QLatin1String ( "<div align=\"right\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }
    else if ( flags & Qt::AlignHCenter )
    {
        richText.prepend( QLatin1String ( "<div align=\"center\">" ) );
        richText.append( QLatin1String ( "</div>" ) );
    }

    return richText;
}

namespace
{
    class QwtRichTextDocument : public QTextDocument
    {
      public:
        QwtRichTextDocument( const QString& text, int flags, const QFont& font )
        {
            setUndoRedoEnabled( false );
            setDefaultFont( font );
            setHtml( text );

            // make sure we have a document layout
            ( void )documentLayout();

            QTextOption option = defaultTextOption();
            if ( flags & Qt::TextWordWrap )
                option.setWrapMode( QTextOption::WordWrap );
            else
                option.setWrapMode( QTextOption::NoWrap );

            option.setAlignment( static_cast< Qt::Alignment >( flags ) );
            setDefaultTextOption( option );

            QTextFrame* root = rootFrame();
            QTextFrameFormat fm = root->frameFormat();
            fm.setBorder( 0 );
            fm.setMargin( 0 );
            fm.setPadding( 0 );
            fm.setBottomMargin( 0 );
            fm.setLeftMargin( 0 );
            root->setFrameFormat( fm );

            adjustSize();
        }
    };
}

class QwtPlainTextEngine::PrivateData
{
  public:
    int effectiveAscent( const QFont& font ) const
    {
        const QString fontKey = font.key();

        QMap< QString, int >::const_iterator it =
            m_ascentCache.constFind( fontKey );

        if ( it != m_ascentCache.constEnd() )
            return *it;

        const int ascent = findAscent( font );
        m_ascentCache.insert( fontKey, ascent );

        return ascent;
    }

  private:
    static int findAscent( const QFont& font )
    {
        static const QString dummy( "E" );
        static const QColor white( Qt::white );

        const QFontMetrics fm( font );

        QPixmap pm( QwtPainter::horizontalAdvance( fm, dummy ), fm.height() );
        pm.fill( white );

        QPainter p( &pm );
        p.setFont( font );
        p.drawText( 0, 0,  pm.width(), pm.height(), 0, dummy );
        p.end();

        const QImage img = pm.toImage();

        int row = 0;
        for ( row = 0; row < img.height(); row++ )
        {
            const QRgb* line = reinterpret_cast< const QRgb* >(
                img.scanLine( row ) );

            const int w = pm.width();
            for ( int col = 0; col < w; col++ )
            {
                if ( line[col] != white.rgb() )
                    return fm.ascent() - row + 1;
            }
        }

        return fm.ascent();
    }

    mutable QMap< QString, int > m_ascentCache;
};

//! Constructor
QwtTextEngine::QwtTextEngine()
{
}

//! Destructor
QwtTextEngine::~QwtTextEngine()
{
}

//! Constructor
QwtPlainTextEngine::QwtPlainTextEngine()
{
    m_data = new PrivateData;
}

//! Destructor
QwtPlainTextEngine::~QwtPlainTextEngine()
{
    delete m_data;
}

/*!
   Find the height for a given width

   \param font Font of the text
   \param flags Bitwise OR of the flags used like in QPainter::drawText
   \param text Text to be rendered
   \param width Width

   \return Calculated height
 */
double QwtPlainTextEngine::heightForWidth( const QFont& font, int flags,
    const QString& text, double width ) const
{
    const QFontMetricsF fm( font );
    const QRectF rect = fm.boundingRect(
        QRectF( 0, 0, width, QWIDGETSIZE_MAX ), flags, text );

    return rect.height();
}

/*!
   Returns the size, that is needed to render text

   \param font Font of the text
   \param flags Bitwise OR of the flags used like in QPainter::drawText
   \param text Text to be rendered

   \return Calculated size
 */
QSizeF QwtPlainTextEngine::textSize( const QFont& font,
    int flags, const QString& text ) const
{
    const QFontMetricsF fm( font );
    const QRectF rect = fm.boundingRect(
        QRectF( 0, 0, QWIDGETSIZE_MAX, QWIDGETSIZE_MAX ), flags, text );

    return rect.size();
}

/*!
   Return margins around the texts

   \param font Font of the text
   \param left Return 0
   \param right Return 0
   \param top Return value for the top margin
   \param bottom Return value for the bottom margin
 */
void QwtPlainTextEngine::textMargins( const QFont& font, const QString&,
    double& left, double& right, double& top, double& bottom ) const
{
    left = right = top = 0;

    const QFontMetricsF fm( font );
    top = fm.ascent() - m_data->effectiveAscent( font );
    bottom = fm.descent();
}

/*!
   \brief Draw the text in a clipping rectangle

   A wrapper for QPainter::drawText.

   \param painter Painter
   \param rect Clipping rectangle
   \param flags Bitwise OR of the flags used like in QPainter::drawText
   \param text Text to be rendered
 */
void QwtPlainTextEngine::draw( QPainter* painter, const QRectF& rect,
    int flags, const QString& text ) const
{
    QwtPainter::drawText( painter, rect, flags, text );
}

/*!
   Test if a string can be rendered by this text engine.
   \return Always true. All texts can be rendered by QwtPlainTextEngine
 */
bool QwtPlainTextEngine::mightRender( const QString& ) const
{
    return true;
}

#ifndef QT_NO_RICHTEXT

//! Constructor
QwtRichTextEngine::QwtRichTextEngine()
{
}

/*!
   Find the height for a given width

   \param font Font of the text
   \param flags Bitwise OR of the flags used like in QPainter::drawText()
   \param text Text to be rendered
   \param width Width

   \return Calculated height
 */
double QwtRichTextEngine::heightForWidth( const QFont& font, int flags,
    const QString& text, double width ) const
{
    QwtRichTextDocument doc( text, flags, font );

    doc.setPageSize( QSizeF( width, QWIDGETSIZE_MAX ) );
    return doc.documentLayout()->documentSize().height();
}

/*!
   Returns the size, that is needed to render text

   \param font Font of the text
   \param flags Bitwise OR of the flags used like in QPainter::drawText()
   \param text Text to be rendered

   \return Calculated size
 */

QSizeF QwtRichTextEngine::textSize( const QFont& font,
    int flags, const QString& text ) const
{
    QwtRichTextDocument doc( text, flags, font );

    QTextOption option = doc.defaultTextOption();
    if ( option.wrapMode() != QTextOption::NoWrap )
    {
        option.setWrapMode( QTextOption::NoWrap );
        doc.setDefaultTextOption( option );
        doc.adjustSize();
    }

    return doc.size();
}

/*!
   Draw the text in a clipping rectangle

   \param painter Painter
   \param rect Clipping rectangle
   \param flags Bitwise OR of the flags like in for QPainter::drawText()
   \param text Text to be rendered
 */
void QwtRichTextEngine::draw( QPainter* painter, const QRectF& rect,
    int flags, const QString& text ) const
{
    QwtRichTextDocument doc( text, flags, painter->font() );
    QwtPainter::drawSimpleRichText( painter, rect, flags, doc );
}

/*!
   Wrap text into <div align=...> </div> tags according flags

   \param text Text
   \param flags Bitwise OR of the flags like in for QPainter::drawText()

   \return Tagged text
 */
QString QwtRichTextEngine::taggedText( const QString& text, int flags ) const
{
    return taggedRichText( text, flags );
}

/*!
   Test if a string can be rendered by this text engine

   \param text Text to be tested
   \return Qt::mightBeRichText(text);
 */
bool QwtRichTextEngine::mightRender( const QString& text ) const
{
    return Qt::mightBeRichText( text );
}

/*!
   Return margins around the texts

   \param left Return 0
   \param right Return 0
   \param top Return 0
   \param bottom Return 0
 */
void QwtRichTextEngine::textMargins( const QFont&, const QString&,
    double& left, double& right, double& top, double& bottom ) const
{
    left = right = top = bottom = 0;
}

#endif // !QT_NO_RICHTEXT
