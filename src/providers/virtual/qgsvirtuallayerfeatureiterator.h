/***************************************************************************
                   qgsvirtuallayerfeatureiterator.h
            Feature iterator for the virtual layer provider
begin                : Feb 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVIRTUALLAYER_FEATURE_ITERATOR_H
#define QGSVIRTUALLAYER_FEATURE_ITERATOR_H


#include "qgsvirtuallayerprovider.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometryengine.h"
#include "qgscoordinatetransform.h"

#include <memory>
#include <QPointer>

class QgsVirtualLayerFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    QgsVirtualLayerFeatureSource( const QgsVirtualLayerProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:

    // NOTE: this is really bad and should be removed.
    // it's only here to guard mSqlite - because if the provider is removed
    // then mSqlite will be meaningless.
    // this needs to be totally reworked so that mSqlite no longer depends on the provider
    // and can be fully encapsulated here
    QPointer< const QgsVirtualLayerProvider  > mProvider;

    QString mPath;
    QgsVirtualLayerDefinition mDefinition;
    QgsFields mFields;
    sqlite3 *mSqlite = nullptr;
    QString mTableName;
    QString mSubset;
    QgsCoordinateReferenceSystem mCrs;

    friend class QgsVirtualLayerFeatureIterator;
};

class QgsVirtualLayerFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsVirtualLayerFeatureSource>
{
  public:
    QgsVirtualLayerFeatureIterator( QgsVirtualLayerFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );
    ~QgsVirtualLayerFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:

    bool fetchFeature( QgsFeature &feature ) override;

  private:

    std::unique_ptr<Sqlite::Query> mQuery;

    QgsAttributeList mAttributes;
    QString mSqlQuery;
    QgsFeatureId mFid = 0;
    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
    std::unique_ptr< QgsGeometryEngine > mRectEngine;
};

#endif
