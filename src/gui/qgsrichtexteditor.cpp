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
#include "qgscodeeditor.h"
#include "qgscodeeditorhtml.h"

#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QInputDialog>
#include <QTextList>
#include <QtDebug>
#include <QFileDialog>
#include <QImageReader>
#include <QSettings>
#include <QUrl>
#include <QMenu>
#include <QComboBox>
#include <QToolButton>

QgsRichTextEditor::QgsRichTextEditor( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mMonospaceFontFamily = QgsCodeEditor::getMonospaceFont().family();

  QVBoxLayout *sourceLayout = new QVBoxLayout();
  sourceLayout->setContentsMargins( 0, 0, 0, 0 );
  mSourceEdit = new QgsCodeEditorHTML();
  sourceLayout->addWidget( mSourceEdit );
  mPageSourceEdit->setLayout( sourceLayout );

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

  connect( mParagraphStyleCombo, qOverload< int >( &QComboBox::activated ), this, &QgsRichTextEditor::textStyle );
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

  QAction *clearText = new QAction( tr( "Clear all Content" ), this );
  connect( clearText, &QAction::triggered, this, &QgsRichTextEditor::clearSource );
  mTextEdit->addAction( clearText );

  QMenu *menu = new QMenu( this );
  menu->addAction( removeAllFormat );
  menu->addAction( removeFormat );
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
  const QList< int > sizes = QFontDatabase::standardSizes();
  for ( const int size : sizes )
    mFontSizeCombo->addItem( QString::number( size ), size );

  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( QApplication::font().pointSize() ) );

  // text foreground color
  mForeColorButton = new QgsColorButton();
  mForeColorButton->setAllowOpacity( false );
  mForeColorButton->setColorDialogTitle( tr( "Foreground Color" ) );
  mForeColorButton->setColor( palette().windowText().color() );
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

  connect( mActionEditSource, &QAction::toggled, this, &QgsRichTextEditor::editSource );

  // images
  connect( mActionInsertImage, &QAction::triggered, this, &QgsRichTextEditor::insertImage );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  connect( mFontSizeCombo, qOverload< const QString &>( &QComboBox::activated ), this, &QgsRichTextEditor::textSize );
#else
  connect( mFontSizeCombo, &QComboBox::textActivated, this, &QgsRichTextEditor::textSize );
#endif

  fontChanged( mTextEdit->font() );

  connect( mTextEdit, &QTextEdit::textChanged, this, &QgsRichTextEditor::textChanged );
  connect( mSourceEdit, &QgsCodeEditorHTML::textChanged, this, &QgsRichTextEditor::textChanged );
}

QString QgsRichTextEditor::toPlainText() const
{
  switch ( mStackedWidget->currentIndex() )
  {
    case 0:
      return mTextEdit->toPlainText();

    case 1:
      // go via text edit to remove html from text...
      mTextEdit->setText( mSourceEdit->text() );
      return mTextEdit->toPlainText();
  }
  return QString();
}

QString QgsRichTextEditor::toHtml() const
{
  switch ( mStackedWidget->currentIndex() )
  {
    case 0:
      return mTextEdit->toHtml();

    case 1:
      return mSourceEdit->text();
  }
  return QString();
}

