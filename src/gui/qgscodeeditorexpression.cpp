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
  QFont font = getMonospaceFont();
#ifdef Q_OS_MAC
  // The font size gotten from getMonospaceFont() is too small on Mac
  font.setPointSize( QLabel().font().pointSize() );
#endif
  mSqlLexer = new QgsCaseInsensitiveLexerExpression( this );
  mSqlLexer->setDefaultFont( font );
  mSqlLexer->setFont( font, -1 );
  font.setBold( true );
  mSqlLexer->setFont( font, QsciLexerSQL::Keyword );
  mSqlLexer->setColor( Qt::darkYellow, QsciLexerSQL::DoubleQuotedString ); // fields

  setLexer( mSqlLexer );
}

void QgsCodeEditorExpression::updateApis()
{
  mApis = new QsciAPIs( mSqlLexer );

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

QgsCaseInsensitiveLexerExpression::QgsCaseInsensitiveLexerExpression( QObject *parent )
  : QsciLexerSQL( parent )
{
}

bool QgsCaseInsensitiveLexerExpression::caseSensitive() const
{
  return false;
}

const char *QgsCaseInsensitiveLexerExpression::wordCharacters() const
{
  return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@";
}
