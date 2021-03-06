/***************************************************************************
     qgsvectorwarper.h
     --------------------------------------
    Date                 :
    Copyright            :
    Email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORWARPER_H
#define QGSVECTORWARPER_H

#include <QCoreApplication>
#include <QString>
#include "qgsgcpgeometrytransformer.h"

#include <vector>

class QgsCoordinateReferenceSystem;
class QgsGCPList;
class QWidget;
class QgsFeedback;

/**
 * Base implementation of QgsVectorWarper.
 * \param method QgsGcpTransofmer method, based on gdal transformation methods
 * \param points data points used in the georeferencer
 * \param destCrs destination projection of the vector layer
 * \since QGIS 3.20
 */
class QgsVectorWarper
{

  public:
    explicit QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod &method, const QgsGCPList &points,
                              const QgsCoordinateReferenceSystem &destCrs );

    /**
     * Functions to reproject features of the vector layer
     * \param layer source vector layer
     * \param feedback optional qgsfeedback instance
     * \return True if operation finished properly, otherwise false.
     * \since QGIS 3.20
     */
    bool executeTransformInplace( QgsVectorLayer *layer, QgsFeedback *feedback = nullptr );

    /**
     * Functions to reproject features of the vector layer to a new source
     * \param layer source vector layer
     * \param feedback optional qgsfeedback instance
     * \return True if operation finished properly, otherwise false.
     * \since QGIS 3.20
     */
    bool executeTransform( const QgsVectorLayer *layer, const QString outputName, QgsFeedback *feedback = nullptr );

  private:
    QgsGcpGeometryTransformer *mTransformer;
    QgsCoordinateReferenceSystem mDestCRS;

};


#endif
