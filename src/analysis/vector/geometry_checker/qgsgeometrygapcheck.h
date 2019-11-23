/***************************************************************************
    qgsgeometrygapcheck.h
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

#ifndef QGS_GEOMETRY_GAP_CHECK_H
#define QGS_GEOMETRY_GAP_CHECK_H

#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"
#include "qgsfeatureid.h"

/**
 * \ingroup analysis
 * An error produced by a QgsGeometryGapCheck.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryGapCheckError : public QgsGeometryCheckError
{
  public:

    /**
     * Create a new gap check error produced by \a check on the layer \a layerId.
     * The \a geometry of the gap needs to be in map coordinates.
     * The \a neighbors are a map of layer ids and feature ids.
     * The \a area of the gap in map units and the bounding box of the gap in map units too.
     */
    QgsGeometryGapCheckError( const QgsGeometryCheck *check,
                              const QString &layerId,
                              const QgsGeometry &geometry,
                              const QMap<QString, QgsFeatureIds> &neighbors,
                              double area,
                              const QgsRectangle &gapAreaBBox,
                              const QgsRectangle &contextArea )
      : QgsGeometryCheckError( check, layerId, FID_NULL, geometry, geometry.constGet()->centroid(), QgsVertexId(), area, ValueArea )
      , mNeighbors( neighbors )
      , mGapAreaBBox( gapAreaBBox )
      , mContextBoundingBox( contextArea )
    {
    }

    QgsRectangle contextBoundingBox() const override;

    /**
     * A map of layers and feature ids of the neighbors of the gap.
     */
    const QMap<QString, QgsFeatureIds> &neighbors() const { return mNeighbors; }

    bool isEqual( QgsGeometryCheckError *other ) const override;

    bool closeMatch( QgsGeometryCheckError *other ) const override;

    void update( const QgsGeometryCheckError *other ) override;

    bool handleChanges( const QgsGeometryCheck::Changes & /*changes*/ ) override;

    QgsRectangle affectedAreaBBox() const override;

    QMap<QString, QgsFeatureIds > involvedFeatures() const override;

    QIcon icon() const override;

  private:
    QMap<QString, QgsFeatureIds> mNeighbors;
    QgsRectangle mGapAreaBBox;
    QgsRectangle mContextBoundingBox;
};


/**
 * \ingroup analysis
 * Checks for gaps between neighbouring polygons.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryGapCheck : public QgsGeometryCheck
{
    Q_GADGET
  public:
    //! Resolution methods for geometry gap checks
    enum ResolutionMethod
    {
      MergeLongestEdge, //!< Merge the gap with the polygon with the longest shared edge.
      NoChange, //!< Do not handle the error.
      AddToAllowedGaps
    };
    Q_ENUM( ResolutionMethod )

    /**
     * The \a configuration accepts a "gapThreshold" key which specifies
     * the maximum gap size in squared map units. Any gaps which are larger
     * than this area are accepted. If "gapThreshold" is set to 0, the check
     * is disabled.
     */
    explicit QgsGeometryGapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration );

    void prepare( const QgsGeometryCheckContext *context, const QVariantMap &configuration ) override;

    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;

    QString description() const override;
    QString id() const override;
    QgsGeometryCheck::Flags flags() const override;
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

///@cond private
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::Flags factoryFlags() SIP_SKIP;
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP;
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
///@endcond private

  private:
    bool mergeWithNeighbor( const QMap<QString, QgsFeaturePool *> &featurePools,
                            QgsGeometryGapCheckError *err, Changes &changes, QString &errMsg ) const;

    const double mGapThresholdMapUnits;
    QgsWeakMapLayerPointer mAllowedGapsLayer;
    std::unique_ptr<QgsVectorLayerFeatureSource> mAllowedGapsSource;
    double mAllowedGapsBuffer;

};

#endif // QGS_GEOMETRY_GAP_CHECK_H
