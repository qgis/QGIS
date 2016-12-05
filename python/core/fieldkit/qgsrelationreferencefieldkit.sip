class QgsRelationReferenceFieldKit : QgsFieldKit
{
%TypeHeaderCode
#include "qgsrelationreferencefieldkit.h"
%End
  public:
    QgsRelationReferenceFieldKit();

    QString id() const;
    virtual QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;
    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap&config, const QVariant& cache, const QVariant& value ) const;
};
