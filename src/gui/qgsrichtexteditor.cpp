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

#include "qgsrichtexteditor.h"
#include "qgsguiutils.h"
#include "qgscolorbutton.h"

#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QInputDialog>
#include <QColorDialog>
#include <QTextList>
#include <QtDebug>
#include <QFileDialog>
#include <QImageReader>
#include <QSettings>
#include <QBuffer>
#include <QUrl>
#include <QPlainTextEdit>
#include <QMenu>
#include <QDialog>
#include <QComboBox>
#include <QToolButton>

QgsRichTextEditor::QgsRichTextEditor( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mToolBar->setIconSize( QgsGuiUtils::iconSize( false ) );

  connect( mTextEdit, &QTextEdit::currentCharFormatChanged, this, &QgsRichTextEditor::slotCurrentCharFormatChanged );
  connect( mTextEdit, &QTextEdit::cursorPositionChanged, this, &QgsRichTextEditor::slotCursorPositionChanged );

  // paragraph formatting
  mParagraphStyleCombo = new QComboBox();
  mParagraphStyleCombo->addItem( tr( "Standard" ), ParagraphStandard );
  mParagraphStyleCombo->addItem( tr( "Heading 1" ), ParagraphHeading1 );
  mParagraphStyleCombo->addItem( tr( "Heading 2" ), ParagraphHeading2 );
  mParagraphStyleCombo->addItem( tr( "Heading 3" ), ParagraphHeading3 );
  mParagraphStyleCombo->addItem( tr( "Heading 4" ), ParagraphHeading4 );
  mParagraphStyleCombo->addItem( tr( "Monospace" ), ParagraphMonospace );

  connect( mParagraphStyleCombo, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsRichTextEditor::textStyle );
  mToolBar->insertWidget( mToolBar->actions().at( 0 ), mParagraphStyleCombo );

  mFontSizeCombo = new QComboBox();
  mFontSizeCombo->setEditable( true );
  mToolBar->insertWidget( mActionBold, mFontSizeCombo );

  // undo & redo
  mActionUndo->setShortcut( QKeySequence::Undo );
  mActionRedo->setShortcut( QKeySequence::Redo );

  connect( mTextEdit->document(), &QTextDocument::undoAvailable, mActionUndo, &QAction::setEnabled );
  connect( mTextEdit->document(), &QTextDocument::redoAvailable, mActionRedo, &QAction::setEnabled );

  mActionUndo->setEnabled( mTextEdit->document()->isUndoAvailable() );
  mActionRedo->setEnabled( mTextEdit->document()->isRedoAvailable() );

  connect( mActionUndo, &QAction::triggered, mTextEdit, &QTextEdit::undo );
  connect( mActionRedo, &QAction::triggered, mTextEdit, &QTextEdit::redo );

  // cut, copy & paste
  mActionCut->setShortcut( QKeySequence::Cut );
  mActionCopy->setShortcut( QKeySequence::Copy );
  mActionPaste->setShortcut( QKeySequence::Paste );

  mActionCut->setEnabled( false );
  mActionCopy->setEnabled( false );

  connect( mActionCut, &QAction::triggered, mTextEdit, &QTextEdit::cut );
  connect( mActionCopy, &QAction::triggered, mTextEdit, &QTextEdit::copy );
  connect( mActionPaste, &QAction::triggered, mTextEdit, &QTextEdit::paste );

  connect( mTextEdit, &QTextEdit::copyAvailable, mActionCut, &QAction::setEnabled );
  connect( mTextEdit, &QTextEdit::copyAvailable, mActionCopy, &QAction::setEnabled );

#ifndef QT_NO_CLIPBOARD
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, &QgsRichTextEditor::slotClipboardDataChanged );
#endif

  // link
  mActionInsertLink->setShortcut( QKeySequence( QStringLiteral( "CTRL+L" ) ) );
  connect( mActionInsertLink, &QAction::triggered, this, &QgsRichTextEditor::textLink );

  // bold, italic & underline
  mActionBold->setShortcut( QKeySequence( QStringLiteral( "CTRL+B" ) ) );
  mActionItalic->setShortcut( QKeySequence( QStringLiteral( "CTRL+I" ) ) );
  mActionUnderline->setShortcut( QKeySequence( QStringLiteral( "CTRL+U" ) ) );

  connect( mActionBold, &QAction::triggered, this, &QgsRichTextEditor::textBold );
  connect( mActionItalic, &QAction::triggered, this, &QgsRichTextEditor::textItalic );
  connect( mActionUnderline, &QAction::triggered, this, &QgsRichTextEditor::textUnderline );
  connect( mActionStrikeOut, &QAction::triggered, this, &QgsRichTextEditor::textStrikeout );

  QAction *removeFormat = new QAction( tr( "Remove Character Formatting" ), this );
  removeFormat->setShortcut( QKeySequence( QStringLiteral( "CTRL+M" ) ) );
  connect( removeFormat, &QAction::triggered, this, &QgsRichTextEditor::textRemoveFormat );
  mTextEdit->addAction( removeFormat );

  QAction *removeAllFormat = new QAction( tr( "Remove all Formatting" ), this );
  connect( removeAllFormat, &QAction::triggered, this, &QgsRichTextEditor::textRemoveAllFormat );
  mTextEdit->addAction( removeAllFormat );

  QAction *textsource = new QAction( tr( "Edit Document Source" ), this );
  textsource->setShortcut( QKeySequence( QStringLiteral( "CTRL+O" ) ) );
  connect( textsource, &QAction::triggered, this, &QgsRichTextEditor::textSource );
  mTextEdit->addAction( textsource );

  QAction *clearText = new QAction( tr( "Clear all Content" ), this );
  connect( clearText, &QAction::triggered, this, &QgsRichTextEditor::clearSource );
  mTextEdit->addAction( clearText );

  QMenu *menu = new QMenu( this );
  menu->addAction( removeAllFormat );
  menu->addAction( removeFormat );
  menu->addAction( textsource );
  menu->addAction( clearText );

  QToolButton *menuButton = new QToolButton();
  menuButton->setMenu( menu );
  menuButton->setPopupMode( QToolButton::InstantPopup );
  menuButton->setToolTip( tr( "Advanced Options" ) );
  menuButton->setText( QStringLiteral( "…" ) );
  QWidget *spacer = new QWidget();
  spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mToolBar->addWidget( spacer );
  mToolBar->addWidget( menuButton );

  // lists
  mActionBulletList->setShortcut( QKeySequence( QStringLiteral( "CTRL+-" ) ) );
  mActionOrderedList->setShortcut( QKeySequence( QStringLiteral( "CTRL+=" ) ) );
  connect( mActionBulletList, &QAction::triggered, this, &QgsRichTextEditor::listBullet );
  connect( mActionOrderedList, &QAction::triggered, this, &QgsRichTextEditor::listOrdered );

  // indentation
  mActionDecreaseIndent->setShortcut( QKeySequence( QStringLiteral( "CTRL+," ) ) );
  mActionIncreaseIndent->setShortcut( QKeySequence( QStringLiteral( "CTRL+." ) ) );
  connect( mActionIncreaseIndent, &QAction::triggered, this, &QgsRichTextEditor::increaseIndentation );
  connect( mActionDecreaseIndent, &QAction::triggered, this, &QgsRichTextEditor::decreaseIndentation );

  // font size
  QFontDatabase db;
  const QList< int > sizes = db.standardSizes();
  for ( int size : sizes )
    mFontSizeCombo->addItem( QString::number( size ), size );

  connect( mFontSizeCombo, &QComboBox::currentTextChanged, this, &QgsRichTextEditor::textSize );
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( QApplication::font().pointSize() ) );

  // text foreground color
  mForeColorButton = new QgsColorButton();
  mForeColorButton->setAllowOpacity( false );
  mForeColorButton->setColorDialogTitle( tr( "Foreground Color" ) );
  mForeColorButton->setColor( QApplication::palette().windowText().color() );
  mForeColorButton->setShowNoColor( false );
  mForeColorButton->setToolTip( tr( "Foreground color" ) );
  mForeColorButton->setMinimumWidth( QFontMetrics( font() ).horizontalAdvance( QStringLiteral( "x" ) ) * 10 );
  mForeColorButton->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

  QAction *listSeparator = mToolBar->insertSeparator( mActionBulletList );

  connect( mForeColorButton, &QgsColorButton::colorChanged, this, &QgsRichTextEditor::textFgColor );
  mToolBar->insertWidget( listSeparator, mForeColorButton );

  // text background color
  mBackColorButton = new QgsColorButton();
  mBackColorButton->setAllowOpacity( false );
  mBackColorButton->setColorDialogTitle( tr( "Background Color" ) );
  mBackColorButton->setToolTip( tr( "Background color" ) );
  mBackColorButton->setShowNull( true, tr( "No Background Color" ) );
  mBackColorButton->setToNull();
  mBackColorButton->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  mBackColorButton->setMinimumWidth( QFontMetrics( font() ).horizontalAdvance( QStringLiteral( "x" ) ) * 10 );
  connect( mBackColorButton, &QgsColorButton::colorChanged, this, &QgsRichTextEditor::textBgColor );
  mToolBar->insertWidget( listSeparator, mBackColorButton );

  // images
  connect( mActionInsertImage, &QAction::triggered, this, &QgsRichTextEditor::insertImage );

  fontChanged( mTextEdit->font() );
}

