/***************************************************************************
                      qgsgeometryisvalidcheck.h
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

#ifndef QGSGEOMETRYISVALIDCHECK_H
#define QGSGEOMETRYISVALIDCHECK_H

#define SIP_NO_FILE

#include "qgssinglegeometrycheck.h"

/**
 * Checks if geometries are valid using the configured method of
 */
class ANALYSIS_EXPORT QgsGeometryIsValidCheck : public QgsSingleGeometryCheck
{
  public:
    explicit QgsGeometryIsValidCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration );

    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override;
    QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const override;
    QStringList resolutionMethods() const override;
    QString description() const override { return factoryDescription(); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

///@cond private
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP;
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP;
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::Flags factoryFlags() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
///@endcond
};

#endif // QGSGEOMETRYISVALIDCHECK_H
