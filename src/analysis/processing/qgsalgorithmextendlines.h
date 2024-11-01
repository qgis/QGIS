/***************************************************************************
                         qgsalgorithmextendlines.h
                         ---------------------
    begin                : July 2018
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

#ifndef QGSEXTENDLINESALGORITHM_H
#define QGSEXTENDLINESALGORITHM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native extend lines algorithm.
 */
class QgsExtendLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExtendLinesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    QgsExtendLinesAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;

  private:
    double mStartDistance = 0.0;
    bool mDynamicStartDistance = false;
    QgsProperty mStartDistanceProperty;

    double mEndDistance = 0.0;
    bool mDynamicEndDistance = false;
    QgsProperty mEndDistanceProperty;
};


///@endcond PRIVATE

#endif // QGSEXTENDLINESALGORITHM_H
