/***************************************************************************
                         qgsalgorithmsimplify.h
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

#ifndef QGSALGORITHMSIMPLIFY_H
#define QGSALGORITHMSIMPLIFY_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

class QgsMapToPixelSimplifier;

///@cond PRIVATE

/**
 * Native simplify algorithm.
 */
class QgsSimplifyAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSimplifyAlgorithm() = default;
    ~QgsSimplifyAlgorithm() override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSimplify.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSimplify.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSimplifyAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
  private:

    double mTolerance = 1.0;
    bool mDynamicTolerance = false;
    QgsProperty mToleranceProperty;
    Qgis::VectorSimplificationAlgorithm mMethod = Qgis::VectorSimplificationAlgorithm::Distance;
    std::unique_ptr< QgsMapToPixelSimplifier > mSimplifier;

};


///@endcond PRIVATE

#endif // QGSALGORITHMSIMPLIFY_H