void QgsRichTextEditor::textSource()
{
  QDialog dialog( this );
  QPlainTextEdit *pte = new QPlainTextEdit( &dialog );
  pte->setPlainText( mTextEdit->toHtml() );
  QGridLayout *gl = new QGridLayout( &dialog );
  gl->addWidget( pte, 0, 0, 1, 1 );
  dialog.setWindowTitle( tr( "Document Source" ) );
  dialog.setMinimumWidth( 400 );
  dialog.setMinimumHeight( 600 );
  dialog.exec();

  mTextEdit->setHtml( pte->toPlainText() );
}

void QgsRichTextEditor::clearSource()
{
  mTextEdit->clear();
}

void QgsRichTextEditor::textRemoveFormat()
{
  QTextCharFormat fmt;
  fmt.setFontWeight( QFont::Normal );
  fmt.setFontUnderline( false );
  fmt.setFontStrikeOut( false );
  fmt.setFontItalic( false );
  fmt.setFontPointSize( 9 );
//  fmt.setFontFamily     ("Helvetica");
//  fmt.setFontStyleHint  (QFont::SansSerif);
//  fmt.setFontFixedPitch (true);

  mActionBold->setChecked( false );
  mActionUnderline->setChecked( false );
  mActionItalic->setChecked( false );
  mActionStrikeOut->setChecked( false );
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( 9 ) );

