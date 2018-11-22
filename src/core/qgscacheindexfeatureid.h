/***************************************************************************
    qgscacheindexfeatureid.h
     --------------------------------------
    Date                 : 13.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCACHEINDEXFEATUREID_H
#define QGSCACHEINDEXFEATUREID_H

#include "qgis_core.h"
#include "qgscacheindex.h"

class QgsVectorLayerCache;

/**
 * \ingroup core
 * \class QgsCacheIndexFeatureId
 */
class CORE_EXPORT QgsCacheIndexFeatureId : public QgsAbstractCacheIndex
{
  public:
    QgsCacheIndexFeatureId( QgsVectorLayerCache * );

    void flushFeature( QgsFeatureId fid ) override;
    void flush() override;
    void requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids ) override;
    bool getCacheIterator( QgsFeatureIterator &featureIterator, const QgsFeatureRequest &featureRequest ) override;

  private:
    QgsVectorLayerCache *C = nullptr;
};

#endif // QGSCACHEINDEXFEATUREID_H
