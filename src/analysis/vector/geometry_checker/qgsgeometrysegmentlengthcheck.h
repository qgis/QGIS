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

#define SIP_NO_FILE

#ifndef QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
#define QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometrySegmentLengthCheck : public QgsGeometryCheck
{
  public:
    QgsGeometrySegmentLengthCheck( QgsGeometryCheckContext *context, double minLengthMapUnits )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context )
    , mMinLengthMapUnits( minLengthMapUnits )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Minimal segment length" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySegmentLengthCheck" ); }

    enum ResolutionMethod { NoChange };

  private:
    double mMinLengthMapUnits;
};

#endif // QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
