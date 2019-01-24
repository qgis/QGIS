/***************************************************************************
                         qgsalgorithmsnaptogrid.h
                         ---------------------
    begin                : October 2017
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

#ifndef QGSALGORITHMSNAPTOGRID_H
#define QGSALGORITHMSNAPTOGRID_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native snap to grid algorithm.
 */
class QgsSnapToGridAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSnapToGridAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSnapToGridAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mIntervalX = 0.0;
    bool mDynamicIntervalX = false;
    QgsProperty mIntervalXProperty;

    double mIntervalY = 0.0;
    bool mDynamicIntervalY = false;
    QgsProperty mIntervalYProperty;

    double mIntervalZ = 0.0;
    bool mDynamicIntervalZ = false;
    QgsProperty mIntervalZProperty;

    double mIntervalM = 0.0;
    bool mDynamicIntervalM = false;
    QgsProperty mIntervalMProperty;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSNAPTOGRID_H


