/***************************************************************************
    qgscodeeditorhtml.cpp - A HTML editor based on QScintilla
     --------------------------------------
    Date                 : 20-Jul-2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow.nathan (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscodeeditorhtml.h"
#include "qgssymbollayerutils.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerhtml.h>


QgsCodeEditorHTML::QgsCodeEditorHTML( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "HTML Editor" ) );
  }
  setFoldingVisible( true );
  QgsCodeEditorHTML::initializeLexer();
}

void QgsCodeEditorHTML::initializeLexer()
{
  QFont font = lexerFont();
  const QColor defaultColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Default );

  QsciLexerHTML *lexer = new QsciLexerHTML( this );
  lexer->setDefaultFont( font );
  lexer->setDefaultColor( defaultColor );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QsciLexerHTML::HTMLComment );
  lexer->setFont( font, QsciLexerHTML::JavaScriptComment );
  lexer->setFont( font, QsciLexerHTML::JavaScriptCommentLine );

  lexer->setColor( defaultColor, QsciLexerHTML::Default );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QsciLexerHTML::Tag );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::UnknownTag ), QsciLexerHTML::UnknownTag );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerHTML::Attribute );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerHTML::UnknownAttribute );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerHTML::Entity );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerHTML::HTMLNumber );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerHTML::HTMLComment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerHTML::JavaScriptComment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QsciLexerHTML::JavaScriptCommentLine );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerHTML::JavaScriptNumber );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerHTML::JavaScriptKeyword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerHTML::JavaScriptDoubleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerHTML::JavaScriptSingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerHTML::HTMLSingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerHTML::HTMLDoubleQuotedString );

  setLexer( lexer );
  runPostLexerConfigurationTasks();
}
