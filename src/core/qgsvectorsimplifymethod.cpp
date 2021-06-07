/***************************************************************************
    qgsvectorsimplifymethod.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsvectorsimplifymethod.h"

QgsVectorSimplifyMethod::QgsVectorSimplifyMethod()
  : mSimplifyHints( Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD > 1 ? QgsVectorSimplifyMethod::FullSimplification : QgsVectorSimplifyMethod::GeometrySimplification )
  , mThreshold( Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD )
{
}