//  QTextBlockFormat bfmt = cursor.blockFormat();
//  bfmt->setIndent(0);

  fmt.clearBackground();

  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textRemoveAllFormat()
{
  mActionBold->setChecked( false );
  mActionUnderline->setChecked( false );
  mActionItalic->setChecked( false );
  mActionStrikeOut->setChecked( false );
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( 9 ) );
  QString text = mTextEdit->toPlainText();
  mTextEdit->setPlainText( text );
}

void QgsRichTextEditor::textBold()
{
  QTextCharFormat fmt;
  fmt.setFontWeight( mActionBold->isChecked() ? QFont::Bold : QFont::Normal );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::focusInEvent( QFocusEvent * )
{
  mTextEdit->setFocus( Qt::TabFocusReason );
}

void QgsRichTextEditor::textUnderline()
{
  QTextCharFormat fmt;
  fmt.setFontUnderline( mActionUnderline->isChecked() );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textItalic()
{
  QTextCharFormat fmt;
  fmt.setFontItalic( mActionItalic->isChecked() );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textStrikeout()
{
  QTextCharFormat fmt;
  fmt.setFontStrikeOut( mActionStrikeOut->isChecked() );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textSize( const QString &p )
{
  qreal pointSize = p.toDouble();
  if ( p.toFloat() > 0 )
  {
    QTextCharFormat fmt;
    fmt.setFontPointSize( pointSize );
    mergeFormatOnWordOrSelection( fmt );
  }
}

void QgsRichTextEditor::textLink( bool checked )
{
  bool unlink = false;
  QTextCharFormat fmt;
  if ( checked )
  {
    QString url = mTextEdit->currentCharFormat().anchorHref();
    bool ok;
    QString newUrl = QInputDialog::getText( this, tr( "Create a Link" ),
                                            tr( "Link URL:" ), QLineEdit::Normal,
                                            url,
                                            &ok );
    if ( ok )
    {
      fmt.setAnchor( true );
      fmt.setAnchorHref( newUrl );
      fmt.setForeground( QApplication::palette().color( QPalette::Link ) );
      fmt.setFontUnderline( true );
    }
    else
    {
      unlink = true;
    }
  }
  else
  {
    unlink = true;
  }
  if ( unlink )
  {
    fmt.setAnchor( false );
    fmt.setForeground( QApplication::palette().color( QPalette::Text ) );
    fmt.setFontUnderline( false );
  }
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textStyle( int )
{
  QTextCursor cursor = mTextEdit->textCursor();
  cursor.beginEditBlock();

  // standard
  if ( !cursor.hasSelection() )
  {
    cursor.select( QTextCursor::BlockUnderCursor );
  }
  QTextCharFormat fmt;
  cursor.setCharFormat( fmt );
  mTextEdit->setCurrentCharFormat( fmt );

  ParagraphItems style = static_cast< ParagraphItems >( mParagraphStyleCombo->currentData().toInt() );
  if ( style == ParagraphHeading1
       || style == ParagraphHeading2
       || style == ParagraphHeading3
       || style == ParagraphHeading4 )
  {
    if ( style == ParagraphHeading1 )
    {
      fmt.setFontPointSize( mFontSizeH1 );
    }
    if ( style == ParagraphHeading2 )
    {
      fmt.setFontPointSize( mFontSizeH2 );
    }
    if ( style == ParagraphHeading3 )
    {
      fmt.setFontPointSize( mFontSizeH3 );
    }
    if ( style == ParagraphHeading4 )
    {
      fmt.setFontPointSize( mFontSizeH4 );
    }
    if ( style == ParagraphHeading2 || style == ParagraphHeading4 )
    {
      fmt.setFontItalic( true );
    }

    fmt.setFontWeight( QFont::Bold );
  }
  if ( style == ParagraphMonospace )
  {
    fmt = cursor.charFormat();
    fmt.setFontFamily( QStringLiteral( "Monospace" ) );
    fmt.setFontStyleHint( QFont::Monospace );
    fmt.setFontFixedPitch( true );
  }
  cursor.setCharFormat( fmt );
  mTextEdit->setCurrentCharFormat( fmt );

  cursor.endEditBlock();
}

void QgsRichTextEditor::textFgColor()
{
  QColor col = mForeColorButton->color();
  QTextCursor cursor = mTextEdit->textCursor();
  if ( !cursor.hasSelection() )
  {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat fmt = cursor.charFormat();
  if ( col.isValid() )
  {
    fmt.setForeground( col );
  }
  else
  {
    fmt.clearForeground();
  }
  cursor.setCharFormat( fmt );
  mTextEdit->setCurrentCharFormat( fmt );
  fgColorChanged( col );
}

void QgsRichTextEditor::textBgColor()
{
  QColor col = mBackColorButton->color();
  QTextCursor cursor = mTextEdit->textCursor();
  if ( !cursor.hasSelection() )
  {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat fmt = cursor.charFormat();
  if ( col.isValid() )
  {
    fmt.setBackground( col );
  }
  else
  {
    fmt.clearBackground();
  }
  cursor.setCharFormat( fmt );
  mTextEdit->setCurrentCharFormat( fmt );
  bgColorChanged( col );
}

void QgsRichTextEditor::listBullet( bool checked )
{
  if ( checked )
  {
    mActionOrderedList->setChecked( false );
  }
  list( checked, QTextListFormat::ListDisc );
}

void QgsRichTextEditor::listOrdered( bool checked )
{
  if ( checked )
  {
    mActionBulletList->setChecked( false );
  }
  list( checked, QTextListFormat::ListDecimal );
}

void QgsRichTextEditor::list( bool checked, QTextListFormat::Style style )
{
  QTextCursor cursor = mTextEdit->textCursor();
  cursor.beginEditBlock();
  if ( !checked )
  {
    QTextBlockFormat obfmt = cursor.blockFormat();
    QTextBlockFormat bfmt;
    bfmt.setIndent( obfmt.indent() );
    cursor.setBlockFormat( bfmt );
  }
  else
  {
    QTextListFormat listFmt;
    if ( cursor.currentList() )
    {
      listFmt = cursor.currentList()->format();
    }
    listFmt.setStyle( style );
    cursor.createList( listFmt );
  }
  cursor.endEditBlock();
}

void QgsRichTextEditor::mergeFormatOnWordOrSelection( const QTextCharFormat &format )
{
  QTextCursor cursor = mTextEdit->textCursor();
  if ( !cursor.hasSelection() )
  {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  cursor.mergeCharFormat( format );
  mTextEdit->mergeCurrentCharFormat( format );
  mTextEdit->setFocus( Qt::TabFocusReason );
}

void QgsRichTextEditor::slotCursorPositionChanged()
{
  QTextList *l = mTextEdit->textCursor().currentList();
  if ( mLastBlockList && ( l == mLastBlockList || ( l != nullptr && mLastBlockList != nullptr
                           && l->format().style() == mLastBlockList->format().style() ) ) )
  {
    return;
  }
  mLastBlockList = l;
  if ( l )
  {
    QTextListFormat lfmt = l->format();
    if ( lfmt.style() == QTextListFormat::ListDisc )
    {
      mActionBulletList->setChecked( true );
      mActionOrderedList->setChecked( false );
    }
    else if ( lfmt.style() == QTextListFormat::ListDecimal )
    {
      mActionBulletList->setChecked( false );
      mActionOrderedList->setChecked( true );
    }
    else
    {
      mActionBulletList->setChecked( false );
      mActionOrderedList->setChecked( false );
    }
  }
  else
  {
    mActionBulletList->setChecked( false );
    mActionOrderedList->setChecked( false );
  }
}

void QgsRichTextEditor::fontChanged( const QFont &f )
{
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( f.pointSize() ) );
  mActionBold->setChecked( f.bold() );
  mActionItalic->setChecked( f.italic() );
  mActionUnderline->setChecked( f.underline() );
  mActionStrikeOut->setChecked( f.strikeOut() );
  if ( f.pointSize() == mFontSizeH1 )
  {
    mParagraphStyleCombo->setCurrentIndex( ParagraphHeading1 );
  }
  else if ( f.pointSize() == mFontSizeH2 )
  {
    mParagraphStyleCombo->setCurrentIndex( ParagraphHeading2 );
  }
  else if ( f.pointSize() == mFontSizeH3 )
  {
    mParagraphStyleCombo->setCurrentIndex( ParagraphHeading3 );
  }
  else if ( f.pointSize() == mFontSizeH4 )
  {
    mParagraphStyleCombo->setCurrentIndex( ParagraphHeading4 );
  }
  else
  {
    if ( f.fixedPitch() && f.family() == QLatin1String( "Monospace" ) )
    {
      mParagraphStyleCombo->setCurrentIndex( ParagraphMonospace );
    }
    else
    {
      mParagraphStyleCombo->setCurrentIndex( ParagraphStandard );
    }
  }
  if ( mTextEdit->textCursor().currentList() )
  {
    QTextListFormat lfmt = mTextEdit->textCursor().currentList()->format();
    if ( lfmt.style() == QTextListFormat::ListDisc )
    {
      mActionBulletList->setChecked( true );
      mActionOrderedList->setChecked( false );
    }
    else if ( lfmt.style() == QTextListFormat::ListDecimal )
    {
      mActionBulletList->setChecked( false );
      mActionOrderedList->setChecked( true );
    }
    else
    {
      mActionBulletList->setChecked( false );
      mActionOrderedList->setChecked( false );
    }
  }
  else
  {
    mActionBulletList->setChecked( false );
    mActionOrderedList->setChecked( false );
  }
}

void QgsRichTextEditor::fgColorChanged( const QColor &c )
{
  whileBlocking( mForeColorButton )->setColor( c );
}

void QgsRichTextEditor::bgColorChanged( const QColor &c )
{
  if ( c.isValid() )
    whileBlocking( mBackColorButton )->setColor( c );
  else
    whileBlocking( mBackColorButton )->setToNull();
}

void QgsRichTextEditor::slotCurrentCharFormatChanged( const QTextCharFormat &format )
{
  fontChanged( format.font() );
  bgColorChanged( ( format.background().isOpaque() ) ? format.background().color() : QColor() );
  fgColorChanged( ( format.foreground().isOpaque() ) ? format.foreground().color() : QApplication::palette().windowText().color() );
  mActionInsertLink->setChecked( format.isAnchor() );
}

void QgsRichTextEditor::slotClipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
  if ( const QMimeData *md = QApplication::clipboard()->mimeData() )
    mActionPaste->setEnabled( md->hasText() );
#endif
}

QString QgsRichTextEditor::toHtml() const
{
  QString s = mTextEdit->toHtml();
  // convert emails to links
  s = s.replace( QRegularExpression( QStringLiteral( "(<[^a][^>]+>(?:<span[^>]+>)?|\\s)([a-zA-Z\\d]+@[a-zA-Z\\d]+\\.[a-zA-Z]+)" ) ), QStringLiteral( "\\1<a href=\"mailto:\\2\">\\2</a>" ) );
  // convert links
  s = s.replace( QRegularExpression( QStringLiteral( "(<[^a][^>]+>(?:<span[^>]+>)?|\\s)((?:https?|ftp|file)://[^\\s'\"<>]+)" ) ), QStringLiteral( "\\1<a href=\"\\2\">\\2</a>" ) );
  // see also: Utils::linkify()
  return s;
}

void QgsRichTextEditor::increaseIndentation()
{
  indent( +1 );
}

void QgsRichTextEditor::decreaseIndentation()
{
  indent( -1 );
}

void QgsRichTextEditor::indent( int delta )
{
  QTextCursor cursor = mTextEdit->textCursor();
  cursor.beginEditBlock();
  QTextBlockFormat bfmt = cursor.blockFormat();
  int ind = bfmt.indent();
  if ( ind + delta >= 0 )
  {
    bfmt.setIndent( ind + delta );
  }
  cursor.setBlockFormat( bfmt );
  cursor.endEditBlock();
}

void QgsRichTextEditor::setText( const QString &text )
{
  if ( text.isEmpty() )
  {
    setPlainText( text );
    return;
  }
  if ( text[0] == '<' )
  {
    setHtml( text );
  }
  else
  {
    setPlainText( text );
  }
}

void QgsRichTextEditor::insertImage()
{
  QSettings s;
  QString attdir = s.value( QStringLiteral( "general/filedialog-path" ) ).toString();
  QString file = QFileDialog::getOpenFileName( this,
                 tr( "Select an image" ),
                 attdir,
                 tr( "JPEG (*.jpg);; GIF (*.gif);; PNG (*.png);; BMP (*.bmp);; All (*)" ) );
  if ( file.isEmpty() )
    return;

  QImage image = QImageReader( file ).read();

  mTextEdit->dropImage( image, QFileInfo( file ).suffix().toUpper().toLocal8Bit().data() );
}
