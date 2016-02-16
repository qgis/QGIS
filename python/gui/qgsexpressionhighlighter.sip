class QgsExpressionHighlighter : QSyntaxHighlighter
{
%TypeHeaderCode
#include "qgsexpressionhighlighter.h"
%End

  public:
    QgsExpressionHighlighter( QTextDocument *parent = 0 );
    void addFields( const QStringList& fieldList );

  protected:
    void highlightBlock( const QString &text );
};
