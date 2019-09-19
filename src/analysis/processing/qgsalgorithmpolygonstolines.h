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
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native convert polygons to lines algorithm
 */
class QgsPolygonsToLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsPolygonsToLinesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsPolygonsToLinesAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;

  protected:
    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsGeometry convertToLines( const QgsGeometry &geometry ) const;
    QList<QgsCurve *> extractRings( const QgsAbstractGeometry *geom ) const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPOLYGONSTOLINES_H


