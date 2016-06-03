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

#ifndef QGS_GEOMETRY_OVERLAP_CHECK_H
#define QGS_GEOMETRY_OVERLAP_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryOverlapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryOverlapCheckError( const QgsGeometryCheck* check,
                                  QgsFeatureId featureId,
                                  const QgsPointV2& errorLocation,
                                  const QVariant& value,
                                  QgsFeatureId otherId )
        : QgsGeometryCheckError( check, featureId, errorLocation, QgsVertexId(), value, ValueArea )
        , mOtherId( otherId )
    { }
    QgsFeatureId otherId() const { return mOtherId; }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      QgsGeometryOverlapCheckError* err = dynamic_cast<QgsGeometryOverlapCheckError*>( other );
      return err &&
             other->featureId() == featureId() &&
             err->otherId() == otherId() &&
             QgsGeomUtils::pointsFuzzyEqual( location(), other->location(), QgsGeometryCheckPrecision::reducedTolerance() ) &&
             qAbs( value().toDouble() - other->value().toDouble() ) < QgsGeometryCheckPrecision::reducedTolerance();
    }

    bool closeMatch( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryOverlapCheckError* err = dynamic_cast<QgsGeometryOverlapCheckError*>( other );
      return err && other->featureId() == featureId() && err->otherId() == otherId();
    }

    virtual QString description() const override { return QApplication::translate( "QgsGeometryTypeCheckError", "Overlap with %1" ).arg( otherId() ); }

  private:
    QgsFeatureId mOtherId;
};

class QgsGeometryOverlapCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryOverlapCheck( QgsFeaturePool* featurePool, double threshold )
        : QgsGeometryCheck( FeatureCheck, featurePool )
        , mThreshold( threshold )
    {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Overlap" ); }
    QString errorName() const override { return "QgsGeometryOverlapCheck"; }
  private:
    enum ResolutionMethod { Subtract, NoChange };
    double mThreshold;
};

#endif // QGS_GEOMETRY_OVERLAP_CHECK_H
