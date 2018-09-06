/***************************************************************************
                      qgsisvalidgeometrycheck.h
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSISVALIDGEOMETRYCHECK_H
#define QGSISVALIDGEOMETRYCHECK_H

#define SIP_NO_FILE

#include "qgssinglegeometrycheck.h"

/**
 * Checks if geometries are valid.
 */
class ANALYSIS_EXPORT QgsIsValidGeometryCheck : public QgsSingleGeometryCheck
{
  public:
    explicit QgsIsValidGeometryCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsSingleGeometryCheck( FeatureNodeCheck, context, configuration ) {}

    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;

    QStringList resolutionMethods() const override;
    QString factoryDescription() const { return tr( "Is Valid" ); }
    QString description() const override { return factoryDescription(); }
    QString factoryId() const
    {
      return QStringLiteral( "QgsIsValidCheck" );
    }

    QString id() const override { return factoryId(); }
};

#endif // QGSISVALIDGEOMETRYCHECK_H
