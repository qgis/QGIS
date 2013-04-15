#ifndef QGSFEATUREMODEL_H
#define QGSFEATUREMODEL_H

#include "qgsfeature.h" // QgsFeatureId
#include <QModelIndex>

class QgsFeatureModel
{
  public:
    virtual QModelIndex fidToIndex( QgsFeatureId fid ) = 0;
};

#endif // QGSFEATUREMODEL_H
