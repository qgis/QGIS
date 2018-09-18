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
#include "qgsvectorlayer.h"

class ANALYSIS_EXPORT QgsGeometryOverlapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryOverlapCheckError( const QgsGeometryCheck *check,
                                  const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                  const QgsGeometry &geometry,
                                  const QgsPointXY &errorLocation,
                                  const QVariant &value,
                                  const QgsGeometryCheckerUtils::LayerFeature &overlappedFeature )
      : QgsGeometryCheckError( check, layerFeature.layer()->id(), layerFeature.feature().id(), geometry, errorLocation, QgsVertexId(), value, ValueArea )
      , mOverlappedFeature( qMakePair( overlappedFeature.layer()->id(), overlappedFeature.feature().id() ) )
    { }
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

    QString description() const override { return QApplication::translate( "QgsGeometryTypeCheckError", "Overlap with %1:%2" ).arg( mOverlappedFeature.first ).arg( mOverlappedFeature.second ); }

  private:
    QPair<QString, QgsFeatureId> mOverlappedFeature;
};

class ANALYSIS_EXPORT QgsGeometryOverlapCheck : public QgsGeometryCheck
{
  public:
    QgsGeometryOverlapCheck( QgsGeometryCheckerContext *context, double thresholdMapUnits )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PolygonGeometry}, context )
    , mThresholdMapUnits( thresholdMapUnits )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Overlap" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryOverlapCheck" ); }

    enum ResolutionMethod { Subtract, NoChange };

  private:
    double mThresholdMapUnits;
};

#endif // QGS_GEOMETRY_OVERLAP_CHECK_H
