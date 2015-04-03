
class QgsFieldValidator : QValidator
{
%TypeHeaderCode
#include <qgsfieldvalidator.h>
%End

  public:
    QgsFieldValidator( QObject *parent, const QgsField &field, QString defaultValue, QString dateFormat = "yyyy-MM-dd" );
    ~QgsFieldValidator();

    virtual State validate(QString & /In,Out/, int & /In,Out/) const = 0 /API=QString:2 - /;
    virtual void fixup(QString &input /In,Out/) const /API=QString:2 - /;

    QString dateFormat() const;

    virtual State validate(QString & /Constrained/, int & /In,Out/) const = 0 /API=QString: - 2/;
    virtual void fixup(QString & /Constrained/) const /API=QString: - 2/;
};
