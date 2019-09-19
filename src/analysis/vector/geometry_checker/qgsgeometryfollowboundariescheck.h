/***************************************************************************
    qgsgeometryfollowboundariescheck.h
    ---------------------
    begin                : September 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H
#define QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H

#include "qgsgeometrycheck.h"

class QgsSpatialIndex;


/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryFollowBoundariesCheck : public QgsGeometryCheck
{
  public:
    QgsGeometryFollowBoundariesCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration, QgsVectorLayer *checkLayer );
    ~QgsGeometryFollowBoundariesCheck() override;
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    static QString factoryDescription() { return tr( "Polygon does not follow boundaries" ); }
    QString description() const override { return factoryDescription(); }
    static QString factoryId() { return QStringLiteral( "QgsGeometryFollowBoundariesCheck" ); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
  private:
    enum ResolutionMethod { NoChange };
    QgsVectorLayer *mCheckLayer;
    QgsSpatialIndex *mIndex = nullptr;
};

#endif // QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H
