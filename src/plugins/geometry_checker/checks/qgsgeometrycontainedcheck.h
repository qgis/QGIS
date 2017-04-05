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

#ifndef QGS_GEOMETRY_COVER_CHECK_H
#define QGS_GEOMETRY_COVER_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryContainedCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryContainedCheckError( const QgsGeometryCheck *check,
                                    const QString &layerId,
                                    QgsFeatureId featureId,
                                    const QgsPoint &errorLocation,
                                    const QPair<QString, QgsFeatureId> &containingFeature
                                  )
      : QgsGeometryCheckError( check, layerId, featureId, errorLocation, QgsVertexId(), QString( "%1:%2" ).arg( containingFeature.first ).arg( containingFeature.second ), ValueOther )
      , mContainingFeature( containingFeature )
    { }
    const QPair<QString, QgsFeatureId> &containingFeature() const { return mContainingFeature; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             static_cast<QgsGeometryContainedCheckError *>( other )->containingFeature() == containingFeature();
    }

    virtual QString description() const override { return QApplication::translate( "QgsGeometryContainedCheckError", "Within %1:%2" ).arg( mContainingFeature.first ).arg( mContainingFeature.second ); }

  private:
    QPair<QString, QgsFeatureId> mContainingFeature;
};

class QgsGeometryContainedCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryContainedCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Within" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryContainedCheck" ); }
  private:
    enum ResolutionMethod { Delete, NoChange };
};

#endif // QGS_GEOMETRY_COVER_CHECK_H