void QgsRichTextEditor::editSource( bool enabled )
{
  if ( enabled )
  {
    mSourceEdit->setText( mTextEdit->toHtml() );
    mStackedWidget->setCurrentIndex( 1 );
  }
  else
  {
    mTextEdit->setHtml( mSourceEdit->text() );
    mStackedWidget->setCurrentIndex( 0 );
    mSourceEdit->clear();
  }

  // disable formatting actions when in html edit mode
  mFontSizeCombo->setEnabled( !enabled );
  mParagraphStyleCombo->setEnabled( !enabled );
  mForeColorButton->setEnabled( !enabled );
  mBackColorButton->setEnabled( !enabled );
  mActionUndo->setEnabled( !enabled );
  mActionRedo->setEnabled( !enabled );
  mActionCut->setEnabled( !enabled );
  mActionCopy->setEnabled( !enabled );
  mActionPaste->setEnabled( !enabled );
  mActionInsertLink->setEnabled( !enabled );
  mActionBold->setEnabled( !enabled );
  mActionItalic->setEnabled( !enabled );
  mActionUnderline->setEnabled( !enabled );
  mActionStrikeOut->setEnabled( !enabled );
  mActionBulletList->setEnabled( !enabled );
  mActionOrderedList->setEnabled( !enabled );
  mActionDecreaseIndent->setEnabled( !enabled );
  mActionIncreaseIndent->setEnabled( !enabled );
  mActionInsertImage->setEnabled( !enabled );
}

void QgsRichTextEditor::clearSource()
{
  mTextEdit->clear();
}

void QgsRichTextEditor::textRemoveFormat()
{
  QTextCharFormat format;
  format.setFontWeight( QFont::Normal );
  format.setFontUnderline( false );
  format.setFontStrikeOut( false );
  format.setFontItalic( false );
  format.setFontPointSize( 9 );

  mActionBold->setChecked( false );
  mActionUnderline->setChecked( false );
  mActionItalic->setChecked( false );
  mActionStrikeOut->setChecked( false );
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( 9 ) );

  format.clearBackground();

  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::textRemoveAllFormat()
{
  mActionBold->setChecked( false );
  mActionUnderline->setChecked( false );
  mActionItalic->setChecked( false );
  mActionStrikeOut->setChecked( false );
  mFontSizeCombo->setCurrentIndex( mFontSizeCombo->findData( 9 ) );
  const QString text = mTextEdit->toPlainText();
  mTextEdit->setPlainText( text );
}

