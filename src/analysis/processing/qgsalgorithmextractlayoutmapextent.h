/***************************************************************************
                         qgsalgorithmextractlayoutmapextent.h
                         ---------------------
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

#ifndef QGSALGORITHMEXTRACTLAYOUTMAPEXTENT_H
#define QGSALGORITHMEXTRACTLAYOUTMAPEXTENT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native layout map extent to layer algorithm.
 */
class QgsLayoutMapExtentToLayerAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsLayoutMapExtentToLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsLayoutMapExtentToLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters,
                           QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsFeatureList mFeatures;
    double mWidth = 0;
    double mHeight = 0;
    double mScale = 0;
    double mRotation = 0;
    QgsCoordinateReferenceSystem mCrs;

};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTLAYOUTMAPEXTENT_H


