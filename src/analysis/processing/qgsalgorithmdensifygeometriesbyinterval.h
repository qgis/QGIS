/***************************************************************************
                         qgsalgorithmdensifygeometries.h
                         ---------------------
    begin                : January 2019
    copyright            : (C) 2019 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMDENSIFYGEOMETRIES_H
#define QGSALGORITHMDENSIFYGEOMETRIES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


class QgsDensifyGeometriesByIntervalAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:

    QgsDensifyGeometriesByIntervalAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDensifyGeometriesByIntervalAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mInterval = 0.0;
    bool mDynamicInterval = false;
    QgsProperty mIntervalProperty;
};

///@endcond PRIVATE
#endif // QGSALGORITHMDENSIFYGEOMETRIES_H
