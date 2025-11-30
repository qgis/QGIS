/***************************************************************************
                         qgsalgorithmserviceareafrompoint.h
                         ---------------------
    begin                : JUly 2018
    copyright            : (C) 2018 by Alexander Bruy
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

#ifndef QGSALGORITHMSERVICEAREAFROMPOINT_H
#define QGSALGORITHMSERVICEAREAFROMPOINT_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsalgorithmnetworkanalysisbase.h"

///@cond PRIVATE

/**
 * Native service area (from point) algorithm.
 */
class QgsServiceAreaFromPointAlgorithm : public QgsNetworkAnalysisAlgorithmBase
{
  public:
    QgsServiceAreaFromPointAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsServiceAreaFromPointAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSERVICEAREAFROMPOINT_H
