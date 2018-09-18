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
#include "qgssinglegeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometrySelfIntersectionCheckError : public QgsSingleGeometryCheckError
{
  public:
    QgsGeometrySelfIntersectionCheckError( const QgsSingleGeometryCheck *check,
                                           const QgsGeometry &geometry,
                                           const QgsGeometry &errorLocation,
                                           QgsVertexId vertexId,
                                           const QgsGeometryUtils::SelfIntersection &intersection )
      : QgsSingleGeometryCheckError( check, geometry, errorLocation, vertexId )
      , mIntersection( intersection )
    {}

    const QgsGeometryUtils::SelfIntersection &intersection() const { return mIntersection; }
    bool isEqual( const QgsSingleGeometryCheckError *other ) const override;
    bool handleChanges( const QList<QgsGeometryCheck::Change> &changes ) override;
    void update( const QgsSingleGeometryCheckError *other ) override;

  private:
    QgsGeometryUtils::SelfIntersection mIntersection;
};

class ANALYSIS_EXPORT QgsGeometrySelfIntersectionCheck : public QgsSingleGeometryCheck
{
  public:
    explicit QgsGeometrySelfIntersectionCheck( QgsGeometryCheckerContext *context )
      : QgsSingleGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Self intersection" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySelfIntersectionCheck" ); }
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry, const QVariantMap &configuration ) const override;

    enum ResolutionMethod { ToMultiObject, ToSingleObjects, NoChange };
};

#endif // QGS_GEOMETRY_SELFINTERSECTION_CHECK_H
