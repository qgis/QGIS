/***************************************************************************
                         qgsalgorithmoffsetlines.h
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

#ifndef QGSALGORITHMOFFSETLINES_H
#define QGSALGORITHMOFFSETLINES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native offset lines algorithm.
 */
class QgsOffsetLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsOffsetLinesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsOffsetLinesAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mOffset = 0.0;
    bool mDynamicOffset = false;
    QgsProperty mOffsetProperty;

    int mSegments = 8;
    QgsGeometry::JoinStyle mJoinStyle = QgsGeometry::JoinStyleRound;
    double mMiterLimit = 2;


};


///@endcond PRIVATE

#endif // QGSALGORITHMOFFSETLINES_H


