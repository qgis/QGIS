/*
** Copyright (C) 2013 Jiří Procházka (Hobrasoft)
** Contact: http://www.hobrasoft.cz/
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
*/

#include "qgsrichtexteditor.h"
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

QgsRichTextEditor::QgsRichTextEditor( QWidget *parent ) : QWidget( parent )
{
  setupUi( this );
  m_lastBlockList = nullptr;

  connect( mTextEdit, &QTextEdit::currentCharFormatChanged, this, &QgsRichTextEditor::slotCurrentCharFormatChanged );
  connect( mTextEdit, &QTextEdit::cursorPositionChanged, this, &QgsRichTextEditor::slotCursorPositionChanged );

  m_fontsize_h1 = 18;
  m_fontsize_h2 = 16;
  m_fontsize_h3 = 14;
  m_fontsize_h4 = 12;

  fontChanged( mTextEdit->font() );
  bgColorChanged( mTextEdit->textColor() );

  // paragraph formatting

  m_paragraphItems << tr( "Standard" )
                   << tr( "Heading 1" )
                   << tr( "Heading 2" )
                   << tr( "Heading 3" )
                   << tr( "Heading 4" )
                   << tr( "Monospace" );
  f_paragraph->addItems( m_paragraphItems );

  connect( f_paragraph, qOverload< int >( &QComboBox::activated ), this, &QgsRichTextEditor::textStyle );

  // undo & redo

  f_undo->setShortcut( QKeySequence::Undo );
  f_redo->setShortcut( QKeySequence::Redo );

  connect( mTextEdit->document(), &QTextDocument::undoAvailable, f_undo, &QWidget::setEnabled );
  connect( mTextEdit->document(), &QTextDocument::redoAvailable, f_redo, &QWidget::setEnabled );

  f_undo->setEnabled( mTextEdit->document()->isUndoAvailable() );
  f_redo->setEnabled( mTextEdit->document()->isRedoAvailable() );

  connect( f_undo, &QAbstractButton::clicked, mTextEdit, &QTextEdit::undo );
  connect( f_redo, &QAbstractButton::clicked, mTextEdit, &QTextEdit::redo );

  // cut, copy & paste

  f_cut->setShortcut( QKeySequence::Cut );
  f_copy->setShortcut( QKeySequence::Copy );
  f_paste->setShortcut( QKeySequence::Paste );

  f_cut->setEnabled( false );
  f_copy->setEnabled( false );

  connect( f_cut, &QAbstractButton::clicked, mTextEdit, &QTextEdit::cut );
  connect( f_copy, &QAbstractButton::clicked, mTextEdit, &QTextEdit::copy );
  connect( f_paste, &QAbstractButton::clicked, mTextEdit, &QTextEdit::paste );

  connect( mTextEdit, &QTextEdit::copyAvailable, f_cut, &QWidget::setEnabled );
  connect( mTextEdit, &QTextEdit::copyAvailable, f_copy, &QWidget::setEnabled );

#ifndef QT_NO_CLIPBOARD
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, &QgsRichTextEditor::slotClipboardDataChanged );
#endif

  // link

  f_link->setShortcut( QKeySequence( QStringLiteral( "CTRL+L" ) ) );

  connect( f_link, &QAbstractButton::clicked, this, &QgsRichTextEditor::textLink );

  // bold, italic & underline

  f_bold->setShortcut( QKeySequence( QStringLiteral( "CTRL+B" ) ) );
  f_italic->setShortcut( QKeySequence( QStringLiteral( "CTRL+I" ) ) );
  f_underline->setShortcut( QKeySequence( QStringLiteral( "CTRL+U" ) ) );

  connect( f_bold, &QAbstractButton::clicked, this, &QgsRichTextEditor::textBold );
  connect( f_italic, &QAbstractButton::clicked, this, &QgsRichTextEditor::textItalic );
  connect( f_underline, &QAbstractButton::clicked, this, &QgsRichTextEditor::textUnderline );
  connect( f_strikeout, &QAbstractButton::clicked, this, &QgsRichTextEditor::textStrikeout );

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
  f_menu->setMenu( menu );
  f_menu->setPopupMode( QToolButton::InstantPopup );

  // lists

  f_list_bullet->setShortcut( QKeySequence( QStringLiteral( "CTRL+-" ) ) );
  f_list_ordered->setShortcut( QKeySequence( QStringLiteral( "CTRL+=" ) ) );

  connect( f_list_bullet, &QAbstractButton::clicked, this, &QgsRichTextEditor::listBullet );
  connect( f_list_ordered, &QAbstractButton::clicked, this, &QgsRichTextEditor::listOrdered );

  // indentation

  f_indent_dec->setShortcut( QKeySequence( QStringLiteral( "CTRL+," ) ) );
  f_indent_inc->setShortcut( QKeySequence( QStringLiteral( "CTRL+." ) ) );

  connect( f_indent_inc, &QAbstractButton::clicked, this, &QgsRichTextEditor::increaseIndentation );
  connect( f_indent_dec, &QAbstractButton::clicked, this, &QgsRichTextEditor::decreaseIndentation );

  // font size

  QFontDatabase db;
  const QList< int > sizes = db.standardSizes();
  for ( int size : sizes )
    f_fontsize->addItem( QString::number( size ) );

  connect( f_fontsize, &QComboBox::textActivated, this, &QgsRichTextEditor::textSize );
  f_fontsize->setCurrentIndex( f_fontsize->findText( QString::number( QApplication::font()
                               .pointSize() ) ) );

  // text foreground color

  QPixmap pix( 16, 16 );
  pix.fill( QApplication::palette().windowText().color() );
  f_fgcolor->setIcon( pix );

  connect( f_fgcolor, &QAbstractButton::clicked, this, &QgsRichTextEditor::textFgColor );

  // text background color

  pix.fill( QApplication::palette().window().color() );
  f_bgcolor->setIcon( pix );

  connect( f_bgcolor, &QAbstractButton::clicked, this, &QgsRichTextEditor::textBgColor );

  // images
  connect( f_image, &QAbstractButton::clicked, this, &QgsRichTextEditor::insertImage );
}


