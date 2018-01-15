/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionhighlighter.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsExpressionHighlighter : QSyntaxHighlighter
{

%TypeHeaderCode
#include "qgsexpressionhighlighter.h"
%End
  public:
    QgsExpressionHighlighter( QTextDocument *parent = 0 );
    void addFields( const QStringList &fieldList );

  protected:
    virtual void highlightBlock( const QString &text );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionhighlighter.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
