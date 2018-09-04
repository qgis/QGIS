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

class ANALYSIS_EXPORT QgsGeometryContainedCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryContainedCheckError( const QgsGeometryCheck *check,
                                    const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                    const QgsPointXY &errorLocation,
                                    const QgsGeometryCheckerUtils::LayerFeature &containingFeature
                                  )
      : QgsGeometryCheckError( check, layerFeature, errorLocation, QgsVertexId(), containingFeature.id(), ValueOther )
      , mContainingFeature( qMakePair( containingFeature.layer().id(), containingFeature.feature().id() ) )
    { }
    const QPair<QString, QgsFeatureId> &containingFeature() const { return mContainingFeature; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             static_cast<QgsGeometryContainedCheckError *>( other )->containingFeature() == containingFeature();
    }

    QString description() const override { return QApplication::translate( "QgsGeometryContainedCheckError", "Within feature" ); }

  private:
    QPair<QString, QgsFeatureId> mContainingFeature;
};

class ANALYSIS_EXPORT QgsGeometryContainedCheck : public QgsGeometryCheck
{
  public:
    explicit QgsGeometryContainedCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Within" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryContainedCheck" ); }

    enum ResolutionMethod { Delete, NoChange };
};

#endif // QGS_GEOMETRY_COVER_CHECK_H
