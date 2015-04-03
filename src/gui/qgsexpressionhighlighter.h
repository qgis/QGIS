/***************************************************************************
    qgsexpressionhighlighter.h - A syntax highlighter for a qgsexpression
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

#ifndef QGSEXPRESSIONHIGHLIGHTER_H
#define QGSEXPRESSIONHIGHLIGHTER_H

#include "qgsfield.h"
#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include <QStringList>

class QTextDocument;

class GUI_EXPORT QgsExpressionHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

  public:
    QgsExpressionHighlighter( QTextDocument *parent = 0 );
    void addFields( QStringList fieldList );

  protected:
    void highlightBlock( const QString &text ) override;

  private:
    struct HighlightingRule
    {
      QRegExp pattern;
      QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat columnNameFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // QGSEXPRESSIONHIGHLIGHTER_H
