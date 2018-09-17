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
    explicit QgsIsValidGeometryCheck( QgsGeometryCheckerContext *context )
      : QgsSingleGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}

    QList<QgsGeometryCheckError *> processGeometry( const QgsGeometryCheckerUtils::LayerFeature &layerFeature, const QgsGeometry &geometry ) const override;

    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Is Valid" ); }
    QString errorName() const override { return QStringLiteral( "QgsIsValidCheck" ); }
};

#endif // QGSISVALIDGEOMETRYCHECK_H
