/***************************************************************************
    qgscodeeditorexpressoin.cpp - An expression editor based on QScintilla
     --------------------------------------
    Date                 : 8.9.2018
    Copyright            : (C) 2018 by Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditorexpression.h"
#include "qgsexpression.h"

#include <QString>
#include <QFont>

QgsCodeEditorExpression::QgsCodeEditorExpression( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "Expression Editor" ) );
  }
  setAutoCompletionCaseSensitivity( false );
  QgsCodeEditorExpression::initializeLexer(); // avoid cppcheck warning by explicitly specifying namespace
}

Qgis::ScriptLanguage QgsCodeEditorExpression::language() const
{
  return Qgis::ScriptLanguage::QgisExpression;
}

void QgsCodeEditorExpression::setExpressionContext( const QgsExpressionContext &context )
{
  mVariables.clear();

  const QStringList variableNames = context.filteredVariableNames();
  for ( const QString &var : variableNames )
  {
    mVariables << '@' + var;
  }

  // always show feature variables in autocomplete -- they may not be available in the context
  // at time of showing an expression builder, but they'll generally be available at evaluation time.
  mVariables << QStringLiteral( "@feature" );
  mVariables << QStringLiteral( "@id" );
  mVariables << QStringLiteral( "@geometry" );

  mContextFunctions = context.functionNames();

  mFunctions.clear();

  const int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpressionFunction *func = QgsExpression::Functions()[i];
    if ( func->isDeprecated() ) // don't show deprecated functions
      continue;
    if ( func->isContextual() )
    {
      //don't show contextual functions by default - it's up the the QgsExpressionContext
      //object to provide them if supported
      continue;
    }

    QString signature = func->name();
    if ( !signature.startsWith( '$' ) )
    {
      signature += '(';

      QStringList paramNames;
      const QgsExpressionFunction::ParameterList parameters = func->parameters();
      for ( const QgsExpressionFunction::Parameter &param : parameters )
      {
        paramNames << param.name();
      }

      // No named parameters but there should be parameteres? Show an ellipsis at least
      if ( parameters.isEmpty() && func->params() )
        signature += QChar( 0x2026 );

      signature += paramNames.join( ", " );

      signature += ')';
    }
    mFunctions << signature;
  }

  updateApis();
}

void QgsCodeEditorExpression::setFields( const QgsFields &fields )
{
  mFieldNames.clear();

  for ( const QgsField &field : fields )
  {
    mFieldNames << field.name();
  }

  updateApis();
}

void QgsCodeEditorExpression::initializeLexer()
{
  QFont font = lexerFont();
  QColor defaultColor = lexerColor( QgsCodeEditorColorScheme::ColorRole::Default );

  mSqlLexer = new QgsLexerExpression( this );
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

  mSqlLexer->setColor( Qt::darkYellow, QsciLexerSQL::DoubleQuotedString ); // fields

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

void QgsCodeEditorExpression::updateApis()
{
  mApis = new QgsSciApisExpression( mSqlLexer );

  for ( const QString &var : std::as_const( mVariables ) )
  {
    mApis->add( var );
  }

  for ( const QString &function : std::as_const( mContextFunctions ) )
  {
    mApis->add( function );
  }

  for ( const QString &function : std::as_const( mFunctions ) )
  {
    mApis->add( function );
  }

  for ( const QString &fieldName : std::as_const( mFieldNames ) )
  {
    mApis->add( fieldName );
  }

  mApis->add( QString( "NULL" ) );
  mApis->prepare();
  mSqlLexer->setAPIs( mApis );
}

///@cond PRIVATE
QgsLexerExpression::QgsLexerExpression( QObject *parent )
  : QsciLexerSQL( parent )
{
  setBackslashEscapes( true );
}

const char *QgsLexerExpression::language() const
{
  return "QGIS Expression";
}

bool QgsLexerExpression::caseSensitive() const
{
  return false;
}

const char *QgsLexerExpression::wordCharacters() const
{
  return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@";
}

QgsSciApisExpression::QgsSciApisExpression( QsciLexer *lexer )
  : QsciAPIs( lexer )
{

}

QStringList QgsSciApisExpression::callTips( const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList<int> &shifts )
{
  const QStringList originalTips = QsciAPIs::callTips( context, commas, style, shifts );
  QStringList lowercaseTips;
  for ( const QString &tip : originalTips )
    lowercaseTips << tip.toLower();

  return lowercaseTips;
}
///@endcond
