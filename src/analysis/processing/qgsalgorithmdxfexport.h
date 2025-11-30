/***************************************************************************
  qgsalgorithmdxfexport.h
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORTALGORITHM_H
#define QGSDXFEXPORTALGORITHM_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsDxfExportAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDxfExportAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QgsDxfExportAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QMap<QString, QString> mMapThemeStyleOverrides;
    Qgis::ScaleCalculationMethod mScaleMethod = Qgis::ScaleCalculationMethod::HorizontalMiddle;
};

///@endcond PRIVATE

#endif // QGSDXFEXPORTALGORITHM_H
