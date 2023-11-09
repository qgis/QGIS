/***************************************************************************
    qgscodeeditorjs.cpp - A Javascript editor based on QScintilla
     --------------------------------------
    Date                 : June 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditorjs.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerjavascript.h>


QgsCodeEditorJavascript::QgsCodeEditorJavascript( QWidget *parent )
  : QgsCodeEditor( parent,
                   QString(),
                   false,
                   false,
                   QgsCodeEditor::Flag::CodeFolding )
{
  if ( !parent )
  {
    setTitle( tr( "JavaScript Editor" ) );
  }
  QgsCodeEditorJavascript::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorJavascript::language() const
{
  return Qgis::ScriptLanguage::JavaScript;
}

void QgsCodeEditorJavascript::initializeLexer()
{
  QsciLexerJavaScript *lexer = new QsciLexerJavaScript( this );

  QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QsciLexerJavaScript::Comment );
  lexer->setFont( font, QsciLexerJavaScript::CommentLine );

  lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerJavaScript::GlobalClass );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerJavaScript::Keyword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QsciLexerJavaScript::Operator );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerJavaScript::Number );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerJavaScript::Comment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QsciLexerJavaScript::CommentLine );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerJavaScript::DoubleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerJavaScript::SingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerJavaScript::Identifier );

  setLexer( lexer );
  setLineNumbersVisible( true );
  runPostLexerConfigurationTasks();
}
