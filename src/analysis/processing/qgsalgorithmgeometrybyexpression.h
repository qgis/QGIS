/***************************************************************************
                         qgsalgorithmgeometrybyexpression.h
                         ----------------------
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

#ifndef QGSALGORITHMGEOMETRYBYEXPRESSION_H
#define QGSALGORITHMGEOMETRYBYEXPRESSION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native geometry by expression algorithm.
 */
class QgsGeometryByExpressionAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsGeometryByExpressionAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsGeometryByExpressionAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType ) const override;
    Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    Qgis::WkbType mWkbType;
    QgsExpression mExpression;
    QgsExpressionContext mExpressionContext;
};

///@endcond PRIVATE

#endif // QGSALGORITHMGEOMETRYBYEXPRESSION_H
