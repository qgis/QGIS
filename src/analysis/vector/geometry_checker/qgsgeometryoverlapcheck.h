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


class ANALYSIS_EXPORT QgsGeometryOverlapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryOverlapCheckError( const QgsGeometryCheck *check,
                                  const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                  const QgsGeometry &geometry,
                                  const QgsPointXY &errorLocation,
                                  const QVariant &value,
                                  const QgsGeometryCheckerUtils::LayerFeature &overlappedFeature );
    const QPair<QString, QgsFeatureId> &overlappedFeature() const { return mOverlappedFeature; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryOverlapCheckError *err = dynamic_cast<QgsGeometryOverlapCheckError *>( other );
      return err &&
             other->layerId() == layerId() &&
             other->featureId() == featureId() &&
             err->overlappedFeature() == overlappedFeature() &&
             QgsGeometryCheckerUtils::pointsFuzzyEqual( location(), other->location(), mCheck->context()->reducedTolerance ) &&
             std::fabs( value().toDouble() - other->value().toDouble() ) < mCheck->context()->reducedTolerance;
    }

    bool closeMatch( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryOverlapCheckError *err = dynamic_cast<QgsGeometryOverlapCheckError *>( other );
      return err && other->layerId() == layerId() && other->featureId() == featureId() && err->overlappedFeature() == overlappedFeature();
    }

    bool handleChanges( const QgsGeometryCheck::Changes &changes ) override
    {
      if ( !QgsGeometryCheckError::handleChanges( changes ) )
      {
        return false;
      }
      if ( changes.value( mOverlappedFeature.first ).keys().contains( mOverlappedFeature.second ) )
      {
        return false;
      }
      return true;
    }

    QString description() const override;

  private:
    QPair<QString, QgsFeatureId> mOverlappedFeature;
};

class ANALYSIS_EXPORT QgsGeometryOverlapCheck : public QgsGeometryCheck
{
  public:

    enum ResolutionMethod { Subtract, NoChange };

    QgsGeometryOverlapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration );
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
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
