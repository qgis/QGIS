/***************************************************************************
    qgsdelimitedtextfeatureiterator.h
    ---------------------
    begin                : Oktober 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDELIMITEDTEXTFEATUREITERATOR_H
#define QGSDELIMITEDTEXTFEATUREITERATOR_H

#include <QList>
#include "qgsfeatureiterator.h"
#include "qgsfeature.h"

class QgsDelimitedTextProvider;

class QgsDelimitedTextFeatureIterator : public QgsAbstractFeatureIterator
{
    enum IteratorMode
    {
      FileScan,
      SubsetIndex,
      FeatureIds
    };
  public:
    QgsDelimitedTextFeatureIterator( QgsDelimitedTextProvider* p, const QgsFeatureRequest& request );

    ~QgsDelimitedTextFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

    // Flags used by nextFeature function of QgsDelimitedTextProvider
    bool testSubset() const { return mTestSubset; }
    bool testGeometry() const { return mTestGeometry; }
    bool loadGeometry() const { return mLoadGeometry; }
    bool loadSubsetOfAttributes() const { return ! mTestSubset && mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;}
    bool scanningFile() const { return mMode == FileScan; }

    // Pass through attribute subset
    const QgsAttributeList &subsetOfAttributes() const { return mRequest.subsetOfAttributes(); }

    // Tests whether the geometry is required, given that testGeometry is true.
    bool wantGeometry( const QgsPoint & point ) const;
    bool wantGeometry( QgsGeometry *geom ) const;

  protected:
    QgsDelimitedTextProvider* P;
    QList<QgsFeatureId> mFeatureIds;
    IteratorMode mMode;
    long mNextId;
    bool mTestSubset;
    bool mTestGeometry;
    bool mTestGeometryExact;
    bool mLoadGeometry;
};


#endif // QGSDELIMITEDTEXTFEATUREITERATOR_H
