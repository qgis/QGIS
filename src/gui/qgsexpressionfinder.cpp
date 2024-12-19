/*********************************************************************************
    qgsexpressionfinder.cpp - A helper class to locate expression in text editors
     --------------------------------------
    begin                : September 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 *********************************************************************************
 *                                                                               *
 *   This program is free software; you can redistribute it and/or modify        *
 *   it under the terms of the GNU General Public License as published by        *
 *   the Free Software Foundation; either version 2 of the License, or           *
 *   (at your option) any later version.                                         *
 *                                                                               *
 *********************************************************************************/
#include <QRegularExpression>
#include <QPlainTextEdit>
#include <QTextEdit>

#include "qgscodeeditor.h"
#include "qgsexpressionfinder.h"


static const QString EXPRESSION_PATTERN = QStringLiteral( "\\[%\\s*(.*?)\\s*%\\]" );


void QgsExpressionFinder::findExpressionAtPos( const QString &text, int startSelectionPos, int endSelectionPos, int &start, int &end, QString &expression, const QString &pattern )
{
  start = startSelectionPos;
  end = endSelectionPos;
  // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
  expression = text.mid( startSelectionPos, endSelectionPos - startSelectionPos ).replace( QChar( 0x2029 ), QChar( '\n' ) );

  // When the expression is selected including the opening and closing brackets,
  // we still want it to be matched
  if ( startSelectionPos != endSelectionPos )
  {
    startSelectionPos++;
    endSelectionPos--;
  }

  const QRegularExpression regex( pattern.isEmpty() ? EXPRESSION_PATTERN : pattern );
  QRegularExpressionMatchIterator result = regex.globalMatch( text );

  while ( result.hasNext() )
  {
    const QRegularExpressionMatch match = result.next();

    // Check if the selection or cursor is inside the opening and closing brackets
    if ( match.capturedStart() < startSelectionPos && match.capturedEnd() > endSelectionPos )
    {
      start = match.capturedStart();
      end = match.capturedEnd();
      // Set the expression builder text to the trimmed expression
      expression = match.captured( 1 );
      // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
      expression = expression.replace( QChar( 0x2029 ), QChar( '\n' ) );

      break;
    }
  }
}

QString QgsExpressionFinder::findAndSelectActiveExpression( QgsCodeEditor *editor, const QString &pattern )
{
  QString res;

  int startPosition = editor->selectionStart();
  int endPosition = editor->selectionEnd();

  // Find the expression at the cursor position
  int newSelectionStart, newSelectionEnd;
  findExpressionAtPos( editor->text(), startPosition, endPosition, newSelectionStart, newSelectionEnd, res, pattern );

  editor->setLinearSelection( newSelectionStart, newSelectionEnd );

  return res;
}

QString QgsExpressionFinder::findAndSelectActiveExpression( QTextEdit *editor, const QString &pattern )
{
  QString res;

  int startPosition = editor->textCursor().selectionStart();
  int endPosition = editor->textCursor().selectionEnd();

  // Find the expression at the cursor position
  int newSelectionStart, newSelectionEnd;
  findExpressionAtPos( editor->toPlainText(), startPosition, endPosition, newSelectionStart, newSelectionEnd, res, pattern );

  QTextCursor cursor = editor->textCursor();
  cursor.setPosition( newSelectionStart, QTextCursor::MoveAnchor );
  cursor.setPosition( newSelectionEnd, QTextCursor::KeepAnchor );
  editor->setTextCursor( cursor );

  return res;
}

QString QgsExpressionFinder::findAndSelectActiveExpression( QPlainTextEdit *editor, const QString &pattern )
{
  QString res;

  int startPosition = editor->textCursor().selectionStart();
  int endPosition = editor->textCursor().selectionEnd();

  // Find the expression at the cursor position
  int newSelectionStart, newSelectionEnd;
  findExpressionAtPos( editor->toPlainText(), startPosition, endPosition, newSelectionStart, newSelectionEnd, res, pattern );

  QTextCursor cursor = editor->textCursor();
  cursor.setPosition( newSelectionStart, QTextCursor::MoveAnchor );
  cursor.setPosition( newSelectionEnd, QTextCursor::KeepAnchor );
  editor->setTextCursor( cursor );

  return res;
}
