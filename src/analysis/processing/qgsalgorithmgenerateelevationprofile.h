/***************************************************************************
                         qgsalgorithmgenerateelevationprofile.h
                         ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMGENERATEELEVATIONPROFILE_H
#define QGSALGORITHMGENERATEELEVATIONPROFILE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprofilerenderer.h"
#include "qgsapplication.h"

class QgsProfilePlotRenderer;

///@cond PRIVATE

/**
 * Native generate elevation profile image algorithm.
 */
class QgsGenerateElevationProfileAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsGenerateElevationProfileAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsGenerateElevationProfileAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsProfilePlotRenderer> mRenderer;

    static constexpr double MAX_ERROR_PIXELS = 2;
};

///@endcond

#endif // QGSALGORITHMGENERATEELEVATIONPROFILE_H
