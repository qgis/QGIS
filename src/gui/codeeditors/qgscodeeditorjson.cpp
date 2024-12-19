/***************************************************************************
    qgscodeeditorjson.cpp - A JSON editor based on QScintilla
     --------------------------------------
    Date                 : 4.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditorjson.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerjson.h>


QgsCodeEditorJson::QgsCodeEditorJson( QWidget *parent )
  : QgsCodeEditor( parent,
                   QString(),
                   false,
                   false,
                   QgsCodeEditor::Flag::CodeFolding )
{
  if ( !parent )
  {
    setTitle( tr( "JSON Editor" ) );
  }
  QgsCodeEditorJson::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorJson::language() const
{
  return Qgis::ScriptLanguage::Json;
}

void QgsCodeEditorJson::initializeLexer()
{
  QsciLexerJSON *lexer = new QsciLexerJSON( this );

  QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QsciLexerJSON::CommentBlock );
  lexer->setFont( font, QsciLexerJSON::CommentLine );

  lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerJSON::Property );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerJSON::Keyword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QsciLexerJSON::Operator );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerJSON::Number );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentBlock ), QsciLexerJSON::CommentBlock );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QsciLexerJSON::CommentLine );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerJSON::String );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerJSON::IRI );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerJSON::UnclosedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ), QsciLexerJSON::Error );

  setLexer( lexer );
  setLineNumbersVisible( true );
  runPostLexerConfigurationTasks();
}
