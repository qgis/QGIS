/***************************************************************************
    qgsgeometryanglecheck.h
    ---------------------
    begin                : September 2014
    copyright            : (C) 2015 by Sandro Mani / Sourcepole AG
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

#ifndef QGS_GEOMETRY_ANGLE_CHECK_H
#define QGS_GEOMETRY_ANGLE_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometryAngleCheck : public QgsGeometryCheck
{
  public:
    QgsGeometryAngleCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( FeatureNodeCheck, context, configuration )
      , mMinAngle( configuration.value( QStringLiteral( "minAngle" ), 0.0 ).toDouble() )
    {}
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString factoryDescription() const { return tr( "Minimal angle" ); }
    QString description() const override { return factoryDescription(); }
    QString factoryId() const { return QStringLiteral( "QgsGeometryAngleCheck" ); }
    QString id() const override { return factoryId(); }

    enum ResolutionMethod { DeleteNode, NoChange };

  private:
    double mMinAngle;
};

#endif // QGS_GEOMETRY_ANGLE_CHECK_H
