/***************************************************************************
                         qgsalgorithmpointonsurface.h
                         ----------------------
    begin                : March 2018
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

#ifndef QGSALGORITHMPOINTONSURFACE_H
#define QGSALGORITHMPOINTONSURFACE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native centroid algorithm.
 */
class QgsPointOnSurfaceAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsPointOnSurfaceAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsPointOnSurfaceAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:

    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorPoint; }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override { Q_UNUSED( inputWkbType ); return QgsWkbTypes::Point; }

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    bool mAllParts = false;
    bool mDynamicAllParts = false;
    QgsProperty mAllPartsProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMPOINTONSURFACE_H