void QgsRichTextEditor::textSource()
{
  QDialog *dialog = new QDialog( this );
  QPlainTextEdit *pte = new QPlainTextEdit( dialog );
  pte->setPlainText( mTextEdit->toHtml() );
  QGridLayout *gl = new QGridLayout( dialog );
  gl->addWidget( pte, 0, 0, 1, 1 );
  dialog->setWindowTitle( tr( "Document Source" ) );
  dialog->setMinimumWidth( 400 );
  dialog->setMinimumHeight( 600 );
  dialog->exec();

  mTextEdit->setHtml( pte->toPlainText() );

  delete dialog;
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

  f_bold      ->setChecked( false );
  f_underline ->setChecked( false );
  f_italic    ->setChecked( false );
  f_strikeout ->setChecked( false );
  f_fontsize  ->setCurrentIndex( f_fontsize->findText( QStringLiteral( "9" ) ) );

//  QTextBlockFormat bfmt = cursor.blockFormat();
//  bfmt->setIndent(0);

  fmt.clearBackground();

  mergeFormatOnWordOrSelection( fmt );
}


void QgsRichTextEditor::textRemoveAllFormat()
{
  f_bold      ->setChecked( false );
  f_underline ->setChecked( false );
  f_italic    ->setChecked( false );
  f_strikeout ->setChecked( false );
  f_fontsize  ->setCurrentIndex( f_fontsize->findText( QStringLiteral( "9" ) ) );
  QString text = mTextEdit->toPlainText();
  mTextEdit->setPlainText( text );
}


void QgsRichTextEditor::textBold()
{
  QTextCharFormat fmt;
  fmt.setFontWeight( f_bold->isChecked() ? QFont::Bold : QFont::Normal );
  mergeFormatOnWordOrSelection( fmt );
}


void QgsRichTextEditor::focusInEvent( QFocusEvent * )
{
  mTextEdit->setFocus( Qt::TabFocusReason );
}


void QgsRichTextEditor::textUnderline()
{
  QTextCharFormat fmt;
  fmt.setFontUnderline( f_underline->isChecked() );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textItalic()
{
  QTextCharFormat fmt;
  fmt.setFontItalic( f_italic->isChecked() );
  mergeFormatOnWordOrSelection( fmt );
}

void QgsRichTextEditor::textStrikeout()
{
  QTextCharFormat fmt;
  fmt.setFontStrikeOut( f_strikeout->isChecked() );
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

void QgsRichTextEditor::textStyle( int index )
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

  if ( index == ParagraphHeading1
       || index == ParagraphHeading2
       || index == ParagraphHeading3
       || index == ParagraphHeading4 )
  {
    if ( index == ParagraphHeading1 )
    {
      fmt.setFontPointSize( m_fontsize_h1 );
    }
    if ( index == ParagraphHeading2 )
    {
      fmt.setFontPointSize( m_fontsize_h2 );
    }
    if ( index == ParagraphHeading3 )
    {
      fmt.setFontPointSize( m_fontsize_h3 );
    }
    if ( index == ParagraphHeading4 )
    {
      fmt.setFontPointSize( m_fontsize_h4 );
    }
    if ( index == ParagraphHeading2 || index == ParagraphHeading4 )
    {
      fmt.setFontItalic( true );
    }

    fmt.setFontWeight( QFont::Bold );
  }
  if ( index == ParagraphMonospace )
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
  QColor col = QColorDialog::getColor( mTextEdit->textColor(), this );
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
  QColor col = QColorDialog::getColor( mTextEdit->textBackgroundColor(), this );
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
    f_list_ordered->setChecked( false );
  }
  list( checked, QTextListFormat::ListDisc );
}

