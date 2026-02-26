/***************************************************************************
                         qgsalgorithmforcecw.h
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Andrea Giudiceandrea
    email                : andreaerdna at libero dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMFORCECW_H
#define QGSALGORITHMFORCECW_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Native force polygons clockwise algorithm.
 */
class QgsForceCWAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsForceCWAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QList<int> inputLayerTypes() const override;
    QgsForceCWAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    QString outputName() const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFORCECW_H
