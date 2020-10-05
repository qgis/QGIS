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

#include "qgsapplication.h"
#include "qgscodeeditorjs.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerjavascript.h>


QgsCodeEditorJavascript::QgsCodeEditorJavascript( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "JavaScript Editor" ) );
  }
  setMarginVisible( false );
  setFoldingVisible( true );
  QgsCodeEditorJavascript::initializeLexer();
}

void QgsCodeEditorJavascript::initializeLexer()
{
  QsciLexerJavaScript *lexer = new QsciLexerJavaScript( this );

  const QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  lexer->setDefaultColor( lexerColor( ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( ColorRole::Background ) );
  lexer->setPaper( lexerColor( ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( ColorRole::Class ), QsciLexerJavaScript::GlobalClass );
  lexer->setColor( lexerColor( ColorRole::Keyword ), QsciLexerJavaScript::Keyword );
  lexer->setColor( lexerColor( ColorRole::Operator ), QsciLexerJavaScript::Operator );
  lexer->setColor( lexerColor( ColorRole::Number ), QsciLexerJavaScript::Number );
  lexer->setColor( lexerColor( ColorRole::Comment ), QsciLexerJavaScript::Comment );
  lexer->setColor( lexerColor( ColorRole::CommentLine ), QsciLexerJavaScript::CommentLine );
  lexer->setColor( lexerColor( ColorRole::DoubleQuote ), QsciLexerJavaScript::DoubleQuotedString );
  lexer->setColor( lexerColor( ColorRole::SingleQuote ), QsciLexerJavaScript::SingleQuotedString );
  lexer->setColor( lexerColor( ColorRole::Identifier ), QsciLexerJavaScript::Identifier );

  setLexer( lexer );
}
