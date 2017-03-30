/***************************************************************************
    qgsgeometrysegmentlengthcheck.h
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

#ifndef QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
#define QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometrySegmentLengthCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometrySegmentLengthCheck( const QMap<QString, QgsFeaturePool *> &featurePools, double minLengthMapUnits )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, featurePools )
    , mMinLengthMapUnits( minLengthMapUnits )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Minimal segment length" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySegmentLengthCheck" ); }
  private:
    enum ResolutionMethod { NoChange };
    double mMinLengthMapUnits;
};

#endif // QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
