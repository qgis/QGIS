#ifndef QGSDELIMITEDTEXTFEATUREITERATOR_H
#define QGSDELIMITEDTEXTFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

class QgsDelimitedTextProvider;

class QgsDelimitedTextFeatureIterator : public QgsAbstractFeatureIterator
{
public:
    QgsDelimitedTextFeatureIterator( QgsDelimitedTextProvider* p, const QgsFeatureRequest& request );

    ~QgsDelimitedTextFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsDelimitedTextProvider* P;

    //! Feature id
    long mFid;

    QgsGeometry* loadGeometryWkt( const QStringList& tokens );
    QgsGeometry* loadGeometryXY( const QStringList& tokens );

    bool boundsCheck( double x, double y );
    bool boundsCheck( QgsGeometry *geom );

    void fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens );
};


#endif // QGSDELIMITEDTEXTFEATUREITERATOR_H
