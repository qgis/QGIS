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

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include <QStringList>
#include <QRegularExpression>
#include "qgis_gui.h"

class QTextDocument;

/**
 * \ingroup gui
 * \class QgsExpressionHighlighter
 */
class GUI_EXPORT QgsExpressionHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

  public:
    QgsExpressionHighlighter( QTextDocument *parent = nullptr );
    void addFields( const QStringList &fieldList );

  protected:
    void highlightBlock( const QString &text ) override;

  private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat columnNameFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // QGSEXPRESSIONHIGHLIGHTER_H
