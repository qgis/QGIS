/***************************************************************************
    qgsexpressionhighlighter.cpp - A syntax highlighter for a qgsexpression
     --------------------------------------
    Date                 :  28-Dec-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionhighlighter.h"

QgsExpressionHighlighter::QgsExpressionHighlighter( QTextDocument *parent )
    : QSyntaxHighlighter( parent )
{
  HighlightingRule rule;

  keywordFormat.setForeground( Qt::darkBlue );
  keywordFormat.setFontWeight( QFont::Bold );
  QStringList keywordPatterns;
  keywordPatterns << "\\bCASE\\b" << "\\bWHEN\\b" << "\\bTHEN\\b"
  << "\\bELSE\\b" << "\\bEND\\b";

  foreach( const QString &pattern, keywordPatterns )
  {
    rule.pattern = QRegExp( pattern, Qt::CaseInsensitive );
    rule.format = keywordFormat;
    highlightingRules.append( rule );
  }

  quotationFormat.setForeground( Qt::darkGreen );
  rule.pattern = QRegExp( "\'[^\'\r\n]*\'" );
  rule.format = quotationFormat;
  highlightingRules.append( rule );

  columnNameFormat.setForeground( Qt::darkRed );
  rule.pattern = QRegExp( "\"[^\"\r\n]*\"" );
  rule.format = columnNameFormat;
  highlightingRules.append( rule );
}

void QgsExpressionHighlighter::addFields( QStringList fieldList )
{
  columnNameFormat.setForeground( Qt::darkRed );
  HighlightingRule rule;
  foreach( const QString field, fieldList )
  {
    if ( field.isEmpty() ) // this really happened :)
      continue;
    rule.pattern = QRegExp( "\\b" + field + "\\b" );
    rule.format = columnNameFormat;
    highlightingRules.append( rule );
  }
}

void QgsExpressionHighlighter::highlightBlock( const QString &text )
{
  foreach( const HighlightingRule &rule, highlightingRules )
  {
    QRegExp expression( rule.pattern );
    int index = expression.indexIn( text );
    while ( index >= 0 )
    {
      int length = expression.matchedLength();
      if ( length == 0 )
        break; // avoid infinite loops
      setFormat( index, length, rule.format );
      index = expression.indexIn( text, index + length );
    }
  }
}

