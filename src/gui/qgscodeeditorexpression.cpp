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

#include "qgsapplication.h"
#include "qgscodeeditorexpression.h"
#include "qgssymbollayerutils.h"

#include <QString>
#include <QFont>
#include <QLabel>

QgsCodeEditorExpression::QgsCodeEditorExpression( QWidget *parent )
  : QgsCodeEditor( parent )
{
  if ( !parent )
  {
    setTitle( tr( "Expression Editor" ) );
  }
  setMarginVisible( false );
  setFoldingVisible( true );
  setAutoCompletionCaseSensitivity( false );
  initializeLexer();
}

void QgsCodeEditorExpression::setExpressionContext( const QgsExpressionContext &context )
{
  mVariables.clear();

  const QStringList variableNames = context.filteredVariableNames();
  for ( const QString &var : variableNames )
  {
    mVariables << '@' + var;
  }

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
      const auto &parameters = func->parameters();
      for ( const auto &param : parameters )
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

  mSqlLexer = new QgsLexerExpression( this );
  mSqlLexer->setDefaultFont( font );
  mSqlLexer->setDefaultColor( defaultColor );
  mSqlLexer->setDefaultPaper( colors.value( QStringLiteral( "sql/paperBackgroundColor" ), Qt::white ) );
  mSqlLexer->setFont( font, -1 );
  font.setBold( true );
  mSqlLexer->setFont( font, QsciLexerSQL::Keyword );

  mSqlLexer->setColor( Qt::darkYellow, QsciLexerSQL::DoubleQuotedString ); // fields

  mSqlLexer->setColor( defaultColor, QsciLexerSQL::Default );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/commentFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerSQL::Comment );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/commentLineFontColor" ), QColor( 142, 144, 140 ) ), QsciLexerSQL::CommentLine );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/numberFontColor" ), QColor( 200, 40, 41 ) ), QsciLexerSQL::Number );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/keywordFontColor" ), QColor( 137, 89, 168 ) ), QsciLexerSQL::Keyword );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/singleQuoteFontColor" ), QColor( 113, 140, 0 ) ), QsciLexerSQL::SingleQuotedString );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/doubleQuoteFontColor" ), QColor( 234, 183, 0 ) ), QsciLexerSQL::DoubleQuotedString );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/operatorFontColor" ), QColor( 66, 113, 174 ) ), QsciLexerSQL::Operator );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/identifierFontColor" ), QColor( 62, 153, 159 ) ), QsciLexerSQL::Identifier );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/QuotedIdentifierFontColor" ), Qt::black ), QsciLexerSQL::QuotedIdentifier );
  mSqlLexer->setColor( colors.value( QStringLiteral( "sql/QuotedOperatorFontColor" ), Qt::black ), QsciLexerSQL::QuotedOperator );

  setLexer( mSqlLexer );
}

void QgsCodeEditorExpression::updateApis()
{
  mApis = new QgsSciApisExpression( mSqlLexer );

  for ( const QString &var : qgis::as_const( mVariables ) )
  {
    mApis->add( var );
  }

  for ( const QString &function : qgis::as_const( mContextFunctions ) )
  {
    mApis->add( function );
  }

  for ( const QString &function : qgis::as_const( mFunctions ) )
  {
    mApis->add( function );
  }

  for ( const QString &fieldName : qgis::as_const( mFieldNames ) )
  {
    mApis->add( fieldName );
  }

  mApis->prepare();
  mSqlLexer->setAPIs( mApis );
}

///@cond PRIVATE
QgsLexerExpression::QgsLexerExpression( QObject *parent )
  : QsciLexerSQL( parent )
{
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
