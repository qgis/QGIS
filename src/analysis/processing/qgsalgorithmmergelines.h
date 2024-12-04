/***************************************************************************
                         qgsalgorithmmergelines.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSALGORITHMMERGELINES_H
#define QGSALGORITHMMERGELINES_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

/**
 * Native merge lines algorithm.
 */
class QgsMergeLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsMergeLinesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsMergeLinesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMMERGELINES_H
