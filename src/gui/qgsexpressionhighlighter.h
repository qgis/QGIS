/***************************************************************************
    qgsexpressionhighlighter.h - A syntax highlighter for a qgsexpression
     --------------------------------------
    Date                 :  28-Dec-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONHIGHLIGHTER_H
#define QGSEXPRESSIONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include <QStringList>
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
