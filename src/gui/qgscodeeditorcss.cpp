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

  const QFont font = getMonospaceFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  lexer->setDefaultColor( color( ColorRole::Default ) );
  lexer->setDefaultPaper( color( ColorRole::Background ) );

  lexer->setColor( color( ColorRole::Tag ), QsciLexerCSS::Tag );
  lexer->setColor( color( ColorRole::Class ), QsciLexerCSS::ClassSelector );
  lexer->setColor( color( ColorRole::Keyword ), QsciLexerCSS::Attribute );
  lexer->setColor( color( ColorRole::Method ), QsciLexerCSS::PseudoClass );
  lexer->setColor( color( ColorRole::Method ), QsciLexerCSS::UnknownPseudoClass );
  lexer->setColor( color( ColorRole::Operator ), QsciLexerCSS::Operator );
  lexer->setColor( color( ColorRole::Number ), QsciLexerCSS::Value );
  lexer->setColor( color( ColorRole::Comment ), QsciLexerCSS::Comment );
  lexer->setColor( color( ColorRole::DoubleQuote ), QsciLexerCSS::DoubleQuotedString );
  lexer->setColor( color( ColorRole::SingleQuote ), QsciLexerCSS::SingleQuotedString );
  lexer->setColor( color( ColorRole::Identifier ), QsciLexerCSS::CSS1Property );
  lexer->setColor( color( ColorRole::Identifier ), QsciLexerCSS::UnknownProperty );
  lexer->setColor( color( ColorRole::Identifier ), QsciLexerCSS::CSS2Property );
  lexer->setColor( color( ColorRole::Identifier ), QsciLexerCSS::CSS3Property );

  setLexer( lexer );
}
