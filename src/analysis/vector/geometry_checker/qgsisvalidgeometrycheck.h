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

    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry, const QVariantMap &configuration ) const override;

    QStringList resolutionMethods() const override;
    QString errorDescription() const override;
    QString errorName() const override;
};

#endif // QGSISVALIDGEOMETRYCHECK_H
