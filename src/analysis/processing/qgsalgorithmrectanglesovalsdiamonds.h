/***************************************************************************
                         qgsalgorithmrectanglesovalsdiamonds.h
                         ----------------------
    begin                : January 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#ifndef QGSALGORITHMRECTANGLESOVALSDIAMONDS_H
#define QGSALGORITHMRECTANGLESOVALSDIAMONDS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native rectangles, ovals, diamonds algorithm.
 */
class QgsRectanglesOvalsDiamondsAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsRectanglesOvalsDiamondsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QgsRectanglesOvalsDiamondsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mShape = 0;
    int mSegments = 0;

    double mWidth = 0.0;
    bool mDynamicWidth = false;
    QgsProperty mWidthProperty;

    double mHeight = 0.0;
    bool mDynamicHeight = false;
    QgsProperty mHeightProperty;

    double mRotation = 0.0;
    bool mDynamicRotation = false;
    QgsProperty mRotationProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRECTANGLESOVALSDIAMONDS_H
