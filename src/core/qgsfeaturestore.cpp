/***************************************************************************
     qgsfeaturestore.cpp
     --------------------------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Radim Blazek
    Email                : radim.blazek@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturestore.h"

QgsFeatureStore::QgsFeatureStore()
{
}

QgsFeatureStore::QgsFeatureStore( const QgsFields& fields, const QgsCoordinateReferenceSystem& crs )
    : mFields( fields )
    , mCrs( crs )
{
}

void QgsFeatureStore::setFields( const QgsFields & fields )
{
  mFields = fields;
  QgsFeatureList::iterator it = mFeatures.begin();
  for ( ; it != mFeatures.end(); ++it )
  {
    ( *it ).setFields( mFields );
  }
}

void QgsFeatureStore::addFeature( const QgsFeature& feature )
{
  QgsFeature f( feature );
  f.setFields( mFields );
  mFeatures.append( f );
}
