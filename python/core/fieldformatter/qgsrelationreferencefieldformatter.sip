class QgsRelationReferenceFieldFormatter : QgsFieldFormatter
{
%TypeHeaderCode
#include "qgsrelationreferencefieldformatter.h"
%End
  public:
    QgsRelationReferenceFieldFormatter();

    QString id() const;
    virtual QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;
    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap&config, const QVariant& cache, const QVariant& value ) const;
};
