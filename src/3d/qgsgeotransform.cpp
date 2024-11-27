/***************************************************************************
  qgsgeotransform.cpp
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeotransform.h"
#include "moc_qgsgeotransform.cpp"


///@cond PRIVATE

QgsGeoTransform::QgsGeoTransform( Qt3DCore::QNode *parent )
  : Qt3DCore::QTransform( parent )
{
}

void QgsGeoTransform::setGeoTranslation( const QgsVector3D &translation )
{
  mTranslation = translation;
  setTranslation( ( mTranslation - mOrigin ).toVector3D() );
}

void QgsGeoTransform::setOrigin( const QgsVector3D &origin )
{
  mOrigin = origin;
  setTranslation( ( mTranslation - mOrigin ).toVector3D() );
}

///@endcond
