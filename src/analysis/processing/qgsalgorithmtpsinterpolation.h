/***************************************************************************
                         qgsalgorithmtpsinterpolation.h
                         ---------------------------------
    begin                : August 2026
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

#ifndef QGSALGORITHMTPSINTERPOLATION_H
#define QGSALGORITHMTPSINTERPOLATION_H

#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


///@cond PRIVATE

class ANALYSIS_EXPORT QgsThinPlateSplineAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QIcon icon() const override;
    QString svgIconPath() const override;
    QString group() const override;
    QString groupId() const override;
    QList<QgsAcademicReference> academicReferences() const override;

  protected:
    void addCommonParameters();
    void addOutputParameters();
    void processBase( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    std::unique_ptr<QgsFeatureSource> mSource;
    QString mFieldName;
    double mRegularization = 0;
    QgsRectangle mExtent;
    double mPixelSize = 0;
    QString mOutputPath;
    QString mCreationOptions;
    double mNoDataValue = 0;
    int mFieldIndex = -1;
};

/**
 * Native local Thin Plate Spline interpolation algorithm.
 */
class ANALYSIS_EXPORT QgsLocalThinPlateSplineAlgorithm : public QgsThinPlateSplineAlgorithmBase
{
  public:
    QgsLocalThinPlateSplineAlgorithm();

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsLocalThinPlateSplineAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native global Thin Plate Spline interpolation algorithm.
 */
class ANALYSIS_EXPORT QgsGlobalThinPlateSplineAlgorithm : public QgsThinPlateSplineAlgorithmBase
{
  public:
    QgsGlobalThinPlateSplineAlgorithm();

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsGlobalThinPlateSplineAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMTPSINTERPOLATION_H
