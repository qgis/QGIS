/***************************************************************************
    qgsgeometrymissingvertexcheck.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGSGEOMETRYMISSINGVERTEXCHECK_H
#define QGSGEOMETRYMISSINGVERTEXCHECK_H

#include "qgsgeometrycheck.h"

class QgsCurvePolygon;

/**
 * \ingroup analysis
 *
 * A topology check for missing vertices.
 * Any vertex which is on the border of another polygon but no corresponding vertex
 * can be found on the other polygon will be reported as an error.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryMissingVertexCheck : public QgsGeometryCheck
{
  public:
    explicit QgsGeometryMissingVertexCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( LayerCheck, {QgsWkbTypes::PolygonGeometry}, context )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Missing Vertex" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryMissingVertexCheck" ); }

    enum ResolutionMethod { NoChange };

  private:
    void processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature ) const;
};

#endif // QGSGEOMETRYMISSINGVERTEXCHECK_
