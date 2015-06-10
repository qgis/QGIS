#ifndef QGSFEATUREMODEL_H
#define QGSFEATUREMODEL_H

#include <QModelIndex>
#include "qgsfeature.h" // QgsFeatureId

class QgsFeatureModel
{
  public:
    virtual ~QgsFeatureModel() {}

    virtual QModelIndex fidToIndex( QgsFeatureId fid ) = 0;
};

#endif // QGSFEATUREMODEL_H