void QgsRichTextEditor::textBold()
{
  QTextCharFormat format;
  format.setFontWeight( mActionBold->isChecked() ? QFont::Bold : QFont::Normal );
  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::focusInEvent( QFocusEvent * )
{
  mTextEdit->setFocus( Qt::TabFocusReason );
}

void QgsRichTextEditor::textUnderline()
{
  QTextCharFormat format;
  format.setFontUnderline( mActionUnderline->isChecked() );
  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::textItalic()
{
  QTextCharFormat format;
  format.setFontItalic( mActionItalic->isChecked() );
  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::textStrikeout()
{
  QTextCharFormat format;
  format.setFontStrikeOut( mActionStrikeOut->isChecked() );
  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::textSize( const QString &p )
{
  const qreal pointSize = p.toDouble();
  if ( p.toFloat() > 0 )
  {
    QTextCharFormat format;
    format.setFontPointSize( pointSize );
    mergeFormatOnWordOrSelection( format );
  }
}

void QgsRichTextEditor::textLink( bool checked )
{
  bool unlink = false;
  QTextCharFormat format;
  if ( checked )
  {
    const QString url = mTextEdit->currentCharFormat().anchorHref();
    bool ok;
    const QString newUrl = QInputDialog::getText( this, tr( "Create a Link" ),
                           tr( "Link URL:" ), QLineEdit::Normal,
                           url,
                           &ok );
    if ( ok )
    {
      format.setAnchor( true );
      format.setAnchorHref( newUrl );
      format.setForeground( palette().color( QPalette::Link ) );
      format.setFontUnderline( true );
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
    format.setAnchor( false );
    format.setForeground( palette().color( QPalette::Text ) );
    format.setFontUnderline( false );
  }
  mergeFormatOnWordOrSelection( format );
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
  QTextCharFormat format;
  cursor.setCharFormat( format );
  mTextEdit->setCurrentCharFormat( format );

  const ParagraphItems style = static_cast< ParagraphItems >( mParagraphStyleCombo->currentData().toInt() );

  switch ( style )
  {
    case QgsRichTextEditor::ParagraphStandard:
      break;

    case QgsRichTextEditor::ParagraphHeading1:
      format.setFontPointSize( mFontSizeH1 );
      format.setFontWeight( QFont::Bold );
      break;

    case QgsRichTextEditor::ParagraphHeading2:
      format.setFontPointSize( mFontSizeH2 );
      format.setFontWeight( QFont::Bold );
      format.setFontItalic( true );
      break;

    case QgsRichTextEditor::ParagraphHeading3:
      format.setFontPointSize( mFontSizeH3 );
      format.setFontWeight( QFont::Bold );
      break;

    case QgsRichTextEditor::ParagraphHeading4:
      format.setFontPointSize( mFontSizeH4 );
      format.setFontWeight( QFont::Bold );
      format.setFontItalic( true );
      break;

    case QgsRichTextEditor::ParagraphMonospace:
    {
      format = cursor.charFormat();
      format.setFontFamily( mMonospaceFontFamily );
      format.setFontStyleHint( QFont::Monospace );
      format.setFontFixedPitch( true );
      break;
    }
  }

  cursor.setCharFormat( format );
  mTextEdit->setCurrentCharFormat( format );

  cursor.endEditBlock();
}

void QgsRichTextEditor::textFgColor()
{
  QTextCharFormat format;
  format.setForeground( mForeColorButton->color() );
  mergeFormatOnWordOrSelection( format );
}

void QgsRichTextEditor::textBgColor()
{
  QTextCharFormat format;
  const QColor col = mBackColorButton->color();
  if ( col.isValid() )
  {
    format.setBackground( col );
  }
  else
  {
    format.clearBackground();
  }
  mergeFormatOnWordOrSelection( format );
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
    const QTextBlockFormat originalFormat = cursor.blockFormat();
    QTextBlockFormat format;
    format.setIndent( originalFormat.indent() );
    cursor.setBlockFormat( format );
  }
  else
  {
    QTextListFormat listFormat;
    if ( cursor.currentList() )
    {
      listFormat = cursor.currentList()->format();
    }
    listFormat.setStyle( style );
    cursor.createList( listFormat );
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
    const QTextListFormat listFormat = l->format();
    if ( listFormat.style() == QTextListFormat::ListDisc )
    {
      mActionBulletList->setChecked( true );
      mActionOrderedList->setChecked( false );
    }
    else if ( listFormat.style() == QTextListFormat::ListDecimal )
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
    if ( f.fixedPitch() && f.family() == mMonospaceFontFamily )
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
    const QTextListFormat listFormat = mTextEdit->textCursor().currentList()->format();
    if ( listFormat.style() == QTextListFormat::ListDisc )
    {
      mActionBulletList->setChecked( true );
      mActionOrderedList->setChecked( false );
    }
    else if ( listFormat.style() == QTextListFormat::ListDecimal )
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
  fgColorChanged( ( format.foreground().isOpaque() ) ? format.foreground().color() : palette().windowText().color() );
  mActionInsertLink->setChecked( format.isAnchor() );
}

void QgsRichTextEditor::slotClipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
  if ( const QMimeData *md = QApplication::clipboard()->mimeData() )
    mActionPaste->setEnabled( md->hasText() );
#endif
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
  QTextBlockFormat format = cursor.blockFormat();
  const int indent = format.indent();
  if ( indent + delta >= 0 )
  {
    format.setIndent( indent + delta );
  }
  cursor.setBlockFormat( format );
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
  const QSettings s;
  const QString attdir = s.value( QStringLiteral( "general/filedialog-path" ) ).toString();
  const QString file = QFileDialog::getOpenFileName( this,
                       tr( "Select an image" ),
                       attdir,
                       tr( "JPEG (*.jpg);; GIF (*.gif);; PNG (*.png);; BMP (*.bmp);; All (*)" ) );
  if ( file.isEmpty() )
    return;

  const QImage image = QImageReader( file ).read();

  mTextEdit->dropImage( image, QFileInfo( file ).suffix().toUpper().toLocal8Bit().data() );
}
