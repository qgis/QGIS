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

#include "qgscodeeditorsql.h"

#include <QWidget>
#include <QString>
#include <QFont>


QgsCodeEditorSQL::QgsCodeEditorSQL( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "SQL Editor" ) );
  }
  setAutoCompletionCaseSensitivity( false );
  QgsCodeEditorSQL::initializeLexer(); // avoid cppcheck warning by explicitly specifying namespace
}

Qgis::ScriptLanguage QgsCodeEditorSQL::language() const
{
  return Qgis::ScriptLanguage::Sql;
}

QgsCodeEditorSQL::~QgsCodeEditorSQL()
{
  if ( mApis )
  {
    mApis->cancelPreparation( );
  }
}

void QgsCodeEditorSQL::initializeLexer()
{
  QFont font = lexerFont();
  QColor defaultColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Default );

  mSqlLexer = new QgsCaseInsensitiveLexerSQL( this );
  mSqlLexer->setDefaultFont( font );
  mSqlLexer->setDefaultColor( defaultColor );
  mSqlLexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  mSqlLexer->setFont( font, -1 );
  font.setBold( true );
  mSqlLexer->setFont( font, QsciLexerSQL::Keyword );

  font.setBold( false );
  font.setItalic( true );
  mSqlLexer->setFont( font, QsciLexerSQL::Comment );
  mSqlLexer->setFont( font, QsciLexerSQL::CommentLine );

  mSqlLexer->setColor( defaultColor, QsciLexerSQL::Default );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Comment ), QsciLexerSQL::Comment );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QsciLexerSQL::CommentLine );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QsciLexerSQL::Number );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QsciLexerSQL::Keyword );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QsciLexerSQL::SingleQuotedString );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QsciLexerSQL::DoubleQuotedString );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QsciLexerSQL::Operator );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QsciLexerSQL::Identifier );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier ), QsciLexerSQL::QuotedIdentifier );
  mSqlLexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::QuotedOperator ), QsciLexerSQL::QuotedOperator );

  setLexer( mSqlLexer );

  runPostLexerConfigurationTasks();
}

void QgsCodeEditorSQL::setFields( const QgsFields &fields )
{

  QStringList fieldNames;

  for ( const QgsField &field : std::as_const( fields ) )
  {
    fieldNames.push_back( field.name() );
  }

  setFieldNames( fieldNames );

}

void QgsCodeEditorSQL::updateApis()
{
  mApis = new QsciAPIs( mSqlLexer );

  for ( const QString &fieldName : std::as_const( mFieldNames ) )
  {
    mApis->add( fieldName );
  }

  for ( const QString &keyword : std::as_const( mExtraKeywords ) )
  {
    mApis->add( keyword );
  }

  mApis->prepare();
  mSqlLexer->setAPIs( mApis );
}

QStringList QgsCodeEditorSQL::extraKeywords() const
{
  return mExtraKeywords.values();
}

void QgsCodeEditorSQL::setExtraKeywords( const QStringList &extraKeywords )
{
  mExtraKeywords = qgis::listToSet( extraKeywords );
  updateApis();
}

QStringList QgsCodeEditorSQL::fieldNames() const
{
  return mFieldNames.values();
}

void QgsCodeEditorSQL::setFieldNames( const QStringList &fieldNames )
{
  mFieldNames = qgis::listToSet( fieldNames );
  updateApis();
}


