/***************************************************************************
    qgspointsample.h
    ---------------------
    begin                : July 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTSAMPLE_H
#define QGSPOINTSAMPLE_H

#include "qgsfeature.h"
#include <QString>

class QgsFeature;
class QgsPoint;
class QgsSpatialIndex;
class QgsVectorFileWriter;
class QgsVectorLayer;
class QProgressDialog;

/** \ingroup analysis
 * Creates random points in polygons / multipolygons*/
class ANALYSIS_EXPORT QgsPointSample
{
  public:
    QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, const QString& nPointsAttribute, const QString& minDistAttribute = QString() );

    /** Starts calculation of random points
        @return 0 in case of success*/
    int createRandomPoints( QProgressDialog* pd );

  private:

    QgsPointSample(); //default constructor is forbidden
    void addSamplePoints( QgsFeature& inputFeature, QgsVectorFileWriter& writer, int nPoints, double minDistance );
    bool checkMinDistance( QgsPoint& pt, QgsSpatialIndex& index, double minDistance, QMap< QgsFeatureId, QgsPoint >& pointMap );

    /** Layer id of input polygon/multipolygon layer*/
    QgsVectorLayer* mInputLayer;
    /** Output path of result layer*/
    QString mOutputLayer;
    /** Attribute containing number of points per feature*/
    QString mNumberOfPointsAttribute;
    /** Attribute containing minimum distance between sample points (or -1 if no min. distance constraint)*/
    QString mMinDistanceAttribute;
    QgsFeatureId mNCreatedPoints; //helper to find free ids
};

#endif // QGSPOINTSAMPLE_H
