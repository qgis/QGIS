/***************************************************************************
                         qgsalgorithmsumlinelength.h
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

#ifndef QGSALGORITHMSUMLINELENGTH_H
#define QGSALGORITHMSUMLINELENGTH_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsdistancearea.h"
///@cond PRIVATE

/**
 * Native sum line length algorithm
 */
class QgsSumLineLengthAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSumLineLengthAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString svgIconPath() const override;
    QIcon icon() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSumLineLengthAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QList<int> inputLayerTypes() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;


  protected:
    QString inputParameterName() const override;
    QString inputParameterDescription() const override;
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    bool mIsInPlace = false;
    QString mLengthFieldName;
    QString mCountFieldName;
    mutable int mLengthFieldIndex = -1;
    mutable int mCountFieldIndex = -1;
    mutable QgsFields mFields;
    mutable QgsCoordinateReferenceSystem mCrs;
    mutable QgsDistanceArea mDa;
    QgsCoordinateTransformContext mTransformContext;
    std::unique_ptr< QgsProcessingFeatureSource > mLinesSource;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSUMLINELENGTH_H
