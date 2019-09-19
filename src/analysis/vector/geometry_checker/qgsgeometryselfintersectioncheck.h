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

/**
 * \ingroup analysis
 */
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

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometrySelfIntersectionCheck : public QgsSingleGeometryCheck
{
  public:
    enum ResolutionMethod
    {
      ToMultiObject,
      ToSingleObjects,
      NoChange
    };

    explicit QgsGeometrySelfIntersectionCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration = QVariantMap() )
      : QgsSingleGeometryCheck( context,
                                configuration ) {}
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString description() const override { return factoryDescription(); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::Flags flags() const override {return factoryFlags(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;

///@cond private
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP;
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP;
    static QString factoryDescription() SIP_SKIP;
    static QgsGeometryCheck::Flags factoryFlags() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
///@endcond private

};

#endif // QGS_GEOMETRY_SELFINTERSECTION_CHECK_H
