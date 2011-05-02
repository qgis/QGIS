
class QgsFieldValidator : QValidator
{
%TypeHeaderCode
#include <qgsfieldvalidator.h>
%End
    QgsFieldValidator( QObject *parent, const QgsField &field );
    ~QgsFieldValidator();

    virtual State validate( QString &, int & ) const;
    virtual void fixup( QString & ) const;
};
