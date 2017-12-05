/***************************************************************************
                         qgsalgorithmconvexhull.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCONVEXHULL_H
#define QGSALGORITHMCONVEXHULL_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Native convex hull algorithm.
 */
class QgsConvexHullAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsConvexHullAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConvexHullAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};


///@endcond PRIVATE

#endif // QGSALGORITHMCONVEXHULL_H


