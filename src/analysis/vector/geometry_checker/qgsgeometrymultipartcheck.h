/***************************************************************************
    qgsgeometrymultipartcheck.h
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

#ifndef QGS_GEOMETRY_MULTIPART_CHECK_H
#define QGS_GEOMETRY_MULTIPART_CHECK_H

#include "qgssinglegeometrycheck.h"

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryMultipartCheck : public QgsSingleGeometryCheck
{
  public:
    explicit QgsGeometryMultipartCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsSingleGeometryCheck( context,
                                configuration ) {}
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    static QString factoryDescription() { return tr( "Multipart object with only one feature" ); }
    QString description() const override { return factoryDescription(); }
    static QString factoryId() { return QStringLiteral( "QgsGeometryMultipartCheck" ); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }
    static QgsGeometryCheck::CheckType factoryCheckType() { return QgsGeometryCheck::FeatureCheck; }

    enum ResolutionMethod { ConvertToSingle, RemoveObject, NoChange };
};

#endif // QGS_GEOMETRY_MULTIPART_CHECK_H
