/***************************************************************************
                         qgsalgorithmdistancematrix.h
                         ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#ifndef QGSALGORITHMDISTANCEMATRIX_H
#define QGSALGORITHMDISTANCEMATRIX_H

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


///@cond PRIVATE

/**
 * Native distance matrix algorithm.
 */
class QgsDistanceMatrixAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDistanceMatrixAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmDistanceMatrix.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmDistanceMatrix.svg"_s ); }
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDistanceMatrixAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;

  private:
    enum MatrixType
    {
      Linear = 0, //!< N*k x 3: one row per (input, target) pair
      Standard,   //!< N x T: one row per input, one column per target
      Summary     //!< N x 5: one row per input with mean/stddev/min/max
    };

    QVariantMap linearMatrix( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );
    QVariantMap regularMatrix( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    std::unique_ptr<QgsFeatureSource> mSource;
    std::unique_ptr<QgsFeatureSource> mTarget;
    QString mSourceField;
    QString mTargetField;
    MatrixType mMatrixType = Linear;
    long long mKPoints = 0;
    bool mSameLayer = false;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDISTANCEMATRIX_H
