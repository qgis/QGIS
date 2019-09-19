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
  QColor defaultColor = colors.value( QStringLiteral( "html/defaultFontColor" ), Qt::black );

  QsciLexerHTML *lexer = new QsciLexerHTML( this );
  lexer->setDefaultFont( font );
  lexer->setDefaultColor( defaultColor );
  lexer->setDefaultPaper( colors.value( QStringLiteral( "html/paperBackgroundColor" ), Qt::white ) );
  lexer->setFont( font, -1 );

  lexer->setColor( defaultColor, QsciLexerHTML::Default );
  lexer->setColor( colors.value( QStringLiteral( "html/tagFontColor" ), QColor( 66, 113, 174 ) ), QsciLexerHTML::Tag );
  lexer->setColor( colors.value( QStringLiteral( "html/unknownTagFontColor" ), QColor( 255, 0, 0 ) ), QsciLexerHTML::UnknownTag );
  lexer->setColor( colors.value( QStringLiteral( "html/numberFontColor" ), QColor( 200, 40, 41 ) ), QsciLexerHTML::HTMLNumber );
  lexer->setColor( colors.value( QStringLiteral( "html/commentFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerHTML::HTMLComment );
  lexer->setColor( colors.value( QStringLiteral( "html/singleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerHTML::HTMLSingleQuotedString );
  lexer->setColor( colors.value( QStringLiteral( "html/doubleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerHTML::HTMLDoubleQuotedString );

  setLexer( lexer );
}
