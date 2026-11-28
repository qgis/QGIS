/***************************************************************************
                         qgsalgorithmlinestopolygons.h
                         ------------------------------
    begin                : November 2026
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

#ifndef QGSALGORITHMLINESTOPOLYGONS_H
#define QGSALGORITHMLINESTOPOLYGONS_H

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;

///@cond PRIVATE

/**
 * Native lines to polygons algorithm.
 */
class QgsLinesToPolygonsAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsLinesToPolygonsAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmLineToPolygon.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmLineToPolygon.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsLinesToPolygonsAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;
    Qgis::ProcessingSourceType outputType() const;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    QString outputName() const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    static Qgis::WkbType convertWkbToPolygons( Qgis::WkbType wkb );
    static QgsGeometry convertToPolygons( const QgsGeometry &geometry );
    static void collectSurfaces( const QgsAbstractGeometry *geometry, QList<QgsPolygon *> &surfaces );
};

///@endcond PRIVATE

#endif // QGSALGORITHMLINESTOPOLYGONS_H
