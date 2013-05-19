class QgsErrorMessage
{
%TypeHeaderCode
#include <qgserror.h>
%End
  public:
    enum Format
    {
      Text, // Plain text
      Html
    };

    QgsErrorMessage();

    QgsErrorMessage( const QString & theMessage, const QString & theTag = QString::null, const QString & theFile = QString::null, const QString & theFunction = QString::null, int theLine = 0 );

    QString message() const;
    QString tag() const;
    QString file() const;
    QString function() const;
    int line() const;
};

class QgsError
{
%TypeHeaderCode
#include <qgserror.h>
%End
  public:

    QgsError();

    QgsError( const QString & theMessage, const QString & theTag );

    void append( const QString & theMessage, const QString & theTag );

    void append( const QgsErrorMessage & theMessage );

    bool isEmpty() const;

    QString message( QgsErrorMessage::Format theFormat = QgsErrorMessage::Html ) const;

    QString summary() const;

    void clear();
};

