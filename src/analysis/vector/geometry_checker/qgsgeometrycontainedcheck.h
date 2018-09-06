/***************************************************************************
    qgsgeometrycontainedcheck.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
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

#ifndef QGS_GEOMETRY_COVER_CHECK_H
#define QGS_GEOMETRY_COVER_CHECK_H

#include "qgsgeometrycheck.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheckerror.h"

class ANALYSIS_EXPORT QgsGeometryContainedCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryContainedCheckError( const QgsGeometryCheck *check,
                                    const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                    const QgsPointXY &errorLocation,
                                    const QgsGeometryCheckerUtils::LayerFeature &containingFeature
                                  )
      : QgsGeometryCheckError( check, layerFeature, errorLocation, QgsVertexId(), containingFeature.id(), ValueOther )
      , mContainingFeature( qMakePair( containingFeature.layer()->id(), containingFeature.feature().id() ) )
    { }
    const QPair<QString, QgsFeatureId> &containingFeature() const { return mContainingFeature; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             static_cast<QgsGeometryContainedCheckError *>( other )->containingFeature() == containingFeature();
    }

    QString factoryDescription() const { return QApplication::translate( "QgsGeometryContainedCheckError", "Within feature" ); }
    QString description() const override { return factoryDescription(); }

  private:
    QPair<QString, QgsFeatureId> mContainingFeature;
};

class ANALYSIS_EXPORT QgsGeometryContainedCheck : public QgsGeometryCheck
{
  public:
    explicit QgsGeometryContainedCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( FeatureCheck, context, configuration ) {}
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString factoryDescription() const { return tr( "Within" ); }
    QString description() const override { return factoryDescription(); }
    QString factoryId() const { return QStringLiteral( "QgsGeometryContainedCheck" ); }
    QString id() const override { return factoryId(); }

    enum ResolutionMethod { Delete, NoChange };
};

#endif // QGS_GEOMETRY_COVER_CHECK_H
