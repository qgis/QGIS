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

/**
 * \ingroup analysis
 * \brief Area check.
 */
class ANALYSIS_EXPORT QgsGeometryAreaCheck : public QgsGeometryCheck
{
    Q_DECLARE_TR_FUNCTIONS( QgsGeometryAreaCheck )
  public:
    enum ResolutionMethod { MergeLongestEdge, MergeLargestArea, MergeIdenticalAttribute, Delete, NoChange };

    QgsGeometryAreaCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration )
      , mAreaThreshold( configurationValue<double>( "areaThreshold" ) )
    {}
    QList<Qgis::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    Q_DECL_DEPRECATED QStringList resolutionMethods() const override;
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

    static QList<Qgis::GeometryType> factoryCompatibleGeometryTypes() {return {Qgis::GeometryType::Polygon}; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QString description() const override { return factoryDescription(); }
    static QgsGeometryCheck::CheckType factoryCheckType() { return QgsGeometryCheck::FeatureCheck; }

  private:
    static QString factoryId() { return QStringLiteral( "QgsGeometryAreaCheck" ); }
    static QString factoryDescription() { return tr( "Minimal area" ); }
    virtual bool checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const;
    bool mergeWithNeighbor( const QMap<QString, QgsFeaturePool *> &featurePools, const QString &layerId, QgsFeature &feature, int partIdx, int method, int mergeAttributeIndex, Changes &changes, QString &errMsg ) const;

    const double mAreaThreshold;
};

#endif // QGS_GEOMETRY_AREA_CHECK_H
