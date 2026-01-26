/***************************************************************************
                         qgsalgorithmpointsinpolygon.h
                         ---------------------
    begin                : November 2019
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

#ifndef QGSALGORITHMPOINTSINPOLYGON_H
#define QGSALGORITHMPOINTSINPOLYGON_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native points in polygon algorithm
 */
class QgsPointsInPolygonAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsPointsInPolygonAlgorithm() = default;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString svgIconPath() const override;
    QIcon icon() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsPointsInPolygonAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;


  protected:
    QString inputParameterName() const override;
    QString inputParameterDescription() const override;
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

  private:
    bool mIsInPlace = false;
    QString mFieldName;
    QString mWeightFieldName;
    QString mClassFieldName;
    int mWeightFieldIndex = -1;
    int mClassFieldIndex = -1;
    mutable int mDestFieldIndex = -1;
    mutable QgsFields mFields;
    mutable QgsCoordinateReferenceSystem mCrs;
    QgsAttributeList mPointAttributes;
    std::unique_ptr<QgsProcessingFeatureSource> mPointSource;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPOINTSINPOLYGON_H
