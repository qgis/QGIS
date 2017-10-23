/***************************************************************************
    qgsgeometrylineintersectioncheck.h
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

#ifndef QGSGEOMETRYLINEINTERSECTIONCHECK_H
#define QGSGEOMETRYLINEINTERSECTIONCHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometryLineIntersectionCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryLineIntersectionCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry}, context )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Intersection" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryLineIntersectionCheck" ); }

    enum ResolutionMethod { NoChange };
};

#endif // QGSGEOMETRYLINEINTERSECTIONCHECK_H
