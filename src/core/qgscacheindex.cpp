/***************************************************************************
    qgscacheindex.cpp
     --------------------------------------
    Date                 : 13.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgscacheindex.h"
#include "qgsfeaturerequest.h"

void QgsAbstractCacheIndex::requestCompleted( const QgsFeatureRequest &featureRequest, const QgsFeatureIds &fids )
{
  Q_UNUSED( featureRequest )
  Q_UNUSED( fids )
}
