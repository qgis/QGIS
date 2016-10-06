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
#include <QLabel>
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

/** Internal use.

   setAutoCompletionCaseSensitivity( false ) is not sufficient when installing
   a lexer, since its caseSensitive() method is actually used, and defaults
   to true.
   @note not available in Python bindings
   @ingroup gui
*/
class QgsCaseInsensitiveLexerSQL: public QsciLexerSQL
{
  public:
    //! constructor
    explicit QgsCaseInsensitiveLexerSQL( QObject *parent = 0 ) : QsciLexerSQL( parent ) {}

    bool caseSensitive() const override { return false; }
};

void QgsCodeEditorSQL::setSciLexerSQL()
{
  QFont font = getMonospaceFont();
#ifdef Q_OS_MAC
  // The font size gotten from getMonospaceFont() is too small on Mac
  font.setPointSize( QLabel().font().pointSize() );
#endif
  QsciLexerSQL* sqlLexer = new QgsCaseInsensitiveLexerSQL( this );
  sqlLexer->setDefaultFont( font );
  sqlLexer->setFont( font, -1 );
  font.setBold( true );
  sqlLexer->setFont( font, QsciLexerSQL::Keyword );
  sqlLexer->setColor( Qt::darkYellow, QsciLexerSQL::DoubleQuotedString ); // fields

  setLexer( sqlLexer );
}
