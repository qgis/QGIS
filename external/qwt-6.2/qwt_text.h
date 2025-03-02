/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_TEXT_H
#define QWT_TEXT_H

#include "qwt_global.h"
#include <qmetatype.h>

class QFont;
class QString;
class QColor;
class QPen;
class QBrush;
class QSizeF;
class QRectF;
class QPainter;
class QwtTextEngine;

/*!
   \brief A class representing a text

   A QwtText is a text including a set of attributes how to render it.

   - Format\n
    A text might include control sequences (f.e tags) describing
    how to render it. Each format (f.e MathML, TeX, Qt Rich Text)
    has its own set of control sequences, that can be handles by
    a special QwtTextEngine for this format.
   - Background\n
    A text might have a background, defined by a QPen and QBrush
    to improve its visibility. The corners of the background might
    be rounded.
   - Font\n
    A text might have an individual font.
   - Color\n
    A text might have an individual color.
   - Render Flags\n
    Flags from Qt::AlignmentFlag and Qt::TextFlag used like in
    QPainter::drawText().

   \sa QwtTextEngine, QwtTextLabel
 */

class QWT_EXPORT QwtText
{
  public:

    /*!
       \brief Text format

       The text format defines the QwtTextEngine, that is used to render
       the text.

       \sa QwtTextEngine, setTextEngine()
     */

    enum TextFormat
    {
        /*!
           The text format is determined using QwtTextEngine::mightRender() for
           all available text engines in increasing order > PlainText.
           If none of the text engines can render the text is rendered
           like QwtText::PlainText.
         */
        AutoText = 0,

        //! Draw the text as it is, using a QwtPlainTextEngine.
        PlainText,

        //! Use the Scribe framework (Qt Rich Text) to render the text.
        RichText,

        /*!
           Use a MathML (http://en.wikipedia.org/wiki/MathML) render engine
           to display the text. In earlier versions of Qwt such an engine
           was included - since Qwt 6.2 it can be found here:
           https://github.com/uwerat/qwt-mml-dev

           To enable MathML support the following code needs to be added to the
           application:

           \code
            QwtText::setTextEngine( QwtText::MathMLText, new QwtMathMLTextEngine() );
           \endcode
         */
        MathMLText,

        /*!
           Use a TeX (http://en.wikipedia.org/wiki/TeX) render engine
           to display the text ( not implemented yet ).
         */
        TeXText,

        /*!
           The number of text formats can be extended using setTextEngine.
           Formats >= QwtText::OtherFormat are not used by Qwt.
         */
        OtherFormat = 100
    };

    /*!
       \brief Paint Attributes

       Font and color and background are optional attributes of a QwtText.
       The paint attributes hold the information, if they are set.
     */
    enum PaintAttribute
    {
        //! The text has an individual font.
        PaintUsingTextFont = 0x01,

        //! The text has an individual color.
        PaintUsingTextColor = 0x02,

        //! The text has an individual background.
        PaintBackground = 0x04
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    /*!
       \brief Layout Attributes
       The layout attributes affects some aspects of the layout of the text.
     */
    enum LayoutAttribute
    {
        /*!
           Layout the text without its margins. This mode is useful if a
           text needs to be aligned accurately, like the tick labels of a scale.
           If QwtTextEngine::textMargins is not implemented for the format
           of the text, MinimumLayout has no effect.
         */
        MinimumLayout = 0x01
    };

    Q_DECLARE_FLAGS( LayoutAttributes, LayoutAttribute )

    QwtText();
    QwtText( const QString&, TextFormat textFormat = AutoText );
    QwtText( const QwtText& );

    ~QwtText();

    QwtText& operator=( const QwtText& );

    bool operator==( const QwtText& ) const;
    bool operator!=( const QwtText& ) const;

    void setText( const QString&,
        QwtText::TextFormat textFormat = AutoText );
    QString text() const;

    bool isNull() const;
    bool isEmpty() const;

    void setFont( const QFont& );
    QFont font() const;

    QFont usedFont( const QFont& ) const;

    void setRenderFlags( int );
    int renderFlags() const;

    void setColor( const QColor& );
    QColor color() const;

    QColor usedColor( const QColor& ) const;

    void setBorderRadius( double );
    double borderRadius() const;

    void setBorderPen( const QPen& );
    QPen borderPen() const;

    void setBackgroundBrush( const QBrush& );
    QBrush backgroundBrush() const;

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setLayoutAttribute( LayoutAttribute, bool on = true );
    bool testLayoutAttribute( LayoutAttribute ) const;

    double heightForWidth( double width ) const;
    double heightForWidth( double width, const QFont& ) const;

    QSizeF textSize() const;
    QSizeF textSize( const QFont& ) const;

    void draw( QPainter* painter, const QRectF& rect ) const;

    static const QwtTextEngine* textEngine(
        const QString& text, QwtText::TextFormat = AutoText );

    static const QwtTextEngine* textEngine( QwtText::TextFormat );
    static void setTextEngine( QwtText::TextFormat, QwtTextEngine* );

  private:
    class PrivateData;
    PrivateData* m_data;

    class LayoutCache;
    LayoutCache* m_layoutCache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtText::PaintAttributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtText::LayoutAttributes )

Q_DECLARE_METATYPE( QwtText )

#endif
