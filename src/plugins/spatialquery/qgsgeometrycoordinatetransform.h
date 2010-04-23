/***************************************************************************
                          qgsgeometrycoordinatetransform.h
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
/*  $Id: $ */
#ifndef GEOMETRYCOORDINATETRANSFORM_H
#define GEOMETRYCOORDINATETRANSFORM_H

#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatetransform.h"

/**
* \class QgsGeometryCoordinateTransform
* \brief Transform the coordinate reference system of the geometry
*/
class QgsGeometryCoordinateTransform
{
public:
    /**
    * \brief Constructor for a Geometry Coordinate Transform.
    *
    */
    QgsGeometryCoordinateTransform () {};

    /**
    * \brief Destructor
    */
    ~QgsGeometryCoordinateTransform ();

    /**
    * \brief Sets the coordinate reference system the target and reference layer
    * \param lyrTarget      target layer.
    * \param lyrReference   reference layer.
    */
    void setCoordinateTransform(QgsVectorLayer* lyrTarget, QgsVectorLayer* lyrReference);

    /**
    * \brief Transform the coordinates reference system of the geometry, if target have the different system of reference
    * \param geom      Geometry
    */
    void transform(QgsGeometry *geom);
private:
    /**
    * \brief Transform the coordinates reference system of the geometry (use by transform)
    * \param geom      Geometry
    */
    void setGeomTransform(QgsGeometry *geom);
    /**
    * \brief None transform the coordinates reference system of the geometry (use by transform)
    * \param geom      Geometry
    */
    void setNoneGeomTransform(QgsGeometry *geom) {};

    QgsCoordinateTransform * mCoordTransform;
    void (QgsGeometryCoordinateTransform::* mFuncTransform)(QgsGeometry *);
};

#endif // GEOMETRYCOORDINATETRANSFORM_H
