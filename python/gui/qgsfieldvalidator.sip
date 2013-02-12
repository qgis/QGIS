
class QgsFieldValidator : QValidator
{
%TypeHeaderCode
#include <qgsfieldvalidator.h>
%End

  public:
    QgsFieldValidator( QObject *parent, const QgsField &field );
    ~QgsFieldValidator();

    virtual State validate( QString &, int & ) const;
    virtual void fixup( QString & ) const;
};
