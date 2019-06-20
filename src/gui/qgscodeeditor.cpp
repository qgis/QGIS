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

#include "qgsapplication.h"
#include "qgscodeeditor.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"

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

  SendScintilla( SCI_SETADDITIONALSELECTIONTYPING, 1 );
  SendScintilla( SCI_SETMULTIPASTE, 1 );
  SendScintilla( SCI_SETVIRTUALSPACEOPTIONS, SCVS_RECTANGULARSELECTION );
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
    QWidget::keyPressEvent( event ); // clazy:exclude=skipped-base-method
  }
  else
  {
    QsciScintilla::keyPressEvent( event );
  }
}

void QgsCodeEditor::setSciWidget()
{
  QHash< QString, QColor > colors;
  if ( QgsApplication::instance()->themeName() != QStringLiteral( "default" ) )
  {
    QSettings ini( QgsApplication::instance()->uiThemes().value( QgsApplication::instance()->themeName() ) + "/qscintilla.ini", QSettings::IniFormat );
    for ( const auto &key : ini.allKeys() )
    {
      colors.insert( key, QgsSymbolLayerUtils::decodeColor( ini.value( key ).toString() ) );
    }
  }
  QPalette pal = qApp->palette();

  setUtf8( true );
  setCaretLineVisible( true );
  setCaretLineBackgroundColor( colors.value( QStringLiteral( "caretLineColor" ), QColor( 252, 243, 237 ) ) );
  setCaretForegroundColor( colors.value( QStringLiteral( "cursorColor" ), QColor( 51, 51, 51 ) ) );
  setSelectionForegroundColor( colors.value( QStringLiteral( "selectionForegroundColor" ), pal.color( QPalette::HighlightedText ) ) );
  setSelectionBackgroundColor( colors.value( QStringLiteral( "selectionBackgroundColor" ), pal.color( QPalette::Highlight ) ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceBackgroundColor( colors.value( QStringLiteral( "matchedBraceColor" ), QColor( 183, 249, 7 ) ) );
  // whether margin will be shown
  setMarginVisible( mMargin );
  setMarginsForegroundColor( colors.value( QStringLiteral( "marginForegroundColor" ), QColor( 62, 62, 227 ) ) );
  setMarginsBackgroundColor( colors.value( QStringLiteral( "marginBackgroundColor" ), QColor( 249, 249, 249 ) ) );
  setIndentationGuidesForegroundColor( colors.value( QStringLiteral( "marginForegroundColor" ), QColor( 62, 62, 227 ) ) );
  setIndentationGuidesBackgroundColor( colors.value( QStringLiteral( "marginBackgroundColor" ), QColor( 249, 249, 249 ) ) );
  // whether margin will be shown
  setFoldingVisible( mFolding );
  QColor foldColor = colors.value( QStringLiteral( "foldColor" ), QColor( 244, 244, 244 ) );
  setFoldMarginColors( foldColor, foldColor );
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
  font.setBold( false );
  return font;
}
