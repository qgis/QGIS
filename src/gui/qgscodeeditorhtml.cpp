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

#include <QWidget>
#include <QString>
#include <QFont>
#include <QLabel>
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
  setSciLexerHTML();
}

void QgsCodeEditorHTML::setSciLexerHTML()
{
  QFont font = getMonospaceFont();
#ifdef Q_OS_MAC
  // The font size gotten from getMonospaceFont() is too small on Mac
  font.setPointSize( QLabel().font().pointSize() );
#endif

  QsciLexerHTML *lexer = new QsciLexerHTML( this );
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );
  setLexer( lexer );
}
