/***************************************************************************
                         qgsalgorithmtranslate.h
                         ---------------------
    begin                : November 2017
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

#ifndef QGSALGORITHMTRANSLATE_H
#define QGSALGORITHMTRANSLATE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native translate algorithm.
 */
class QgsTranslateAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsTranslateAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsTranslateAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;

  private:

    double mDeltaX = 0.0;
    double mDeltaY = 0.0;
    double mDeltaZ = 0.0;
    double mDeltaM = 0.0;

};


///@endcond PRIVATE

#endif // QGSALGORITHMTRANSLATE_H


