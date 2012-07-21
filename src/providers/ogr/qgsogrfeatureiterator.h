#ifndef QGSOGRFEATUREITERATOR_H
#define QGSOGRFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <ogr_api.h>

class QgsOgrProvider;

class QgsOgrFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsOgrFeatureIterator( QgsOgrProvider* p, const QgsFeatureRequest& request );

    ~QgsOgrFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsOgrProvider* P;

    void ensureRelevantFields();

    void readFeature( OGRFeatureH fet, QgsFeature& feature );

    //! Get an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex );

    //! Selection rectangle
    OGRGeometryH mSelectionRectangle;

    bool mFeatureFetched;
};


#endif // QGSOGRFEATUREITERATOR_H
