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
#include "qgssymbollayerutils.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <QLabel>


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


void QgsCodeEditorSQL::setSciLexerSQL()
{
  QHash< QString, QColor > colors;
  if ( QgsApplication::instance()->themeName() != QStringLiteral( "default" ) )
  {
    QSettings ini( QgsApplication::instance()->uiThemes().value( QgsApplication::instance()->themeName() ) + "/qscintilla.ini", QSettings::IniFormat );
    for ( const auto &key : ini.allKeys() )
    {
      colors.insert( key, QgsSymbolLayerUtils::decodeColor( ini.value( key ).toString() ) );
    }
  }

  QFont font = getMonospaceFont();
#ifdef Q_OS_MAC
  // The font size gotten from getMonospaceFont() is too small on Mac
  font.setPointSize( QLabel().font().pointSize() );
#endif
  QColor defaultColor = colors.value( QStringLiteral( "sql/defaultFontColor" ), Qt::black );

  QsciLexerSQL *sqlLexer = new QgsCaseInsensitiveLexerSQL( this );
  sqlLexer->setDefaultFont( font );
  sqlLexer->setDefaultColor( defaultColor );
  sqlLexer->setDefaultPaper( colors.value( QStringLiteral( "sql/paperBackgroundColor" ), Qt::white ) );
  sqlLexer->setFont( font, -1 );
  font.setBold( true );
  sqlLexer->setFont( font, QsciLexerSQL::Keyword );

  sqlLexer->setColor( defaultColor, QsciLexerSQL::Default );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/commentFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerSQL::Comment );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/commentLineFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerSQL::CommentLine );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/numberFontColor" ), QColor( 200, 40, 41 ) ), QsciLexerSQL::Number );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/keywordFontColor" ), QColor( 137, 89, 168 ) ), QsciLexerSQL::Keyword );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/singleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerSQL::SingleQuotedString );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/doubleQuoteFontColor" ), QColor( 234, 183, 0 ) ), QsciLexerSQL::DoubleQuotedString );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/operatorFontColor" ), QColor( 66, 113, 174 ) ), QsciLexerSQL::Operator );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/identifierFontColor" ), QColor( 62, 153, 159 ) ), QsciLexerSQL::Identifier );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/QuotedIdentifierFontColor" ), Qt::black ), QsciLexerSQL::QuotedIdentifier );
  sqlLexer->setColor( colors.value( QStringLiteral( "sql/QuotedOperatorFontColor" ), Qt::black ), QsciLexerSQL::QuotedOperator );

  setLexer( sqlLexer );
}
