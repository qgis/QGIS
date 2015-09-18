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
  setSciLexerCSS();
}

QgsCodeEditorCSS::~QgsCodeEditorCSS()
{
}

void QgsCodeEditorCSS::setSciLexerCSS()
{
  QsciLexerCSS* lexer = new QsciLexerCSS( this );
  lexer->setDefaultFont( QFont( "Sans", 10 ) );

  setLexer( lexer );
}
