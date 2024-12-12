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
 * \brief A geometry type check error.
 */
class ANALYSIS_EXPORT QgsGeometryTypeCheckError : public QgsSingleGeometryCheckError
{
  public:
    /**
     * Constructor for QgsGeometryTypeCheckError.
     * \param check associated geometry check
     * \param geometry original geometry
     * \param errorLocation location of geometry error
     * \param flatType geometry flat type
     */
    QgsGeometryTypeCheckError( const QgsSingleGeometryCheck *check, const QgsGeometry &geometry, const QgsGeometry &errorLocation, Qgis::WkbType flatType )
      : QgsSingleGeometryCheckError( check, geometry, errorLocation )
      , mFlatType( flatType )
    {
    }

    bool isEqual( const QgsSingleGeometryCheckError *other ) const override;

    QString description() const override;

  private:
    Qgis::WkbType mFlatType;
};

/**
 * \ingroup analysis
 * \brief A geometry type check.
 */
class ANALYSIS_EXPORT QgsGeometryTypeCheck : public QgsSingleGeometryCheck
{
    Q_DECLARE_TR_FUNCTIONS( QgsGeometryTypeCheck )
  public:
    QgsGeometryTypeCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration, int allowedTypes )
      : QgsSingleGeometryCheck( context, configuration )
      , mAllowedTypes( allowedTypes )
    {}
    QList<Qgis::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    Q_DECL_DEPRECATED QStringList resolutionMethods() const override;
    QString description() const override;
    QString id() const override;
    QgsGeometryCheck::CheckType checkType() const override;

    static QList<Qgis::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP { return { Qgis::GeometryType::Point, Qgis::GeometryType::Line, Qgis::GeometryType::Polygon }; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;

  private:
    enum ResolutionMethod
    {
      Convert,
      Delete,
      NoChange
    };
    int mAllowedTypes;
};

#endif // QGS_GEOMETRY_TYPE_CHECK_H
