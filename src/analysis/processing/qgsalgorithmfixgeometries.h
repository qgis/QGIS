/***************************************************************************
                         qgsalgorithmfixgeometries.h
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

#ifndef QGSALGORITHMFIXGEOMETRIES_H
#define QGSALGORITHMFIXGEOMETRIES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native repair geometries algorithm.
 */
class QgsFixGeometriesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsFixGeometriesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsFixGeometriesAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QgsProcessingFeatureSource::Flag sourceFlags() const override;
    QString outputName() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type type ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
  private:

    Qgis::MakeValidMethod mMethod = Qgis::MakeValidMethod::Linework;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFIXGEOMETRIES_H


