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
    QgsGeometryAreaCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( FeatureCheck, context, configuration )
      , mAreaThreshold( configurationValue<double>( "areaThreshold" ) )
    {}
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() {return {QgsWkbTypes::PolygonGeometry}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString factoryDescription() const { return tr( "Minimal area" ); }
    QString description() const override { return factoryDescription(); }
    QString factoryId() const { return QStringLiteral( "QgsGeometryAreaCheck" ); }
    QString id() const override { return factoryId(); }
    enum ResolutionMethod { MergeLongestEdge, MergeLargestArea, MergeIdenticalAttribute, Delete, NoChange };

  private:
    virtual bool checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const;
    bool mergeWithNeighbor( const QMap<QString, QgsFeaturePool *> &featurePools, const QString &layerId, QgsFeature &feature, int partIdx, int method, int mergeAttributeIndex, Changes &changes, QString &errMsg ) const;

    const double mAreaThreshold;
};

#endif // QGS_GEOMETRY_AREA_CHECK_H
