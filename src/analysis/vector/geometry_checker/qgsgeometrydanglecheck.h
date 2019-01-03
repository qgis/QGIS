/***************************************************************************
    qgsgeometrydanglecheck.h
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
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

#ifndef QGSGEOMETRYDANGLECHECK_H
#define QGSGEOMETRYDANGLECHECK_H

#include "qgsgeometrycheck.h"

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometryDangleCheck : public QgsGeometryCheck
{
  public:
    QgsGeometryDangleCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration )
    {}
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;

    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    QStringList resolutionMethods() const override;
    QString description() const override { return factoryDescription(); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::LineGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    static QString factoryDescription() { return tr( "Dangle" ); }
    static QString factoryId() { return QStringLiteral( "QgsGeometryDangleCheck" ); }
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;

    enum ResolutionMethod { NoChange };
};

#endif // QGSGEOMETRYDANGLECHECK_H
