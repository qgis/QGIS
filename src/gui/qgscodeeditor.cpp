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

#include <QSettings>
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QFocusEvent>

QgsCodeEditor::QgsCodeEditor( QWidget *parent, const QString& title, bool folding, bool margin )
    : QsciScintilla( parent )
    , mWidgetTitle( title )
    , mFolding( folding )
    , mMargin( margin )
{
  if ( !parent && mWidgetTitle.isEmpty() )
  {
    setWindowTitle( "Text Editor" );
    setMinimumSize( 800, 300 );
  }
  else
  {
    setWindowTitle( mWidgetTitle );
  }
  setSciWidget();
  setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

QgsCodeEditor::~QgsCodeEditor()
{
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
// by the main entry, so that the default behaviour (Dialog closing) can trigger,
// but only is the auto-completion suggestion list isn't displayed
void QgsCodeEditor::keyPressEvent( QKeyEvent * event )
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
  setCaretLineBackgroundColor( QColor( "#fcf3ed" ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceBackgroundColor( QColor( "#b7f907" ) );
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

void QgsCodeEditor::setTitle( const QString& title )
{
  setWindowTitle( title );
}

void QgsCodeEditor::setMarginVisible( bool margin )
{
  mMargin = margin;
  if ( margin )
  {
    QFont marginFont( "Courier", 10 );
    setMarginLineNumbers( 1, true );
    setMarginsFont( marginFont );
    setMarginWidth( 1, "00000" );
    setMarginsForegroundColor( QColor( "#3E3EE3" ) );
    setMarginsBackgroundColor( QColor( "#f9f9f9" ) );
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
    setFoldMarginColors( QColor( "#f4f4f4" ), QColor( "#f4f4f4" ) );
  }
  else
  {
    setFolding( QsciScintilla::NoFoldStyle );
  }
}

void QgsCodeEditor::insertText( const QString& theText )
{
  // Insert the text or replace selected text
  if ( hasSelectedText() )
  {
    replaceSelectedText( theText );
  }
  else
  {
    int line, index;
    getCursorPosition( &line, &index );
    insertAt( theText, line, index );
    setCursorPosition( line, index + theText.length() );
  }
}

// Settings for font and fontsize
bool QgsCodeEditor::isFixedPitch( const QFont& font )
{
  return font.fixedPitch();
}

QFont QgsCodeEditor::getMonospaceFont()
{
  QSettings settings;
  QString loadFont = settings.value( "pythonConsole/fontfamilytextEditor", "Monospace" ).toString();
  int fontSize = settings.value( "pythonConsole/fontsizeEditor", 10 ).toInt();

  QFont font( loadFont );
  font.setFixedPitch( true );
  font.setPointSize( fontSize );
  font.setStyleHint( QFont::TypeWriter );
  font.setStretch( QFont::SemiCondensed );
  font.setLetterSpacing( QFont::PercentageSpacing, 87.0 );
  font.setBold( false );
  return font;
}
