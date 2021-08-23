/***************************************************************************
                         qgsalgorithmsinglesidedbuffer.h
                         ---------------------
    begin                : November 2019
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

#ifndef QGSALGORITHMSINGLESIDEDBUFFER_H
#define QGSALGORITHMSINGLESIDEDBUFFER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native single-sided buffer algorithm.
 */
class QgsSingleSidedBufferAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSingleSidedBufferAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsSingleSidedBufferAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:

    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mDistance = 0.0;
    bool mDynamicDistance = false;
    QgsProperty mDistanceProperty;

    int mSegments = 0;
    Qgis::BufferSide mSide;
    Qgis::JoinStyle mJoinStyle;
    double mMiterLimit = 0.0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSINGLESIDEDBUFFER_H


