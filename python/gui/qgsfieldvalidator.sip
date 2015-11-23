
class QgsFieldValidator : QValidator
{
%TypeHeaderCode
#include <qgsfieldvalidator.h>
%End

  public:
    QgsFieldValidator( QObject *parent, const QgsField &field, const QString& defaultValue, const QString& dateFormat = "yyyy-MM-dd" );
    ~QgsFieldValidator();


    QString dateFormat() const;

    virtual State validate(QString & /Constrained/, int & /In,Out/) const = 0;
    virtual void fixup(QString & /Constrained/) const;
};
