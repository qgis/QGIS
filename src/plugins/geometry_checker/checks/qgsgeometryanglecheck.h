/***************************************************************************
 *  qgsgeometryanglecheck.h                                                *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_ANGLE_CHECK_H
#define QGS_GEOMETRY_ANGLE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryAngleCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryAngleCheck( QgsFeaturePool* featurePool, double minAngle )
        : QgsGeometryCheck( FeatureNodeCheck, featurePool )
        , mMinAngle( minAngle )
    {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Minimal angle" ); }
    QString errorName() const override { return "QgsGeometryAngleCheck"; }
  private:
    enum ResolutionMethod { DeleteNode, NoChange };
    double mMinAngle;
};

#endif // QGS_GEOMETRY_ANGLE_CHECK_H
