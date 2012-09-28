class QgsLongLongValidator : QValidator
{
%TypeHeaderCode
#include <qgslonglongvalidator.h>
%End
  public:
    explicit QgsLongLongValidator( QObject *parent );
    QgsLongLongValidator( qint64 bottom, qint64 top, QObject *parent );
    ~QgsLongLongValidator();

    QValidator::State validate( QString &input, int& ) const;

    void setBottom( qint64 bottom );
    void setTop( qint64 top );

    virtual void setRange( qint64 bottom, qint64 top );

    qint64 bottom() const;
    qint64 top() const;
};
