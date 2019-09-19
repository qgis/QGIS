/***************************************************************************
    qgsgeometryoverlapcheck.h
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

#ifndef QGS_GEOMETRY_OVERLAP_CHECK_H
#define QGS_GEOMETRY_OVERLAP_CHECK_H

#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"

/**
 * \ingroup analysis
 * An error of a QgsGeometryOverlapCheck.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryOverlapCheckError : public QgsGeometryCheckError
{
  public:

    struct OverlappedFeature
    {
      public:
        OverlappedFeature( QgsVectorLayer *vl, QgsFeatureId fid )
          : mLayerId( vl->id() )
          , mLayerName( vl->name() )
          , mFeatureId( fid )
        {}

        QString layerId() const {return mLayerId;}
        QString layerName() const {return mLayerName;}
        QgsFeatureId featureId() const {return mFeatureId;}
        bool operator==( const OverlappedFeature &other ) const {return mLayerId == other.layerId() && mFeatureId == other.featureId();}

      private:
        QString mLayerId;
        QString mLayerName;
        QgsFeatureId mFeatureId;
    };

    /**
     * Creates a new overlap check error for \a check and the \a layerFeature combination.
     * The \a geometry and \a errorLocation ned to be in map coordinates.
     * The \a value is the area of the overlapping area in map units.
     * The \a overlappedFeature provides more details about the overlap.
     */
    QgsGeometryOverlapCheckError( const QgsGeometryCheck *check,
                                  const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                  const QgsGeometry &geometry,
                                  const QgsPointXY &errorLocation,
                                  const QVariant &value,
                                  const QgsGeometryCheckerUtils::LayerFeature &overlappedFeature );

    /**
     * Returns the overlapped feature
     */
    const OverlappedFeature &overlappedFeature() const { return mOverlappedFeature; }

    bool isEqual( QgsGeometryCheckError *other ) const override;

    bool closeMatch( QgsGeometryCheckError *other ) const override;

    bool handleChanges( const QgsGeometryCheck::Changes &changes ) override;

    QString description() const override;

    QMap<QString, QgsFeatureIds > involvedFeatures() const override;
    QIcon icon() const override;

  private:
    OverlappedFeature mOverlappedFeature;
};

/**
 * \ingroup analysis
 * Checks if geometries overlap.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryOverlapCheck : public QgsGeometryCheck
{
  public:

    /**
     * Available resolution methods.
     */
    enum ResolutionMethod
    {
      Subtract, //!< Subtract the overlap region from the polygon
      NoChange //!< Do not change anything
    };

    /**
     * Checks for overlapping polygons.
     *
     * In \a configuration a maxOverlapArea parameter can be passed. In case this parameter is set
     * to something else than 0.0, the error will only be reported if the overlapping area is smaller
     * than maxOverlapArea.
     * Overlapping areas smaller than the reducedTolerance parameter of the \a context are ignored.
     */
    QgsGeometryOverlapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration );
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
    const double mOverlapThresholdMapUnits;

};

#endif // QGS_GEOMETRY_OVERLAP_CHECK_H
