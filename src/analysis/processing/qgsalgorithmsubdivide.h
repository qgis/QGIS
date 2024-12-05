/***************************************************************************
                         qgsalgorithmsubdivide.h
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

#ifndef QGSALGORITHMSUBDIVIDE_H
#define QGSALGORITHMSUBDIVIDE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native subdivide algorithm.
 */
class QgsSubdivideAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsSubdivideAlgorithm() = default;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSubdivideAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;

    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mMaxNodes = -1;
    bool mDynamicMaxNodes = false;
    QgsProperty mMaxNodesProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSUBDIVIDE_H
