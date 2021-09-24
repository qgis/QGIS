/***************************************************************************
                         qgsalgorithmlinedensity.h
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMLINEDENSITY_H
#define QGSALGORITHMLINEDENSITY_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "qgsdistancearea.h"

///@cond PRIVATE

/**
 * Line Density Algorithm as implemented in ESRI ArcGIS Spatial Analyst
 *
 * Literature:
 * Silverman, B.w. Density Estimation for Statistics and Data Analsis.
 * New York: Chapman and Hall, 1986
 *
 */
class QgsLineDensityAlgorithm : public QgsProcessingAlgorithm
{
  public:

    QgsLineDensityAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmLineDensity.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmLineDensity.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsLineDensityAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context,
                                  QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr< QgsFeatureSource > mSource;
    QString mWeightField;
    double mSearchRadius;
    double mPixelSize;
    QgsGeometry mSearchGeometry;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    QgsDistanceArea mDa;
    QgsSpatialIndex mIndex;
    QHash<QgsFeatureId, double> mFeatureWeights;

};

///@endcond PRIVATE

#endif // QGSALGORITHMLINEDENSITY_H
