/***************************************************************************
                         qgsalgorithmaddixyfields.h
                         ---------------------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSALGORITHMADDXYFIELDS_H
#define QGSALGORITHMADDXYFIELDS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native add X/Y fields algorithm.
 */
class QgsAddXYFieldsAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsAddXYFieldsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsAddXYFieldsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:

    QString mPrefix;
    mutable QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransform mTransform;
    bool mTransformNeedsInitialization = true;

};

///@endcond PRIVATE

#endif // QGSALGORITHMADDXYFIELDS_H


