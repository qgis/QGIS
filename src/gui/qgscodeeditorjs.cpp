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
  setSciLexerJs();
}

void QgsCodeEditorJavascript::setSciLexerJs()
{
  QsciLexerJavaScript *lexer = new QsciLexerJavaScript( this );
  QFont f = getMonospaceFont();
  lexer->setDefaultFont( f );
  lexer->setFont( f, -1 );
  setLexer( lexer );
}
