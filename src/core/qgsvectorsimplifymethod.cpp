/***************************************************************************
    qgsvectorsimplifymethod.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsvectorsimplifymethod.h"
#include "qgsvectorlayer.h"

QgsVectorSimplifyMethod::QgsVectorSimplifyMethod()
    : mSimplifyHints( QGis::DEFAULT_MAPTOPIXEL_THRESHOLD > 1 ? QgsVectorSimplifyMethod::FullSimplification : QgsVectorSimplifyMethod::GeometrySimplification )
    , mSimplifyAlgorithm( QgsVectorSimplifyMethod::Distance )
    , mTolerance( 1 )
    , mThreshold( QGis::DEFAULT_MAPTOPIXEL_THRESHOLD )
    , mLocalOptimization( true )
    , mMaximumScale( 1 )
{
}
