/***************************************************************************
                        qgsalgorithmcheckgeometrycontained.h
                        ---------------------
   begin                : January 2025
   copyright            : (C) 2024 by Jacky Volpes
   email                : jacky dot volpes at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCHECKGEOMETRYCONTAINED_H
#define QGSALGORITHMCHECKGEOMETRYCONTAINED_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsfeaturepool.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsGeometryCheckContainedAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsGeometryCheckContainedAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmFlags flags() const override;
    [[nodiscard]] QgsGeometryCheckContainedAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    int mTolerance { 8 };
    static QgsFields outputFields();
};

///@endcond PRIVATE

#endif // QGSALGORITHMCHECKGEOMETRYCONTAINED_H
