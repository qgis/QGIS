/***************************************************************************
    qgsgeometryselfintersectioncheck.h
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

#ifndef QGS_GEOMETRY_SELFINTERSECTION_CHECK_H
#define QGS_GEOMETRY_SELFINTERSECTION_CHECK_H

#include "qgsgeometryutils.h"
#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometrySelfIntersectionCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometrySelfIntersectionCheckError( const QgsGeometryCheck *check,
                                           const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                           const QgsPointXY &errorLocation,
                                           QgsVertexId vidx,
                                           const QgsGeometryUtils::SelfIntersection &inter )
      : QgsGeometryCheckError( check, layerFeature, errorLocation, vidx )
      , mInter( inter )
    { }
    const QgsGeometryUtils::SelfIntersection &intersection() const { return mInter; }
    bool isEqual( QgsGeometryCheckError *other ) const override;
    bool handleChanges( const QgsGeometryCheck::Changes &changes ) override;
    void update( const QgsGeometrySelfIntersectionCheckError *other )
    {
      QgsGeometryCheckError::update( other );
      // Static cast since this should only get called if isEqual == true
      const QgsGeometrySelfIntersectionCheckError *err = static_cast<const QgsGeometrySelfIntersectionCheckError *>( other );
      mInter.point = err->mInter.point;
    }

  private:
    QgsGeometryUtils::SelfIntersection mInter;
};

class ANALYSIS_EXPORT QgsGeometrySelfIntersectionCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometrySelfIntersectionCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Self intersection" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySelfIntersectionCheck" ); }

    enum ResolutionMethod { ToMultiObject, ToSingleObjects, NoChange };
};

#endif // QGS_GEOMETRY_SELFINTERSECTION_CHECK_H
