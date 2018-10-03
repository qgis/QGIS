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

class ANALYSIS_EXPORT QgsGeometryGapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryGapCheckError( const QgsGeometryCheck *check,
                              const QString &layerId,
                              const QgsGeometry &geometry,
                              const QMap<QString, QgsFeatureIds> &neighbors,
                              double area,
                              const QgsRectangle &gapAreaBBox )
      : QgsGeometryCheckError( check, layerId, FEATUREID_NULL, geometry, geometry.constGet()->centroid(), QgsVertexId(), area, ValueArea )
      , mNeighbors( neighbors )
      , mGapAreaBBox( gapAreaBBox )
    {
    }
    const QMap<QString, QgsFeatureIds> &neighbors() const { return mNeighbors; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryGapCheckError *err = dynamic_cast<QgsGeometryGapCheckError *>( other );
      return err && QgsGeometryCheckerUtils::pointsFuzzyEqual( err->location(), location(), mCheck->context()->reducedTolerance ) && err->neighbors() == neighbors();
    }

    bool closeMatch( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryGapCheckError *err = dynamic_cast<QgsGeometryGapCheckError *>( other );
      return err && err->layerId() == layerId() && err->neighbors() == neighbors();
    }

    void update( const QgsGeometryCheckError *other ) override
    {
      QgsGeometryCheckError::update( other );
      // Static cast since this should only get called if isEqual == true
      const QgsGeometryGapCheckError *err = static_cast<const QgsGeometryGapCheckError *>( other );
      mNeighbors = err->mNeighbors;
      mGapAreaBBox = err->mGapAreaBBox;
    }

    bool handleChanges( const QgsGeometryCheck::Changes & /*changes*/ ) override
    {
      return true;
    }

    QgsRectangle affectedAreaBBox() const override
    {
      return mGapAreaBBox;
    }

  private:
    QMap<QString, QgsFeatureIds> mNeighbors;
    QgsRectangle mGapAreaBBox;
};

class ANALYSIS_EXPORT QgsGeometryGapCheck : public QgsGeometryCheck
{
  public:
    explicit QgsGeometryGapCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration );

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

    enum ResolutionMethod { MergeLongestEdge, NoChange };

  private:
    bool mergeWithNeighbor( const QMap<QString, QgsFeaturePool *> &featurePools,
                            QgsGeometryGapCheckError *err, Changes &changes, QString &errMsg ) const;

    const double mGapThresholdMapUnits;
};

#endif // QGS_GEOMETRY_GAP_CHECK_H
