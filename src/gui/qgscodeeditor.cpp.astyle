/***************************************************************************
    qgscodeeditor.cpp - A base code editor for QGIS and plugins.  Provides
                        a base editor using QScintilla for editors
     --------------------------------------
    Date                 : 06-Oct-2013
    Copyright            : (C) 2013 by Salvatore Larosa
    Email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditor.h"
#include "qgssettings.h"

#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QFocusEvent>

QgsCodeEditor::QgsCodeEditor( QWidget *parent, const QString &title, bool folding, bool margin )
  : QsciScintilla( parent )
  , mWidgetTitle( title )
  , mFolding( folding )
  , mMargin( margin )
{
  if ( !parent && mWidgetTitle.isEmpty() )
  {
    setWindowTitle( QStringLiteral( "Text Editor" ) );
  }
  else
  {
    setWindowTitle( mWidgetTitle );
  }
  setSciWidget();
  setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

// Workaround a bug in QScintilla 2.8.X
void QgsCodeEditor::focusOutEvent( QFocusEvent *event )
{
#if QSCINTILLA_VERSION >= 0x020800 && QSCINTILLA_VERSION < 0x020900
  if ( event->reason() != Qt::ActiveWindowFocusReason )
  {
    /* There's a bug in all QScintilla 2.8.X, where
       a focus out event that is not due to ActiveWindowFocusReason doesn't
       lead to the bliking caret being disabled. The hack consists in making
       QsciScintilla::focusOutEvent believe that the event is a ActiveWindowFocusReason
       The bug was fixed in 2.9 per:
        2015-04-14  Phil Thompson  <phil@riverbankcomputing.com>

        * qt/qsciscintillabase.cpp:
        Fixed a problem notifying when focus is lost to another application
        widget.
        [41734678234e]
    */
    QFocusEvent newFocusEvent( QEvent::FocusOut, Qt::ActiveWindowFocusReason );
    QsciScintilla::focusOutEvent( &newFocusEvent );
  }
  else
#endif
  {
    QsciScintilla::focusOutEvent( event );
  }
}

// This workaround a likely bug in QScintilla. The ESC key should not be consumned
// by the main entry, so that the default behavior (Dialog closing) can trigger,
// but only is the auto-completion suggestion list isn't displayed
void QgsCodeEditor::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape && !isListActive() )
  {
    // Shortcut QScintilla and redirect the event to the QWidget handler
    QWidget::keyPressEvent( event );
  }
  else
  {
    QsciScintilla::keyPressEvent( event );
  }
}

void QgsCodeEditor::setSciWidget()
{
  setUtf8( true );
  setCaretLineVisible( true );
  setCaretLineBackgroundColor( QColor( 252, 243, 237 ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceBackgroundColor( QColor( 183, 249, 7 ) );
  // whether margin will be shown
  setMarginVisible( mMargin );
  // whether margin will be shown
  setFoldingVisible( mFolding );
  // indentation
  setAutoIndent( true );
  setIndentationWidth( 4 );
  setTabIndents( true );
  setBackspaceUnindents( true );
  setTabWidth( 4 );
  // autocomplete
  setAutoCompletionThreshold( 2 );
  setAutoCompletionSource( QsciScintilla::AcsAPIs );
}

void QgsCodeEditor::setTitle( const QString &title )
{
  setWindowTitle( title );
}

void QgsCodeEditor::setMarginVisible( bool margin )
{
  mMargin = margin;
  if ( margin )
  {
    QFont marginFont( QStringLiteral( "Courier" ), 10 );
    setMarginLineNumbers( 1, true );
    setMarginsFont( marginFont );
    setMarginWidth( 1, QStringLiteral( "00000" ) );
    setMarginsForegroundColor( QColor( 62, 62, 227 ) );
    setMarginsBackgroundColor( QColor( 249, 249, 249 ) );
  }
  else
  {
    setMarginWidth( 0, 0 );
    setMarginWidth( 1, 0 );
    setMarginWidth( 2, 0 );
  }
}

void QgsCodeEditor::setFoldingVisible( bool folding )
{
  mFolding = folding;
  if ( folding )
  {
    setFolding( QsciScintilla::PlainFoldStyle );
    setFoldMarginColors( QColor( 244, 244, 244 ), QColor( 244, 244, 244 ) );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
  }
}

void QgsCodeEditor::insertText( const QString &text )
{
  // Insert the text or replace selected text
  if ( hasSelectedText() )
  {
    replaceSelectedText( text );
  }
  else
  {
    int line, index;
    getCursorPosition( &line, &index );
    insertAt( text, line, index );
    setCursorPosition( line, index + text.length() );
  }
}

// Settings for font and fontsize
bool QgsCodeEditor::isFixedPitch( const QFont &font )
{
  return font.fixedPitch();
}

QFont QgsCodeEditor::getMonospaceFont()
{
  QgsSettings settings;
  QString loadFont = settings.value( QStringLiteral( "pythonConsole/fontfamilytextEditor" ), "Monospace" ).toString();
  int fontSize = settings.value( QStringLiteral( "pythonConsole/fontsizeEditor" ), 10 ).toInt();

  QFont font( loadFont );
  font.setFixedPitch( true );
  font.setPointSize( fontSize );
  font.setStyleHint( QFont::TypeWriter );
  font.setStretch( QFont::SemiCondensed );
  font.setLetterSpacing( QFont::PercentageSpacing, 87.0 );
  font.setBold( false );
  return font;
}
