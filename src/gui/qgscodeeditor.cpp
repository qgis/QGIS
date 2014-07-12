/***************************************************************************
    qgscodeeditor.cpp - description
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

QgsCodeEditor::QgsCodeEditor( QWidget *parent, QString title, bool folding, bool margin )
    : QsciScintilla( parent ),
    mWidgetTitle( title ),
    mFolding( folding ),
    mMargin( margin )
{
  if ( !parent && mWidgetTitle.isEmpty() )
  {
    setWindowTitle( "QScintilla2 Text Editor" );
    setMinimumSize( 800, 300 );
  }
  else
  {
    setWindowTitle( mWidgetTitle );
  }
  setSciWidget();
}

QgsCodeEditor::~QgsCodeEditor()
{
}

void QgsCodeEditor::setSciWidget()
{
  setUtf8( true );
  setCaretLineVisible( true );
  setCaretLineBackgroundColor( QColor( "#fcf3ed" ) );

  setBraceMatching( QsciScintilla::SloppyBraceMatch );
  setMatchedBraceBackgroundColor( QColor( "#b7f907" ) );
  // whether margin will be shown
  enableMargin( mMargin );
  // whether margin will be shown
  enableFolding( mFolding );
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

bool QgsCodeEditor::enableMargin( bool margin )
{
  if ( margin )
  {
    QFont marginFont( "Courier", 10 );
    setMarginLineNumbers( 1, true );
    setMarginsFont( marginFont );
    setMarginWidth( 1, "00000" );
    setMarginsForegroundColor( QColor( "#3E3EE3" ) );
    setMarginsBackgroundColor( QColor( "#f9f9f9" ) );
    return true;
  }
  else
  {
    setMarginWidth( 0, 0 );
    setMarginWidth( 1, 0 );
    setMarginWidth( 2, 0 );
    return false;
  }
}

void QgsCodeEditor::enableFolding( bool folding )
{
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

// Settings for font and fontsize
bool QgsCodeEditor::isFixedPitch( const QFont& font )
{
  const QFontInfo fi( font );
  qDebug() << fi.family() << fi.fixedPitch();
  return fi.fixedPitch();
}

QFont QgsCodeEditor::getMonospaceFont()
{
  QFont font( "monospace" );
  if ( isFixedPitch( font ) )
  {
    return font;
  }
  font.setStyleHint( QFont::Monospace );
  if ( isFixedPitch( font ) )
  {
    return font;
  }
  font.setStyleHint( QFont::TypeWriter );
  if ( isFixedPitch( font ) )
  {
    return font;
  }
  font.setFamily( "courier" );
  if ( isFixedPitch( font ) )
  {
    return font;
  }
  return font;
}
