/***************************************************************************
    qgsgeometryareacheck.h
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

#ifndef QGS_GEOMETRY_AREA_CHECK_H
#define QGS_GEOMETRY_AREA_CHECK_H

#include "qgsgeometrycheck.h"

class QgsSurface;

class ANALYSIS_EXPORT QgsGeometryAreaCheck : public QgsGeometryCheck
{
  public:
    QgsGeometryAreaCheck( QgsGeometryCheckerContext *context, double thresholdMapUnits )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PolygonGeometry}, context )
    , mThresholdMapUnits( thresholdMapUnits )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Minimal area" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryAreaCheck" ); }
    enum ResolutionMethod { MergeLongestEdge, MergeLargestArea, MergeIdenticalAttribute, Delete, NoChange };

  private:

    virtual bool checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const;
    bool mergeWithNeighbor( const QString &layerId, QgsFeature &feature, int partIdx, int method, int mergeAttributeIndex, Changes &changes, QString &errMsg ) const;

  protected:
    double mThresholdMapUnits;
};

#endif // QGS_GEOMETRY_AREA_CHECK_H
