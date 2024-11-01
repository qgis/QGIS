/***************************************************************************
                         qgsalgorithmdensifygeometries.h
                         ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMDENSIFYGEOMETRIESBYCOUNT_H
#define QGSALGORITHMDENSIFYGEOMETRIESBYCOUNT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


class QgsDensifyGeometriesByCountAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsDensifyGeometriesByCountAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDensifyGeometriesByCountAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mVerticesCnt = 0;
    bool mDynamicVerticesCnt = false;
    QgsProperty mVerticesCntProperty;
};

///@endcond PRIVATE
#endif // QGSALGORITHMDENSIFYGEOMETRIESBYCOUNT_H
