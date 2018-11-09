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
#include "qgsgeometrycheckerror.h"

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
    Q_GADGET

  public:
    enum ResolutionMethod
    {
      NoChange,
      AddMissingVertex
    };
    Q_ENUM( ResolutionMethod )

    explicit QgsGeometryMissingVertexCheck( const QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration );
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;

    QString description() const override;
    QString id() const override;
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override;
    QgsGeometryCheck::Flags flags() const override;
    QgsGeometryCheck::CheckType checkType() const override;

///@cond private
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP;
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP;
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::Flags factoryFlags() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
///@endcond

  private:
    void processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature, QgsFeedback *feedback ) const;
};



#endif // QGSGEOMETRYMISSINGVERTEXCHECK_H
