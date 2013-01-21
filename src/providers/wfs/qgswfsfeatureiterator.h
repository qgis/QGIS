#ifndef QGSWFSFEATUREITERATOR_H
#define QGSWFSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

class QgsWFSProvider;

class QgsWFSFeatureIterator: public QgsAbstractFeatureIterator
{
  public:
    QgsWFSFeatureIterator( QgsWFSProvider* provider, const QgsFeatureRequest& request );
    ~QgsWFSFeatureIterator();

    bool nextFeature( QgsFeature& f );
    bool rewind();
    bool close();

  private:
    QgsWFSProvider* mProvider;
    QList<QgsFeatureId> mSelectedFeatures;
    QList<QgsFeatureId>::const_iterator mFeatureIterator;
};

#endif // QGSWFSFEATUREITERATOR_H
