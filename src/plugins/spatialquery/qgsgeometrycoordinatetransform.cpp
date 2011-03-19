/***************************************************************************
                          qgsgeometrycoordinatetransform.cpp
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#include "qgsgeometrycoordinatetransform.h"

#include "qgscoordinatereferencesystem.h"

QgsGeometryCoordinateTransform::~QgsGeometryCoordinateTransform()
{
  delete mCoordTransform;

} // QgsGeometryCoordinateTransform::~QgsGeometryCoordinateTransform()

void QgsGeometryCoordinateTransform::setCoordinateTransform( QgsVectorLayer* lyrTarget, QgsVectorLayer* lyrReference )
{
  // Transform Forward: Target to Reference
  QgsCoordinateReferenceSystem srsTarget = lyrTarget->crs();
  QgsCoordinateReferenceSystem srsReference = lyrReference->crs();

  mCoordTransform = new QgsCoordinateTransform( srsTarget, srsReference );

  mFuncTransform = ( srsTarget != srsReference )
                   ? &QgsGeometryCoordinateTransform::setGeomTransform
                   : &QgsGeometryCoordinateTransform::setNoneGeomTransform;

} // void QgsGeometryCoordinateTransform::setCoordinateTransform(QgsVectorLayer* lyrTarget, QgsVectorLayer* lyrReference)

void QgsGeometryCoordinateTransform::transform( QgsGeometry *geom )
{
  ( this->*mFuncTransform )( geom );

} // void QgsGeometryCoordinateTransform::transformCoordenate()

void QgsGeometryCoordinateTransform::setGeomTransform( QgsGeometry *geom )
{
  geom->transform( *mCoordTransform );

} // void QgsGeometryCoordinateTransform::setGeomTransform(QgsGeometry *geom)
