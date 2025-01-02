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

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE


/**
 * Native convex hull algorithm.
 */
class QgsConvexHullAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsConvexHullAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmConvexHull.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmConvexHull.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConvexHullAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType ) const override { return Qgis::WkbType::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMCONVEXHULL_H
