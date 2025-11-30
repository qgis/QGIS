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
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Native convex hull algorithm.
 */
class QgsConvexHullAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsConvexHullAlgorithm() = default;
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmConvexHull.svg" ) ); }
    [[nodiscard]] QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmConvexHull.svg" ) ); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsConvexHullAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType ) const override { return Qgis::WkbType::Polygon; }
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMCONVEXHULL_H
