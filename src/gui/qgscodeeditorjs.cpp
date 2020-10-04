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

  const QFont font = getMonospaceFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  lexer->setDefaultColor( color( ColorRole::Default ) );
  lexer->setDefaultPaper( color( ColorRole::Background ) );

  lexer->setColor( color( ColorRole::Class ), QsciLexerJavaScript::GlobalClass );
  lexer->setColor( color( ColorRole::Keyword ), QsciLexerJavaScript::Keyword );
  lexer->setColor( color( ColorRole::Operator ), QsciLexerJavaScript::Operator );
  lexer->setColor( color( ColorRole::Number ), QsciLexerJavaScript::Number );
  lexer->setColor( color( ColorRole::Comment ), QsciLexerJavaScript::Comment );
  lexer->setColor( color( ColorRole::CommentLine ), QsciLexerJavaScript::CommentLine );
  lexer->setColor( color( ColorRole::DoubleQuote ), QsciLexerJavaScript::DoubleQuotedString );
  lexer->setColor( color( ColorRole::SingleQuote ), QsciLexerJavaScript::SingleQuotedString );
  lexer->setColor( color( ColorRole::Identifier ), QsciLexerJavaScript::Identifier );

  setLexer( lexer );
}
