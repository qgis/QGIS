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

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

/**
 * Native simplify algorithm.
 */
class QgsSimplifyAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSimplifyAlgorithm() = default;
    Flags flags() const override;
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
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;

  private:

    double mTolerance = 1.0;
    QgsMapToPixelSimplifier::SimplifyAlgorithm mMethod = QgsMapToPixelSimplifier::Distance;
    std::unique_ptr< QgsMapToPixelSimplifier > mSimplifier;

};


///@endcond PRIVATE

#endif // QGSALGORITHMSIMPLIFY_H


