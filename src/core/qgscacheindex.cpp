/***************************************************************************
    qgscacheindex.cpp
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

#include "qgscacheindex.h"
#include "qgsfeaturerequest.h"

QgsAbstractCacheIndex::QgsAbstractCacheIndex()
{
}

QgsAbstractCacheIndex::~QgsAbstractCacheIndex()
{
}

void QgsAbstractCacheIndex::requestCompleted( const QgsFeatureRequest& featureRequest, const QgsFeatureIds& fids )
{
  Q_UNUSED( featureRequest )
  Q_UNUSED( fids )
}
