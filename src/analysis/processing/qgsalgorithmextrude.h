/***************************************************************************
                         qgsalgorithmextrude.h
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXTRUDE_H
#define QGSALGORITHMEXTRUDE_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE


/**
 * Extrusion algorithm with SFCGAL backend.
 */
class QgsExtrudeAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExtrudeAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsExtrudeAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;

  protected:
    QString outputName() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    void initParameters( const QVariantMap & ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
#ifdef WITH_SFCGAL
    std::optional<QgsGeometry> extrudePolygon( const QgsAbstractGeometry *polygon, const QgsVector3D &extrusion, const QgsFeatureId &featureId, QgsProcessingFeedback *feedback );
#endif

  private:
#ifdef WITH_SFCGAL
    double mExtrudeX = 0.0;
    bool mDynamicExtrudeX = false;
#endif
    QgsProperty mExtrudeXProperty;

#ifdef WITH_SFCGAL
    double mExtrudeY = 0.0;
    bool mDynamicExtrudeY = false;
#endif
    QgsProperty mExtrudeYProperty;

#ifdef WITH_SFCGAL
    double mExtrudeZ = 0.0;
    bool mDynamicExtrudeZ = false;
#endif
    QgsProperty mExtrudeZProperty;
};

///@endcond PRIVATE
#endif // QGSALGORITHMEXTRUDE_H
