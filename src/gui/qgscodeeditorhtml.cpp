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
  setMarginVisible( false );
  setFoldingVisible( true );
  initializeLexer();
}

void QgsCodeEditorHTML::initializeLexer()
{
  QFont font = getMonospaceFont();
  QColor defaultColor = color( ColorRole::Default );

  QsciLexerHTML *lexer = new QsciLexerHTML( this );
  lexer->setDefaultFont( font );
  lexer->setDefaultColor( defaultColor );
  lexer->setDefaultPaper( color( ColorRole::Background ) );
  lexer->setFont( font, -1 );

  lexer->setColor( defaultColor, QsciLexerHTML::Default );
  lexer->setColor( color( ColorRole::Tag ), QsciLexerHTML::Tag );
  lexer->setColor( color( ColorRole::UnknownTag ), QsciLexerHTML::UnknownTag );
  lexer->setColor( color( ColorRole::Number ), QsciLexerHTML::HTMLNumber );
  lexer->setColor( color( ColorRole::Comment ), QsciLexerHTML::HTMLComment );
  lexer->setColor( color( ColorRole::SingleQuote ), QsciLexerHTML::HTMLSingleQuotedString );
  lexer->setColor( color( ColorRole::DoubleQuote ), QsciLexerHTML::HTMLDoubleQuotedString );

  setLexer( lexer );
}
