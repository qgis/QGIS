/***************************************************************************
 *  qgsgeometryoverlapcheck.h                                              *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_OVERLAP_CHECK_H
#define QGS_GEOMETRY_OVERLAP_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryOverlapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryOverlapCheckError( const QgsGeometryCheck* check,
                                  const QgsFeatureId& featureId,
                                  const QgsPointV2& errorLocation,
                                  const QVariant& value,
                                  const QgsFeatureId& otherId )
        : QgsGeometryCheckError( check, featureId, errorLocation, QgsVertexId(), value, ValueArea ), mOtherId( otherId ) { }
    const QgsFeatureId& otherId() const { return mOtherId; }

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
        : QgsGeometryCheck( FeatureCheck, featurePool ), mThreshold( threshold ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Overlap" ); }
    QString errorName() const override { return "QgsGeometryOverlapCheck"; }
  private:
    enum ResolutionMethod { Subtract, NoChange };
    double mThreshold;
};

#endif // QGS_GEOMETRY_OVERLAP_CHECK_H
