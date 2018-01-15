/***************************************************************************
                         qgsalgorithmcentroid.h
                         ----------------------
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

#ifndef QGSALGORITHMCENTROID_H
#define QGSALGORITHMCENTROID_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native centroid algorithm.
 */
class QgsCentroidAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsCentroidAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsCentroidAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorPoint; }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override { Q_UNUSED( inputWkbType ); return QgsWkbTypes::Point; }

    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCENTROID_H


