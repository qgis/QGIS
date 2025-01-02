/***************************************************************************
    qgscodeeditorshell.cpp
     ---------------------
    Date                 : April 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgscodeeditorshell.h"
#include "moc_qgscodeeditorshell.cpp"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerjson.h>


QgsCodeEditorShell::QgsCodeEditorShell( QWidget *parent, Mode mode, Qgis::ScriptLanguage language )
  : QgsCodeEditor( parent, QString(), false, false, QgsCodeEditor::Flag::CodeFolding, mode )
  , mLanguage( language )
{
  if ( mLanguage != Qgis::ScriptLanguage::Bash && mLanguage != Qgis::ScriptLanguage::Batch )
  {
#ifdef Q_OS_WIN
    mLanguage = Qgis::ScriptLanguage::Batch;
#else
    mLanguage = Qgis::ScriptLanguage::Bash;
#endif
  }

  if ( !parent )
  {
    switch ( mLanguage )
    {
      case Qgis::ScriptLanguage::Batch:
        setTitle( tr( "Batch Editor" ) );
        break;
      case Qgis::ScriptLanguage::Bash:
        setTitle( tr( "Bash Editor" ) );
        break;

      case Qgis::ScriptLanguage::Css:
      case Qgis::ScriptLanguage::QgisExpression:
      case Qgis::ScriptLanguage::Html:
      case Qgis::ScriptLanguage::JavaScript:
      case Qgis::ScriptLanguage::Json:
      case Qgis::ScriptLanguage::Python:
      case Qgis::ScriptLanguage::R:
      case Qgis::ScriptLanguage::Sql:
      case Qgis::ScriptLanguage::Unknown:
        break;
    }
  }

  QgsCodeEditorShell::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorShell::language() const
{
  return mLanguage;
}

void QgsCodeEditorShell::initializeLexer()
{
  QsciLexer *lexer = nullptr;
  Qgis::ScriptLanguage language = mLanguage;
  switch ( language )
  {
    case Qgis::ScriptLanguage::Batch:
    case Qgis::ScriptLanguage::Bash:
      break;

    case Qgis::ScriptLanguage::Css:
    case Qgis::ScriptLanguage::QgisExpression:
    case Qgis::ScriptLanguage::Html:
    case Qgis::ScriptLanguage::JavaScript:
    case Qgis::ScriptLanguage::Json:
    case Qgis::ScriptLanguage::Python:
    case Qgis::ScriptLanguage::R:
    case Qgis::ScriptLanguage::Sql:
    case Qgis::ScriptLanguage::Unknown:
#ifdef Q_OS_WIN
      language = Qgis::ScriptLanguage::Batch;
#else
      language = Qgis::ScriptLanguage::Bash;
#endif
      break;
  }

  switch ( language )
  {
    case Qgis::ScriptLanguage::Bash:
      lexer = new QgsQsciLexerBash();
      break;
    case Qgis::ScriptLanguage::Batch:
      lexer = new QgsQsciLexerBatch();
      break;

    default:
      return; // unreachable
  }

  QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  switch ( language )
  {
    case Qgis::ScriptLanguage::Bash:
    {
      font.setItalic( true );
      lexer->setFont( font, QgsQsciLexerBash::LineComment );

      lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
      lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
      lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ), QgsQsciLexerBash::Default );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Error ), QgsQsciLexerBash::Error );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QgsQsciLexerBash::LineComment );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QgsQsciLexerBash::Number );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QgsQsciLexerBash::Keyword );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QgsQsciLexerBash::String );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QgsQsciLexerBash::SingleQuotedString );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QgsQsciLexerBash::Operator );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QgsQsciLexerBash::Identifier );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QgsQsciLexerBash::ScalarVariable );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QgsQsciLexerBash::BacktickQuotedCommand );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QgsQsciLexerBash::HeredocDelimiter );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentBlock ), QgsQsciLexerBash::HeredocQuotedString );
      break;
    }

    case Qgis::ScriptLanguage::Batch:
    {
      font.setItalic( true );
      lexer->setFont( font, QgsQsciLexerBatch::Comment );

      lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
      lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
      lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ), QgsQsciLexerBatch::Default );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QgsQsciLexerBatch::Comment );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QgsQsciLexerBatch::Word );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QgsQsciLexerBatch::Label );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Decoration ), QgsQsciLexerBatch::Hide );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QgsQsciLexerBatch::Command );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QgsQsciLexerBatch::Identifier );
      lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QgsQsciLexerBatch::Operator );
      break;
    }

    default:
      break;
  }


  setLexer( lexer );
  setLineNumbersVisible( true );
  runPostLexerConfigurationTasks();
}

/// @cond PRIVATE
QgsQsciLexerBash::QgsQsciLexerBash( QObject *parent )
  : QsciLexer( parent )
{
}

const char *QgsQsciLexerBash::language() const
{
  return "bash";
}

const char *QgsQsciLexerBash::lexer() const
{
  return nullptr;
}

int QgsQsciLexerBash::lexerId() const
{
  return QsciScintillaBase::SCLEX_BASH;
}

QString QgsQsciLexerBash::description( int style ) const
{
  switch ( style )
  {
    case Default:
      return tr( "Default" );
    case Error:
      return tr( "Error" );
    case LineComment:
      return tr( "Comment" );
    case Number:
      return tr( "Number" );
    case Keyword:
      return tr( "Keyword" );
    case String:
      return tr( "String" );
    case SingleQuotedString:
      return tr( "Single Quoted String" );
    case Operator:
      return tr( "Operator" );
    case Identifier:
      return tr( "Identifier" );
    case ScalarVariable:
      return tr( "Scale Variable" );
    case Parameter:
      return tr( "Parameter" );
    case BacktickQuotedCommand:
      return tr( "Backtick Quoted Command" );
    case HeredocDelimiter:
      return tr( "Heredoc Delimiter" );
    case HeredocQuotedString:
      return tr( "Heredoc Quoted String" );
  }
  return QString();
}

const char *QgsQsciLexerBash::keywords( int set ) const
{
  switch ( set )
  {
    case 1:
      return "case cat do done echo else esac exit export fi find for if in set then while";
  }

  return nullptr;
}


QgsQsciLexerBatch::QgsQsciLexerBatch( QObject *parent )
  : QsciLexer( parent )
{
}

const char *QgsQsciLexerBatch::language() const
{
  return "batch";
}

const char *QgsQsciLexerBatch::lexer() const
{
  return nullptr;
}

int QgsQsciLexerBatch::lexerId() const
{
  return QsciScintillaBase::SCLEX_BATCH;
}

QString QgsQsciLexerBatch::description( int style ) const
{
  switch ( style )
  {
    case Default:
      return tr( "Default" );
    case Comment:
      return tr( "Comment" );
    case Word:
      return tr( "Word" );
    case Label:
      return tr( "Label" );
    case Hide:
      return tr( "Hide" );
    case Command:
      return tr( "Command" );
    case Operator:
      return tr( "Operator" );
    case Identifier:
      return tr( "Identifier" );
  }
  return QString();
}

const char *QgsQsciLexerBatch::keywords( int set ) const
{
  switch ( set )
  {
    case 1:
      return "call defined do echo else errorlevel exist exit for goto if in not set";
  }

  return nullptr;
}


///@endcond
