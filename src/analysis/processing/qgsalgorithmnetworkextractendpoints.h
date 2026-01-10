/***************************************************************************
                         qgsalgorithmnetworkextractendpoints.h
                         -----------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSALGORITHMNETWORKEXTRACTENDPOINTS_H
#define QGSALGORITHMNETWORKEXTRACTENDPOINTS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsalgorithmnetworkanalysisbase.h"

///@cond PRIVATE

/**
 * Native extract network endpoints algorithm.
 */
class QgsExtractNetworkEndpointsAlgorithm : public QgsNetworkAnalysisAlgorithmBase
{
  public:
    QgsExtractNetworkEndpointsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsExtractNetworkEndpointsAlgorithm *createInstance() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMNETWORKEXTRACTENDPOINTS_H
