/***************************************************************************
    qgscacheindexfeatureid.cpp
     --------------------------------------
    Date                 : 13.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscacheindexfeatureid.h"

QgsCacheIndexFeatureId::QgsCacheIndexFeatureId( QgsCachedVectorLayer* cachedVectorLayer )
    : QgsAbstractCacheIndex()
    , C( cachedVectorLayer )
{

}

