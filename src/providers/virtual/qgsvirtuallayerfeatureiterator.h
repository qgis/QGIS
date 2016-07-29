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


#include <qgsvirtuallayerprovider.h>
#include <qgsfeatureiterator.h>

class QgsVirtualLayerFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsVirtualLayerFeatureSource( const QgsVirtualLayerProvider* p );
    ~QgsVirtualLayerFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

    const QgsVirtualLayerProvider* provider() const { return mProvider; }
  private:
    const QgsVirtualLayerProvider* mProvider;
};

class QgsVirtualLayerFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsVirtualLayerFeatureSource>
{
  public:
    QgsVirtualLayerFeatureIterator( QgsVirtualLayerFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );
    ~QgsVirtualLayerFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    QScopedPointer<Sqlite::Query> mQuery;

    QgsFeatureId mFid;

    QString mPath;
    sqlite3* mSqlite;
    QgsVirtualLayerDefinition mDefinition;
    QgsFields mFields;

    QString mSqlQuery;

    QgsAttributeList mAttributes;
};

#endif
