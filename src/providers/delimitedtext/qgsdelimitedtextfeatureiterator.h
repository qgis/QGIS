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

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

    // Tests whether the geometry is required, given that testGeometry is true.
    bool wantGeometry( const QgsPoint & point ) const;
    bool wantGeometry( QgsGeometry *geom ) const;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    bool setNextFeatureId( qint64 fid );

    bool nextFeatureInternal( QgsFeature& feature );
    QgsGeometry* loadGeometryWkt( const QStringList& tokens );
    QgsGeometry* loadGeometryXY( const QStringList& tokens );
    void fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens );

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
