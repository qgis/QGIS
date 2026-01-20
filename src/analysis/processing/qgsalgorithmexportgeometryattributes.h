/***************************************************************************
                         qgsalgorithmexportgeometryattributes.h
                         ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Alexander Bruy
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


#ifndef QGSALGORITHMEXPORTGEOMETRYATTRIBUTES_H
#define QGSALGORITHMEXPORTGEOMETRYATTRIBUTES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native export geometry attributes algorithm.
 */
class QgsExportGeometryAttributesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExportGeometryAttributesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmAddGeometryAttributes.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmAddGeometryAttributes.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsExportGeometryAttributesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsAttributes pointAttributes( const QgsGeometry &geom, const bool exportZ, const bool exportM );
    QgsAttributes polygonAttributes( const QgsGeometry &geom );
    QgsAttributes lineAttributes( const QgsGeometry &geom );

    QgsDistanceArea mDa;
    QgsCoordinateReferenceSystem mProjectCrs;
    double mDistanceConversionFactor = 1;
    double mAreaConversionFactor = 1;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTGEOMETRYATTRIBUTES_H
