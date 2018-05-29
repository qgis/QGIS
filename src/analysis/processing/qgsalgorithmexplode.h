/***************************************************************************
                         qgsalgorithmexplode.h
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMEXPLODE_H
#define QGSALGORITHMEXPLODE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native explode lines algorithm.
 */
class QgsExplodeAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsExplodeAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsExplodeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    std::vector< QgsGeometry > extractAsParts( const QgsGeometry &geometry ) const;
    std::vector< QgsGeometry > curveAsSingleSegments( const QgsCurve *curve, bool useCompoundCurves = false ) const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPLODE_H


