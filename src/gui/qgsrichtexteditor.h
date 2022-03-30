/****************************************************************************
**
** Copyright (C) 2013 Jiří Procházka (Hobrasoft)
** Contact: http://www.hobrasoft.cz/
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file is under the terms of the GNU Lesser General Public License
** version 2.1 as published by the Free Software Foundation and appearing
** in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the
** GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGSRICHTEXTEDITOR_H
#define QGSRICHTEXTEDITOR_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgsrichtexteditorbase.h"
#include <QPointer>

class QImage;
class QComboBox;
class QgsColorButton;
class QgsCodeEditorHTML;

/*
 * Originally ported from https://github.com/Anchakor/MRichTextEditor, courtesy of Hobrasoft.
 */

/**
 * \ingroup gui
 * \brief A widget for editing rich text documents, with support for user controlled formatting of text
 * and insertion of hyperlinks and images.
 *
 * QgsRichTextEditor provides a reusable widget for allowing users to edit rich text documents,
 * and retrieving and setting the documents via HTML formatted strings.
 *
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsRichTextEditor : public QWidget, protected Ui::QgsRichTextEditorBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsRichTextEditor, with the specified \a parent widget.
     */
    QgsRichTextEditor( QWidget *parent = nullptr );

    /**
     * Returns the widget's content as a plain text string.
     *
     * \see toHtml()
     */
    QString toPlainText() const;

    /**
     * Returns the widget's content as a HTML string.
     *
     * \see toPlainText()
     */
    QString toHtml() const;

    /**
     * Returns a reference to the QTextDocument shown in the widget.
     */
    QTextDocument *document() { return mTextEdit->document(); }

    /**
     * Returns a reference to the text cursor.
     *
     * \see setTextCursor()
     */
    QTextCursor textCursor() const { return mTextEdit->textCursor(); }

    /**
     * Sets the current text \a cursor.
     *
     * \see textCursor()
     */
    void setTextCursor( const QTextCursor &cursor ) { mTextEdit->setTextCursor( cursor ); }

  public slots:

    /**
     * Sets the \a text to show in the widget.
     *
     * The \a text can either be a plain text string or a HTML document.
     */
    void setText( const QString &text );

    /**
     * Clears the current text from the widget.
     */
    void clearSource();

  signals:

    /**
     * Emitted when the text contents are changed.
     *
     * \since QGIS 3.26
     */
    void textChanged();

  protected:
    void focusInEvent( QFocusEvent *event ) override;

  private slots:
    void setPlainText( const QString &text ) { mTextEdit->setPlainText( text ); }
    void setHtml( const QString &text ) { mTextEdit->setHtml( text ); }
    void textRemoveFormat();
    void textRemoveAllFormat();
    void textBold();
    void textUnderline();
    void textStrikeout();
    void textItalic();
    void textSize( const QString &p );
    void textLink( bool checked );
    void textStyle( int index );
    void textFgColor();
    void textBgColor();
    void listBullet( bool checked );
    void listOrdered( bool checked );
    void slotCurrentCharFormatChanged( const QTextCharFormat &format );
    void slotCursorPositionChanged();
    void slotClipboardDataChanged();
    void increaseIndentation();
    void decreaseIndentation();
    void insertImage();
    void editSource( bool enabled );

  private:
    void mergeFormatOnWordOrSelection( const QTextCharFormat &format );
    void fontChanged( const QFont &f );
    void fgColorChanged( const QColor &c );
    void bgColorChanged( const QColor &c );
    void list( bool checked, QTextListFormat::Style style );
    void indent( int delta );

    int mFontSizeH1 = 18;
    int mFontSizeH2 = 16;
    int mFontSizeH3 = 14;
    int mFontSizeH4 = 12;

    enum ParagraphItems
    {
      ParagraphStandard = 0,
      ParagraphHeading1,
      ParagraphHeading2,
      ParagraphHeading3,
      ParagraphHeading4,
      ParagraphMonospace
    };

    QComboBox *mParagraphStyleCombo = nullptr;
    QComboBox *mFontSizeCombo = nullptr;

    QgsColorButton *mForeColorButton = nullptr;
    QgsColorButton *mBackColorButton = nullptr;
    QgsCodeEditorHTML *mSourceEdit = nullptr;

    QPointer<QTextList> mLastBlockList;
    QString mMonospaceFontFamily;
};



#endif // QGSRICHTEXTEDITOR_H
