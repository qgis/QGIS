/***************************************************************************
    qgsgeometrytypecheck.h
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

#ifndef QGS_GEOMETRY_TYPE_CHECK_H
#define QGS_GEOMETRY_TYPE_CHECK_H

#include "qgssinglegeometrycheck.h"

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryTypeCheckError : public QgsSingleGeometryCheckError
{
  public:
    QgsGeometryTypeCheckError( const QgsSingleGeometryCheck *check,
                               const QgsGeometry &geometry,
                               const QgsGeometry &errorLocation,
                               QgsWkbTypes::Type flatType )
      : QgsSingleGeometryCheckError( check, geometry, errorLocation )
      , mFlatType( flatType )
    {
    }

    bool isEqual( const QgsSingleGeometryCheckError *other ) const override;

    QString description() const override;

  private:
    QgsWkbTypes::Type mFlatType;
};

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryTypeCheck : public QgsSingleGeometryCheck
{
  public:
    QgsGeometryTypeCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration, int allowedTypes )
      : QgsSingleGeometryCheck( context, configuration )
      , mAllowedTypes( allowedTypes )
    {}
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString description() const override;
    QString id() const override;
    QgsGeometryCheck::CheckType checkType() const override;

    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP {return {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;

  private:
    enum ResolutionMethod { Convert, Delete, NoChange };
    int mAllowedTypes;
};

#endif // QGS_GEOMETRY_TYPE_CHECK_H
