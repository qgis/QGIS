/***************************************************************************
                         qgsalgorithmangletonearest.h
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSALGORITHMANGLETONEAREST_H
#define QGSALGORITHMANGLETONEAREST_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrenderer.h"

///@cond PRIVATE

/**
 * Native angle to nearest feature algorithm.
 */
class QgsAngleToNearestAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsAngleToNearestAlgorithm() = default;
    ~QgsAngleToNearestAlgorithm() override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsAngleToNearestAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    bool mIsInPlace = false;
    std::unique_ptr<QgsFeatureRenderer> mSourceRenderer;
};

///@endcond PRIVATE

#endif // QGSALGORITHMANGLETONEAREST_H
