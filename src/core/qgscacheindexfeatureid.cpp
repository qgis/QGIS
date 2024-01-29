/***************************************************************************
    qgscacheindexfeatureid.cpp
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

#include "qgscacheindexfeatureid.h"
#include "qgsfeaturerequest.h"
#include "qgscachedfeatureiterator.h"
#include "qgsvectorlayercache.h"

QgsCacheIndexFeatureId::QgsCacheIndexFeatureId( QgsVectorLayerCache *cachedVectorLayer )
  : C( cachedVectorLayer )
{

}

void QgsCacheIndexFeatureId::flushFeature( const QgsFeatureId fid )
{
  Q_UNUSED( fid )
}

void QgsCacheIndexFeatureId::flush()
{
}

void QgsCacheIndexFeatureId::requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids )
{
  Q_UNUSED( featureRequest )
  Q_UNUSED( fids )
}

bool QgsCacheIndexFeatureId::getCacheIterator( QgsFeatureIterator &featureIterator, const QgsFeatureRequest &featureRequest )
{
  switch ( featureRequest.filterType() )
  {
    case Qgis::FeatureRequestFilterType::Fid:
    {
      if ( C->isFidCached( featureRequest.filterFid() ) )
      {
        featureIterator = QgsFeatureIterator( new QgsCachedFeatureIterator( C, featureRequest ) );
        return true;
      }
      break;
    }
    case Qgis::FeatureRequestFilterType::Fids:
    {
      if ( C->cachedFeatureIds().contains( featureRequest.filterFids() ) )
      {
        featureIterator = QgsFeatureIterator( new QgsCachedFeatureIterator( C, featureRequest ) );
        return true;
      }
      break;
    }
    case Qgis::FeatureRequestFilterType::NoFilter:
    case Qgis::FeatureRequestFilterType::Expression:
    {
      if ( C->hasFullCache() )
      {
        featureIterator = QgsFeatureIterator( new QgsCachedFeatureIterator( C, featureRequest ) );
        return true;
      }
      break;
    }
  }

  return false;
}

