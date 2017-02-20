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
#include <memory>

class QgsVirtualLayerFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsVirtualLayerFeatureSource( const QgsVirtualLayerProvider* p );
    ~QgsVirtualLayerFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

    const QgsVirtualLayerProvider* provider() const { return mProvider; }
  private:
    const QgsVirtualLayerProvider* mProvider = nullptr;
};

class QgsVirtualLayerFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsVirtualLayerFeatureSource>
{
  public:
    QgsVirtualLayerFeatureIterator( QgsVirtualLayerFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );
    ~QgsVirtualLayerFeatureIterator();

    virtual bool rewind() override;
    virtual bool close() override;

  protected:

    virtual bool fetchFeature( QgsFeature& feature ) override;

    std::unique_ptr<Sqlite::Query> mQuery;

    QgsFeatureId mFid;

    QString mPath;
    sqlite3* mSqlite = nullptr;
    QgsVirtualLayerDefinition mDefinition;
    QgsFields mFields;

    QString mSqlQuery;

    QgsAttributeList mAttributes;
};

#endif
