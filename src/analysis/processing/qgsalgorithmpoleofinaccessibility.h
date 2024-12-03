/***************************************************************************
                         qgsalgorithmpoleofinaccessibility.h
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMPOLEOFINACCESSIBILITY_H
#define QGSALGORITHMPOLEOFINACCESSIBILITY_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native pole of inaccessibility algorithm.
 */
class QgsPoleOfInaccessibilityAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsPoleOfInaccessibilityAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString svgIconPath() const override;
    QIcon icon() const override;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QgsPoleOfInaccessibilityAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mTolerance = 0.0;
    bool mDynamicTolerance = false;
    QgsProperty mToleranceProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPOLEOFINACCESSIBILITY_H
