/***************************************************************************
    qgscodeeditorsql.cpp - A SQL editor based on QScintilla
     --------------------------------------
    Date                 : 06-Oct-2013
    Copyright            : (C) 2013 by Salvatore Larosa
    Email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscodeeditorsql.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexersql.h>


QgsCodeEditorSQL::QgsCodeEditorSQL( QWidget *parent )
    : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "SQL Editor" ) );
  }
  setMarginVisible( false );
  setFoldingVisible( true );
  setAutoCompletionCaseSensitivity( false );
  setSciLexerSQL();
}

QgsCodeEditorSQL::~QgsCodeEditorSQL()
{
}

void QgsCodeEditorSQL::setSciLexerSQL()
{
  QsciLexerSQL* sqlLexer = new QsciLexerSQL( this );
  sqlLexer->setDefaultFont( QFont( "Sans", 10 ) );

  setLexer( sqlLexer );
}
