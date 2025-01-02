/***************************************************************************
                         qgsalgorithmsetzvalue.h
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#ifndef QGSALGORITHMSETZVALUE_H
#define QGSALGORITHMSETZVALUE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native set Z value algorithm.
 */
class QgsSetZValueAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsSetZValueAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSetZValueAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *l ) const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mZValue = 0.0;
    bool mDynamicZValue = false;
    QgsProperty mZValueProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSETZVALUE_H
