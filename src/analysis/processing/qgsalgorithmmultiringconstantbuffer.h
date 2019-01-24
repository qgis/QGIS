/***************************************************************************
                         qgsalgorithmmultiringconstantbuffer.h
                         -------------------------
    begin                : February 2018
    copyright            : (C) 2018 by Alexander Bruy
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

#ifndef QGSALGORITHMMULTIRINGCONSTANTBUFFER_H
#define QGSALGORITHMMULTIRINGCONSTANTBUFFER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native multiring buffer algorithm.
 */
class QgsMultiRingConstantBufferAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsMultiRingConstantBufferAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsMultiRingConstantBufferAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:

    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorPolygon; }
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override { Q_UNUSED( inputWkbType ); return QgsWkbTypes::MultiPolygon; }
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mRingsNumber = 0;
    bool mDynamicRingsNumber = false;
    QgsProperty mRingsNumberProperty;

    double mDistance = 0.0;
    bool mDynamicDistance = false;
    QgsProperty mDistanceProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMMULTIRINGCONSTANTBUFFER_H


