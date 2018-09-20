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

class ANALYSIS_EXPORT QgsGeometryTypeCheck : public QgsSingleGeometryCheck
{
  public:
    QgsGeometryTypeCheck( QgsGeometryCheckContext *context, int allowedTypes )
      : QgsSingleGeometryCheck( FeatureCheck, {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context )
    , mAllowedTypes( allowedTypes )
    {}
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry, const QVariantMap &configuration ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Geometry type" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryTypeCheck" ); }
  private:
    enum ResolutionMethod { Convert, Delete, NoChange };
    int mAllowedTypes;
};

#endif // QGS_GEOMETRY_TYPE_CHECK_H
