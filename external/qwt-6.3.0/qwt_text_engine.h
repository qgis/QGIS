/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_TEXT_ENGINE_H
#define QWT_TEXT_ENGINE_H

#include "qwt_global.h"
#include <qsize.h>

class QFont;
class QRectF;
class QString;
class QPainter;

/*!
   \brief Abstract base class for rendering text strings

   A text engine is responsible for rendering texts for a
   specific text format. They are used by QwtText to render a text.

   \sa QwtText::setTextEngine()
 */

class QWT_EXPORT QwtTextEngine
{
  public:
    virtual ~QwtTextEngine();

    /*!
       Find the height for a given width

       \param font Font of the text
       \param flags Bitwise OR of the flags used like in QPainter::drawText
       \param text Text to be rendered
       \param width Width

       \return Calculated height
     */
    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const = 0;

    /*!
       Returns the size, that is needed to render text

       \param font Font of the text
       \param flags Bitwise OR of the flags like in for QPainter::drawText
       \param text Text to be rendered

       \return Calculated size
     */
    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const = 0;

    /*!
       Test if a string can be rendered by this text engine

       \param text Text to be tested
       \return true, if it can be rendered
     */
    virtual bool mightRender( const QString& text ) const = 0;

    /*!
       Return margins around the texts

       The textSize might include margins around the
       text, like QFontMetrics::descent(). In situations
       where texts need to be aligned in detail, knowing
       these margins might improve the layout calculations.

       \param font Font of the text
       \param text Text to be rendered
       \param left Return value for the left margin
       \param right Return value for the right margin
       \param top Return value for the top margin
       \param bottom Return value for the bottom margin
     */
    virtual void textMargins( const QFont& font, const QString& text,
        double& left, double& right, double& top, double& bottom ) const = 0;

    /*!
       Draw the text in a clipping rectangle

       \param painter Painter
       \param rect Clipping rectangle
       \param flags Bitwise OR of the flags like in for QPainter::drawText()
       \param text Text to be rendered
     */
    virtual void draw( QPainter* painter, const QRectF& rect,
        int flags, const QString& text ) const = 0;

  protected:
    QwtTextEngine();

  private:
    Q_DISABLE_COPY(QwtTextEngine)
};


/*!
   \brief A text engine for plain texts

   QwtPlainTextEngine renders texts using the basic Qt classes
   QPainter and QFontMetrics.
 */
class QWT_EXPORT QwtPlainTextEngine : public QwtTextEngine
{
  public:
    QwtPlainTextEngine();
    virtual ~QwtPlainTextEngine();

    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const QWT_OVERRIDE;

    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const QWT_OVERRIDE;

    virtual void draw( QPainter*, const QRectF& rect,
        int flags, const QString& text ) const QWT_OVERRIDE;

    virtual bool mightRender( const QString& ) const QWT_OVERRIDE;

    virtual void textMargins(
        const QFont&, const QString&,
        double& left, double& right,
        double& top, double& bottom ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};


#ifndef QT_NO_RICHTEXT

/*!
   \brief A text engine for Qt rich texts

   QwtRichTextEngine renders Qt rich texts using the classes
   of the Scribe framework of Qt.
 */
class QWT_EXPORT QwtRichTextEngine : public QwtTextEngine
{
  public:
    QwtRichTextEngine();

    virtual double heightForWidth( const QFont& font, int flags,
        const QString& text, double width ) const QWT_OVERRIDE;

    virtual QSizeF textSize( const QFont& font, int flags,
        const QString& text ) const QWT_OVERRIDE;

    virtual void draw( QPainter*, const QRectF& rect,
        int flags, const QString& text ) const QWT_OVERRIDE;

    virtual bool mightRender( const QString& ) const QWT_OVERRIDE;

    virtual void textMargins(
        const QFont&, const QString&,
        double& left, double& right,
        double& top, double& bottom ) const QWT_OVERRIDE;

  private:
    QString taggedText( const QString&, int flags ) const;
};

#endif // !QT_NO_RICHTEXT

#endif
