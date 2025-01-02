/***************************************************************************
                         qgsalgorithmvoronoipolygons.h
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMVORONOIPOLYGONS_H
#define QGSALGORITHMVORONOIPOLYGONS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native voronoi polygons algorithm.
 */
class QgsVoronoiPolygonsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsVoronoiPolygonsAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmVoronoi.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmVoronoi.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsVoronoiPolygonsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QString voronoiWithAttributes( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );
    QString voronoiWithoutAttributes( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    std::unique_ptr<QgsProcessingFeatureSource> mSource;
    double mBuffer = 0;
    double mTolerance = 0;
    bool mCopyAttributes = false;
};

///@endcond PRIVATE

#endif // QGSALGORITHMVORONOIPOLYGONS_H
