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

#include "qgscodeeditorcss.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexercss.h>


QgsCodeEditorCSS::QgsCodeEditorCSS( QWidget *parent )
  : QgsCodeEditor( parent,
                   QString(),
                   false,
                   false,
                   QgsCodeEditor::Flag::CodeFolding )
{
  if ( !parent )
  {
    setTitle( tr( "CSS Editor" ) );
  }
  QgsCodeEditorCSS::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorCSS::language() const
{
  return Qgis::ScriptLanguage::Css;
}

void QgsCodeEditorCSS::initializeLexer()
{
  QsciLexerCSS *lexer = new QgsQsciLexerCSS( this );

  QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QsciLexerCSS::Comment );

  font.setItalic( false );
  font.setBold( true );
  lexer->setFont( font, QsciLexerCSS::SingleQuotedString );
  lexer->setFont( font, QsciLexerCSS::DoubleQuotedString );

  lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QsciLexerCSS::Tag );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QsciLexerCSS::ClassSelector );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerCSS::Attribute );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerCSS::PseudoClass );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QsciLexerCSS::UnknownPseudoClass );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QsciLexerCSS::Operator );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerCSS::Value );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerCSS::Comment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerCSS::DoubleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerCSS::SingleQuotedString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerCSS::CSS1Property );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerCSS::UnknownProperty );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerCSS::CSS2Property );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerCSS::CSS3Property );

  setLexer( lexer );

  runPostLexerConfigurationTasks();
}

///@cond PRIVATE
//
// QgsQsciLexerCSS
//
QgsQsciLexerCSS::QgsQsciLexerCSS( QObject *parent )
  : QsciLexerCSS( parent )
{

}

QString QgsQsciLexerCSS::description( int style ) const
{
  // see https://www.riverbankcomputing.com/pipermail/qscintilla/2019-July/001415.html
  if ( style == QsciLexerCSS::Comment )
    return QStringLiteral( "Comment" );

  return QsciLexerCSS::description( style );
}
///@endcond PRIVATE
