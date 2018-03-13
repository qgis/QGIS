/***************************************************************************
                         qgsalgorithmrotate.h
                         ---------------------
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

#ifndef QGSALGORITHMROTATE_H
#define QGSALGORITHMROTATE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native rotate features algorithm.
 */
class QgsRotateFeaturesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsRotateFeaturesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsRotateFeaturesAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    double mAngle = 0.0;
    bool mDynamicAngle = false;
    QgsProperty mAngleProperty;

    bool mUseAnchor = false;
    QgsPointXY mAnchor;
    QgsCoordinateReferenceSystem mAnchorCrs;
    bool mTransformedAnchor = false;

};


///@endcond PRIVATE

#endif // QGSALGORITHMROTATE_H


