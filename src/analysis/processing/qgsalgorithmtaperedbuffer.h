/***************************************************************************
                         qgsalgorithmtaperedbuffer.h
                         ---------------------------
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

#ifndef QGSALGORITHMTAPEREDBUFFER_H
#define QGSALGORITHMTAPEREDBUFFER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native tapered buffer algorithm.
 */
class QgsTaperedBufferAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsTaperedBufferAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsTaperedBufferAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mStartWidth = 0.0;
    bool mDynamicStartWidth = false;
    QgsProperty mStartWidthProperty;

    double mEndWidth = 1.0;
    bool mDynamicEndWidth = false;
    QgsProperty mEndWidthProperty;

    int mSegments = 16;
    bool mDynamicSegments = false;
    QgsProperty mSegmentsProperty;

};


/**
 * Native buffer by m algorithm.
 */
class QgsVariableWidthBufferByMAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsVariableWidthBufferByMAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsVariableWidthBufferByMAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    int mSegments = 16;
    bool mDynamicSegments = false;
    QgsProperty mSegmentsProperty;

};
///@endcond PRIVATE

#endif // QGSALGORITHMTAPEREDBUFFER_H


