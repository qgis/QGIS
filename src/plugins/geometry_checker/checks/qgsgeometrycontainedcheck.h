/***************************************************************************
 *  qgsgeometrycontainedcheck.h                                            *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_COVER_CHECK_H
#define QGS_GEOMETRY_COVER_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryContainedCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryContainedCheckError( const QgsGeometryCheck* check,
                                    const QgsFeatureId& featureId,
                                    const QgsPointV2& errorLocation,
                                    const QgsFeatureId& otherId
                                  )
        : QgsGeometryCheckError( check, featureId, errorLocation, QgsVertexId(), otherId, ValueOther ), mOtherId( otherId ) { }
    const QgsFeatureId& otherId() const { return mOtherId; }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             static_cast<QgsGeometryContainedCheckError*>( other )->otherId() == otherId();
    }

    virtual QString description() const override { return QApplication::translate( "QgsGeometryContainedCheckError", "Within %1" ).arg( otherId() ); }

  private:
    QgsFeatureId mOtherId;
};

class QgsGeometryContainedCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryContainedCheck( QgsFeaturePool* featurePool ) : QgsGeometryCheck( FeatureCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Within" ); }
    QString errorName() const override { return "QgsGeometryContainedCheck"; }
  private:
    enum ResolutionMethod { Delete, NoChange };
};

#endif // QGS_GEOMETRY_COVER_CHECK_H
