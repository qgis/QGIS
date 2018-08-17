/***************************************************************************
    qgsgeometryselfcontactcheck.h
    ---------------------
    begin                : September 2017
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

#ifndef QGS_GEOMETRY_SELFCONTACT_CHECK_H
#define QGS_GEOMETRY_SELFCONTACT_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometrySelfContactCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometrySelfContactCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes & ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Self contact" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySelfContactCheck" ); }

    enum ResolutionMethod { NoChange };
};

#endif // QGS_GEOMETRY_SELFCONTACT_CHECK_H
