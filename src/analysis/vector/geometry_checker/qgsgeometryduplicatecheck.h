/***************************************************************************
    qgsgeometryduplicatecheck.h
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

#ifndef QGS_GEOMETRY_DUPLICATE_CHECK_H
#define QGS_GEOMETRY_DUPLICATE_CHECK_H

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryDuplicateCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryDuplicateCheckError( const QgsGeometryCheck *check,
                                    const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                    const QgsPointXY &errorLocation,
                                    const QMap<QString, QgsFeaturePool *> &featurePools,
                                    const QMap<QString, QList<QgsFeatureId>> &duplicates )
      : QgsGeometryCheckError( check, layerFeature, errorLocation, QgsVertexId(), duplicatesString( featurePools, duplicates ) )
      , mDuplicates( duplicates )
    { }
    QMap<QString, QList<QgsFeatureId>> duplicates() const { return mDuplicates; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return other->check() == check() &&
             other->layerId() == layerId() &&
             other->featureId() == featureId() &&
             // static_cast: since other->checker() == checker is only true if the types are actually the same
             static_cast<QgsGeometryDuplicateCheckError *>( other )->duplicates() == duplicates();
    }

  private:
    QMap<QString, QList<QgsFeatureId>> mDuplicates;

    static QString duplicatesString( const QMap<QString, QgsFeaturePool *> &featurePools, const QMap<QString, QList<QgsFeatureId>> &duplicates );
};

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryDuplicateCheck : public QgsGeometryCheck
{
  public:
    explicit QgsGeometryDuplicateCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration ) {}
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;

    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    QStringList resolutionMethods() const override;
    QString description() const override { return factoryDescription(); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    static QString factoryDescription() { return tr( "Duplicate" ); }
    static QString factoryId();
    static QgsGeometryCheck::CheckType factoryCheckType();

    enum ResolutionMethod { NoChange, RemoveDuplicates };
};

#endif // QGS_GEOMETRY_DUPLICATE_CHECK_H
