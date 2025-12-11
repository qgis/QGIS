/***************************************************************************
                         qgsalgorithmpolygontolines.h
                         ---------------------
    begin                : January 2019
    copyright            : (C) 2019 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMPOLYGONSTOLINES_H
#define QGSALGORITHMPOLYGONSTOLINES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native convert polygons to lines algorithm
 */
class QgsPolygonsToLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsPolygonsToLinesAlgorithm() = default;
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmPolygonToLine.svg" ) ); }
    [[nodiscard]] QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmPolygonToLine.svg" ) ); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsPolygonsToLinesAlgorithm *createInstance() const override SIP_FACTORY;
    [[nodiscard]] QList<int> inputLayerTypes() const override;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    [[nodiscard]] QgsGeometry convertToLines( const QgsGeometry &geometry ) const;
    QList<QgsCurve *> extractRings( const QgsAbstractGeometry *geom ) const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPOLYGONSTOLINES_H
