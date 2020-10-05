/***************************************************************************
    qgscodeeditorcss.cpp - A CSS editor based on QScintilla
     --------------------------------------
    Date                 : 27-Jul-2014
    Copyright            : (C) 2014 by Nyall Dawson
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
#include "qgscodeeditorcss.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexercss.h>


QgsCodeEditorCSS::QgsCodeEditorCSS( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "CSS Editor" ) );
  }
  setMarginVisible( false );
  setFoldingVisible( true );
  QgsCodeEditorCSS::initializeLexer();
}

void QgsCodeEditorCSS::initializeLexer()
{
  QsciLexerCSS *lexer = new QsciLexerCSS( this );

  const QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  lexer->setDefaultColor( lexerColor( ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( ColorRole::Background ) );
  lexer->setPaper( lexerColor( ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( ColorRole::Tag ), QsciLexerCSS::Tag );
  lexer->setColor( lexerColor( ColorRole::Class ), QsciLexerCSS::ClassSelector );
  lexer->setColor( lexerColor( ColorRole::Keyword ), QsciLexerCSS::Attribute );
  lexer->setColor( lexerColor( ColorRole::Method ), QsciLexerCSS::PseudoClass );
  lexer->setColor( lexerColor( ColorRole::Method ), QsciLexerCSS::UnknownPseudoClass );
  lexer->setColor( lexerColor( ColorRole::Operator ), QsciLexerCSS::Operator );
  lexer->setColor( lexerColor( ColorRole::Number ), QsciLexerCSS::Value );
  lexer->setColor( lexerColor( ColorRole::Comment ), QsciLexerCSS::Comment );
  lexer->setColor( lexerColor( ColorRole::DoubleQuote ), QsciLexerCSS::DoubleQuotedString );
  lexer->setColor( lexerColor( ColorRole::SingleQuote ), QsciLexerCSS::SingleQuotedString );
  lexer->setColor( lexerColor( ColorRole::Identifier ), QsciLexerCSS::CSS1Property );
  lexer->setColor( lexerColor( ColorRole::Identifier ), QsciLexerCSS::UnknownProperty );
  lexer->setColor( lexerColor( ColorRole::Identifier ), QsciLexerCSS::CSS2Property );
  lexer->setColor( lexerColor( ColorRole::Identifier ), QsciLexerCSS::CSS3Property );

  setLexer( lexer );
}