void QgsRichTextEditor::listOrdered( bool checked )
{
  if ( checked )
  {
    f_list_bullet->setChecked( false );
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
  if ( m_lastBlockList && ( l == m_lastBlockList || ( l != nullptr && m_lastBlockList != nullptr
                            && l->format().style() == m_lastBlockList->format().style() ) ) )
  {
    return;
  }
  m_lastBlockList = l;
  if ( l )
  {
    QTextListFormat lfmt = l->format();
    if ( lfmt.style() == QTextListFormat::ListDisc )
    {
      f_list_bullet->setChecked( true );
      f_list_ordered->setChecked( false );
    }
    else if ( lfmt.style() == QTextListFormat::ListDecimal )
    {
      f_list_bullet->setChecked( false );
      f_list_ordered->setChecked( true );
    }
    else
    {
      f_list_bullet->setChecked( false );
      f_list_ordered->setChecked( false );
    }
  }
  else
  {
    f_list_bullet->setChecked( false );
    f_list_ordered->setChecked( false );
  }
}

void QgsRichTextEditor::fontChanged( const QFont &f )
{
  f_fontsize->setCurrentIndex( f_fontsize->findText( QString::number( f.pointSize() ) ) );
  f_bold->setChecked( f.bold() );
  f_italic->setChecked( f.italic() );
  f_underline->setChecked( f.underline() );
  f_strikeout->setChecked( f.strikeOut() );
  if ( f.pointSize() == m_fontsize_h1 )
  {
    f_paragraph->setCurrentIndex( ParagraphHeading1 );
  }
  else if ( f.pointSize() == m_fontsize_h2 )
  {
    f_paragraph->setCurrentIndex( ParagraphHeading2 );
  }
  else if ( f.pointSize() == m_fontsize_h3 )
  {
    f_paragraph->setCurrentIndex( ParagraphHeading3 );
  }
  else if ( f.pointSize() == m_fontsize_h4 )
  {
    f_paragraph->setCurrentIndex( ParagraphHeading4 );
  }
  else
  {
    if ( f.fixedPitch() && f.family() == QLatin1String( "Monospace" ) )
    {
      f_paragraph->setCurrentIndex( ParagraphMonospace );
    }
    else
    {
      f_paragraph->setCurrentIndex( ParagraphStandard );
    }
  }
  if ( mTextEdit->textCursor().currentList() )
  {
    QTextListFormat lfmt = mTextEdit->textCursor().currentList()->format();
    if ( lfmt.style() == QTextListFormat::ListDisc )
    {
      f_list_bullet->setChecked( true );
      f_list_ordered->setChecked( false );
    }
    else if ( lfmt.style() == QTextListFormat::ListDecimal )
    {
      f_list_bullet->setChecked( false );
      f_list_ordered->setChecked( true );
    }
    else
    {
      f_list_bullet->setChecked( false );
      f_list_ordered->setChecked( false );
    }
  }
  else
  {
    f_list_bullet->setChecked( false );
    f_list_ordered->setChecked( false );
  }
}

void QgsRichTextEditor::fgColorChanged( const QColor &c )
{
  QPixmap pix( 16, 16 );
  if ( c.isValid() )
  {
    pix.fill( c );
  }
  else
  {
    pix.fill( QApplication::palette().windowText().color() );
  }
  f_fgcolor->setIcon( pix );
}

void QgsRichTextEditor::bgColorChanged( const QColor &c )
{
  QPixmap pix( 16, 16 );
  if ( c.isValid() )
  {
    pix.fill( c );
  }
  else
  {
    pix.fill( QApplication::palette().window().color() );
  }
  f_bgcolor->setIcon( pix );
}

void QgsRichTextEditor::slotCurrentCharFormatChanged( const QTextCharFormat &format )
{
  fontChanged( format.font() );
  bgColorChanged( ( format.background().isOpaque() ) ? format.background().color() : QColor() );
  fgColorChanged( ( format.foreground().isOpaque() ) ? format.foreground().color() : QColor() );
  f_link->setChecked( format.isAnchor() );
}

void QgsRichTextEditor::slotClipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
  if ( const QMimeData *md = QApplication::clipboard()->mimeData() )
    f_paste->setEnabled( md->hasText() );
#endif
}

QString QgsRichTextEditor::toHtml() const
{
  QString s = mTextEdit->toHtml();
  // convert emails to links
  s = s.replace( QRegularExpression( QStringLiteral( "(<[^a][^>]+>(?:<span[^>]+>)?|\\s)([a-zA-Z\\d]+@[a-zA-Z\\d]+\\.[a-zA-Z]+)" ) ), "\\1<a href=\"mailto:\\2\">\\2</a>" );
  // convert links
  s = s.replace( QRegularExpression( QStringLiteral( "(<[^a][^>]+>(?:<span[^>]+>)?|\\s)((?:https?|ftp|file)://[^\\s'\"<>]+)" ) ), "\\1<a href=\"\\2\">\\2</a>" );
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
  QImage image = QImageReader( file ).read();

  mTextEdit->dropImage( image, QFileInfo( file ).suffix().toUpper().toLocal8Bit().data() );
}
